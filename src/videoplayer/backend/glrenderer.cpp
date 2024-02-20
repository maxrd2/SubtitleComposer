/*
    SPDX-FileCopyrightText: 2020-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "glrenderer.h"

#include <QOpenGLShader>
#include <QMutexLocker>
#include <QStringBuilder>
#include <QScreen>
#include <QWindow>

#include "helpers/common.h"
#include "videoplayer/backend/ffplayer.h"
#include "videoplayer/videoplayer.h"
#include "videoplayer/subtitletextoverlay.h"
#include "videoplayer/backend/glcolorspace.h"

extern "C" {
#include "libavutil/pixdesc.h"
#include "libswscale/swscale.h"
}

#define DEBUG_GL 0
#define FORCE_GLES 0
#define OPENGL_CORE 0
#define OPENGL_VER 2,0

#if DEBUG_GL
#define asGL(glCall) glCall; {\
    GLenum __glErr__ = glGetError(); \
    if(__glErr__ != GL_NO_ERROR) \
        qCritical("GL error 0x%04x during %s line %d in @%s", __glErr__, #glCall, __LINE__, __PRETTY_FUNCTION__); \
}
#else
#define asGL(glCall) glCall
#endif

#if defined(GL_ES_VERSION_2_0) || FORCE_GLES
#define USE_GLES
#define TEXTURE_RGB_FORMAT GL_RGBA
// NOTE: we don't support rendering >8bpp on GLES, so 16bit textures are never used
//       and cpu will convert the frame to 8bpp
#define TEXTURE_U16_FORMAT 0x822A
#else
#undef USE_GLES
#define TEXTURE_RGB_FORMAT GL_BGRA
#define TEXTURE_U16_FORMAT GL_R16
#endif

#define FRAME_IS_YUV(f) ((f & AV_PIX_FMT_FLAG_RGB) == 0)
#define FRAME_IS_PLANAR(f) ((f & AV_PIX_FMT_FLAG_PLANAR) != 0)

using namespace SubtitleComposer;

enum { ID_Y, ID_U, ID_V, ID_OVR, ID_SIZE };
enum { AV_POS, AV_VIDTEX, AV_OVRTEX, A_SIZE };

GLRenderer::GLRenderer(QWidget *parent)
	: QOpenGLWidget(parent),
	  m_overlay(nullptr),
	  m_mmOvr(nullptr),
	  m_frameConvCtx(nullptr),
	  m_bufYUV(nullptr),
	  m_mmYUV(nullptr),
	  m_bufSize(0),
	  m_bufWidth(0),
	  m_bufHeight(0),
	  m_crWidth(0),
	  m_crHeight(0),
	  m_csNeedInit(true),
	  m_vertShader(nullptr),
	  m_fragShader(nullptr),
	  m_shaderProg(nullptr),
	  m_texNeedInit(true),
	  m_lastFormat(-1),
	  m_idTex(nullptr),
	  m_vaBuf(nullptr)
{
	setUpdateBehavior(NoPartialUpdate);
}

GLRenderer::~GLRenderer()
{
	makeCurrent();
	if(m_idTex) {
		asGL(glDeleteTextures(ID_SIZE, m_idTex));
		delete[] m_idTex;
	}
	if(m_vaBuf) {
		asGL(glDeleteBuffers(A_SIZE, m_vaBuf));
		delete[] m_vaBuf;
	}
	m_vao.destroy();
	doneCurrent();
	sws_freeContext(m_frameConvCtx);
	delete[] m_bufYUV;
	delete[] m_mmYUV;
	delete[] m_mmOvr;
}

void
GLRenderer::setupProfile()
{
	QSurfaceFormat format(QSurfaceFormat::defaultFormat());
#if FORCE_GLES
	format.setRenderableType(QSurfaceFormat::OpenGLES);
#endif
	format.setVersion(OPENGL_VER);
#if DEBUG_GL
	format.setOption(QSurfaceFormat::DebugContext);
#endif
#if OPENGL_CORE
	format.setProfile(QSurfaceFormat::CoreProfile);
#endif
	QSurfaceFormat::setDefaultFormat(format);
}

void
GLRenderer::setOverlay(SubtitleTextOverlay *overlay)
{
	if(m_overlay)
		disconnect(m_overlay, nullptr, this, nullptr);
	m_overlay = overlay;
#ifdef USE_GLES
	overlay->invertPixels(true);
#endif
	connect(m_overlay, &SubtitleTextOverlay::repaintNeeded, this, QOverload<>::of(&GLRenderer::update));
}

void
GLRenderer::reset()
{
	m_csNeedInit = true;
	m_texNeedInit = true;
}

void
GLRenderer::setFrameFormat(int width, int height, int compBits, int crWidthShift, int crHeightShift)
{
	const quint8 compBytes = compBits > 8 ? 2 : 1;
	const GLsizei crWidth = AV_CEIL_RSHIFT(width, crWidthShift);
	const GLsizei crHeight = AV_CEIL_RSHIFT(height, crHeightShift);
	const quint32 bufSize = width * height * compBytes + 2 * crWidth * crHeight * compBytes;

	if(m_bufWidth == width && m_bufHeight == height
	&& m_bufSize == bufSize && m_crWidth == crWidth && m_crHeight == crHeight)
		return;

	m_bufWidth = width;
	m_bufHeight = height;
	m_crWidth = crWidth;
	m_crHeight = crHeight;

	m_glType = compBytes == 1 ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT;
	m_glFormat = compBytes == 1 ? GL_R8 : TEXTURE_U16_FORMAT;

	delete[] m_bufYUV;
	m_bufSize = bufSize;
	m_bufYUV = new quint8[m_bufSize];

	delete[] m_mmYUV;
	m_mmYUV = new quint8[(m_bufWidth >> 1) * (m_bufHeight >> 1) * compBytes];

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
	QWindow *w = windowHandle();
	if(!w)
		w = nativeParentWidget()->windowHandle();
	const QSize sr = w->screen()->size();
#else
	const QSize sr = screen()->size();
#endif
	const double rr = qMin(double(sr.width()) / width, double(sr.height()) / height);
	m_overlay->setImageSize(rr * width, rr * height);
	delete[] m_mmOvr;
	m_mmOvr = new quint8[(m_overlay->width() >> 1) * (m_overlay->height() >> 1) * 4];

	m_pitch[0] = m_bufWidth * compBytes;
	m_pitch[1] = m_pitch[2] = m_crWidth * compBytes;

	m_pixels[0] = m_bufYUV;
	m_pixels[1] = m_pixels[0] + m_pitch[0] * m_bufHeight;
	m_pixels[2] = m_pixels[1] + m_pitch[1] * m_crHeight;

	m_texNeedInit = true;
	m_csNeedInit = true;

	emit resolutionChanged();
}

void
GLRenderer::setColorspace(const AVFrame *frame)
{
	if(!m_csNeedInit)
		return;

	const AVPixFmtDescriptor *fd = av_pix_fmt_desc_get(AVPixelFormat(frame->format));
	const quint8 compBits = fd->comp[0].depth;
	const quint8 compBytes = compBits > 8 ? 2 : 1;
	const bool isYUV = FRAME_IS_YUV(fd->flags);

	qDebug("Color range: %s(%d); primaries: %s(%d); xfer: %s(%d); space: %s(%d); depth: %d",
		   av_color_range_name(frame->color_range), frame->color_range,
		   av_color_primaries_name(frame->color_primaries), frame->color_primaries,
		   av_color_transfer_name(frame->color_trc), frame->color_trc,
		   av_color_space_name(frame->colorspace), frame->colorspace,
		   compBits);

	// gamma/transfer function
	auto ctfit = _ctfi.constFind(frame->color_trc);
	if(ctfit == _ctfi.constEnd())
		ctfit = _ctfi.constFind(AVCOL_TRC_BT709);
	Q_ASSERT(ctfit != _ctfi.constEnd());
	m_ctfIn = ctfit.value();

	ctfit = _ctf.constFind(AVCOL_TRC_IEC61966_2_1); // sRGB
	Q_ASSERT(ctfit != _ctf.constEnd());
	m_ctfOut = ctfit.value();

	// colorspace conversion
	QMap<int, QVector<GLfloat>>::const_iterator cs;
	if((cs = _csm.constFind(frame->color_primaries)) != _csm.constEnd()) {
		m_csCM = QMatrix4x4(cs.value().constData(), 3, 3);
	} else if((cs = _csc.constFind(frame->colorspace)) != _csc.constEnd()) {
		m_csCM = QMatrix4x4(cs.value().constData(), 3, 3);
	} else if(isYUV) {
		cs = _csm.constFind(AVCOL_PRI_BT709);
		m_csCM = QMatrix4x4(cs.value().constData(), 3, 3);
	} else {
		m_csCM = QMatrix4x4();
	}

	if(isYUV) {
		if(frame->color_range == AVCOL_RANGE_MPEG || frame->color_range == AVCOL_RANGE_UNSPECIFIED) {
			// convert to full range and offset UV channels
			m_csCM.scale(255.0f / (219.0f + 16.0f), 255.0f / 224.0f, 255.0f / 224.0f);
			m_csCM.translate(-16.0f / 255.0f, -128.0f / 255.0f, -128.0f / 255.0f);
		} else {
			// offset signed UV channels
			m_csCM.translate(0.0f, -128.0f / 255.0f, -128.0f / 255.0f);
		}
	}

	// scale to full range when some bits are unused
	const float pixMult = double(1 << (compBytes << 3)) / double(1 << compBits);
	m_csCM.scale(pixMult, pixMult, pixMult);
}

bool
GLRenderer::validTextureFormat(const AVPixFmtDescriptor *fd)
{
	const uint64_t &f = fd->flags;
	if((f & AV_PIX_FMT_FLAG_BITSTREAM)) {
		qCritical("uploadTexture() failed: unsupported frame format [%s] - bitstream", fd->name);
		return false;
	}
	if((f & AV_PIX_FMT_FLAG_PAL)) {
		qCritical("uploadTexture() failed: unsupported frame format [%s] - palette", fd->name);
		return false;
	}
	if((f & AV_PIX_FMT_FLAG_BE)) {
		qCritical("uploadTexture() failed: unsupported frame format [%s] - bigendian", fd->name);
		return false;
	}

	if(FRAME_IS_YUV(f) && FRAME_IS_PLANAR(f)) {
		const quint8 b = fd->comp[0].depth > 8 ? 2 : 1;
		if(fd->comp[0].step != b || fd->comp[1].step != b || fd->comp[2].step != b) {
			qCritical("validTextureFormat() failed: unsupported plane step [%d, %d, %d] %s",
				   fd->comp[0].step, fd->comp[1].step, fd->comp[2].step, fd->name);
			return false;
		}
		if(fd->comp[0].offset || fd->comp[1].offset || fd->comp[2].offset) {
			qCritical("validTextureFormat() failed: unsupported plane offset [%d, %d, %d] %s",
				   fd->comp[0].offset, fd->comp[1].offset, fd->comp[2].offset, fd->name);
			return false;
		}
		if(fd->comp[0].shift || fd->comp[1].shift || fd->comp[2].shift) {
			qCritical("validTextureFormat() failed: unsupported plane shift [%d, %d, %d] %s",
				   fd->comp[0].shift, fd->comp[1].shift, fd->comp[2].shift, fd->name);
			return false;
		}
		if(fd->comp[0].depth != fd->comp[1].depth || fd->comp[0].depth != fd->comp[2].depth) {
			qCritical("validTextureFormat() failed: unsupported plane depths [%d, %d, %d] %s",
				   fd->comp[0].depth, fd->comp[1].depth, fd->comp[2].depth, fd->name);
			return false;
		}
		if(fd->nb_components < 3) {
			qCritical("validTextureFormat() failed: unsupported plane count [%d] %s",
					  fd->nb_components, fd->name);
			return false;
		}
	} else {
		qCritical("validTextureFormat() failed: unsupported frame format [%s]", fd->name);
		return false;
	}
	return true;
}

int
GLRenderer::uploadTexture(AVFrame *frame)
{
	const AVPixFmtDescriptor *fd = av_pix_fmt_desc_get(AVPixelFormat(frame->format));
	if(m_lastFormat != frame->format) {
		if(!validTextureFormat(fd))
			return -1;
		m_lastFormat = frame->format;
	}

	if(!frame->linesize[0] || !frame->linesize[1] || !frame->linesize[2]) {
		qCritical("uploadTexture() failed: invalid linesize [%d, %d, %d]",
			   frame->linesize[0], frame->linesize[1], frame->linesize[2]);
		return -1;
	}

	QMutexLocker l(&m_texMutex);

#ifdef USE_GLES
	if(fd->comp[0].depth > 8) {
		// convert >8bpp YUV
		frame->format = AV_PIX_FMT_YUV420P;

		const static AVPixFmtDescriptor *fd8 = av_pix_fmt_desc_get(AVPixelFormat(frame->format));
		m_frameConvCtx = sws_getCachedContext(m_frameConvCtx,
				frame->width, frame->height, AVPixelFormat(m_lastFormat),
				frame->width, frame->height, AVPixelFormat(frame->format),
				0, nullptr, nullptr, nullptr);

		setFrameFormat(frame->width, frame->height,
			fd8->comp[0].depth, fd8->log2_chroma_w, fd8->log2_chroma_h);

		setColorspace(frame);

		sws_scale(m_frameConvCtx, frame->data, frame->linesize, 0, frame->height,
				m_pixels, reinterpret_cast<const int *>(m_pitch));
	} else
#endif
	{
		setFrameFormat(frame->width, frame->height,
			fd->comp[0].depth, fd->log2_chroma_w, fd->log2_chroma_h);

		setColorspace(frame);

		if(frame->linesize[0] > 0)
			setFrameY(frame->data[0], frame->linesize[0]);
		else
			setFrameY(frame->data[0] + frame->linesize[0] * (frame->height - 1), -frame->linesize[0]);

		if(frame->linesize[1] > 0)
			setFrameU(frame->data[1], frame->linesize[1]);
		else
			setFrameU(frame->data[1] + frame->linesize[1] * (AV_CEIL_RSHIFT(frame->height, 1) - 1), -frame->linesize[1]);

		if(frame->linesize[2] > 0)
			setFrameV(frame->data[2], frame->linesize[2]);
		else
			setFrameV(frame->data[2] + frame->linesize[2] * (AV_CEIL_RSHIFT(frame->height, 1) - 1), -frame->linesize[2]);
	}

	m_texUploaded = false;
	update();

	return 0;
}

void
GLRenderer::setFrameY(quint8 *buf, quint32 pitch)
{
	if(pitch == m_pitch[0]) {
		memcpy(m_pixels[0], buf, pitch * m_bufHeight);
	} else {
		quint8 *dbuf = m_pixels[0];
		for(int i = 0; i < m_bufHeight; i++) {
			memcpy(dbuf, buf, m_pitch[0]);
			dbuf += m_pitch[0];
			buf += pitch;
		}
	}
}

void
GLRenderer::setFrameU(quint8 *buf, quint32 pitch)
{
	if(pitch == m_pitch[1]) {
		memcpy(m_pixels[1], buf, pitch * m_crHeight);
	} else {
		quint8 *dbuf = m_pixels[1];
		for(int i = 0; i < m_crHeight; i++) {
			memcpy(dbuf, buf, m_pitch[1]);
			dbuf += m_pitch[1];
			buf += pitch;
		}
	}
}

void
GLRenderer::setFrameV(quint8 *buf, quint32 pitch)
{
	if(pitch == m_pitch[2]) {
		memcpy(m_pixels[2], buf, pitch * m_crHeight);
	} else {
		quint8 *dbuf = m_pixels[2];
		for(int i = 0; i < m_crHeight; i++) {
			memcpy(dbuf, buf, m_pitch[2]);
			dbuf += m_pitch[2];
			buf += pitch;
		}
	}
}

void
GLRenderer::initShader()
{
	delete m_vertShader;
	m_vertShader = new QOpenGLShader(QOpenGLShader::Vertex, this);
	bool success = m_vertShader->compileSourceCode(
#ifdef USE_GLES
		"#version 100\n"
#else
		"#version 120\n"
#endif
		"attribute vec4 vPos;"
		"attribute vec2 vVidTex;"
		"attribute vec2 vOvrTex;"
		"varying vec2 vfVidTex;"
		"varying vec2 vfOvrTex;"
		"void main(void) {"
			"gl_Position = vPos;"
			"vfVidTex = vVidTex;"
			"vfOvrTex = vOvrTex;"
		"}");
	if(!success) {
		qCritical() << "GLRenderer: vertex shader compilation failed";
		return;
	}

	delete m_fragShader;
	m_fragShader = new QOpenGLShader(QOpenGLShader::Fragment, this);
//	asGL(glUniformMatrix4fv(m_texCSLoc, 1, GL_FALSE, m_csCM.constData()));

	const float *csm = m_csCM.constData();
	QString csms;
	for(int i = 0; i < 16; i++) {
		if(i) csms.append(QLatin1Char(','));
		csms.append(QString::number(csm[i], 'g', 10));
	}

	success = m_fragShader->compileSourceCode(
#ifdef USE_GLES
		$("#version 100\n"
		"precision mediump float;\n") %
#else
		$("#version 120\n") %
#endif
		$("varying vec2 vfVidTex;"
		"varying vec2 vfOvrTex;"
		"uniform sampler2D texY;"
		"uniform sampler2D texU;"
		"uniform sampler2D texV;"
		"uniform sampler2D texOvr;"
		"float toLinear(float vExp) {") % m_ctfIn % $("}"
		"float toDisplay(float vLin) {") % m_ctfOut % $("}"
		"void main(void) {"
			"vec3 yuv;"
			"yuv.x = texture2D(texY, vfVidTex).r;"
			"yuv.y = texture2D(texU, vfVidTex).r;"
			"yuv.z = texture2D(texV, vfVidTex).r;"
			"mat4 texCS = mat4(") % csms % $(");"
			"vec3 rgb = (texCS * vec4(yuv, 1)).xyz;"
			// ideally gamma would be applied to Y, but video signals apply to RGB so we do the same
			"rgb.r = toLinear(rgb.r);"
			"rgb.g = toLinear(rgb.g);"
			"rgb.b = toLinear(rgb.b);"
			// apply display (sRGB) gamma transfer
			"rgb.r = toDisplay(rgb.r);"
			"rgb.g = toDisplay(rgb.g);"
			"rgb.b = toDisplay(rgb.b);"
			"vec4 o = texture2D(texOvr, vfOvrTex);"
			"gl_FragColor = vec4(mix(rgb, o.rgb, o.w), 1);"
		"}"));
	if(!success) {
		qCritical() << "GLRenderer: fragment shader compilation failed";
		return;
	}

	delete m_shaderProg;
	m_shaderProg = new QOpenGLShaderProgram(this);
	m_shaderProg->addShader(m_fragShader);
	m_shaderProg->addShader(m_vertShader);

	m_shaderProg->bindAttributeLocation("vPos", AV_POS);
	m_shaderProg->bindAttributeLocation("vVidTex", AV_VIDTEX);
	m_shaderProg->bindAttributeLocation("vOvrTex", AV_OVRTEX);
	if(!m_shaderProg->link()) {
		qCritical() << "GLRenderer: shader linking failed:" << m_shaderProg->log();
		return;
	}
	m_shaderProg->bind();

	m_texY = m_shaderProg->uniformLocation("texY");
	m_texU = m_shaderProg->uniformLocation("texU");
	m_texV = m_shaderProg->uniformLocation("texV");
	m_texOvr = m_shaderProg->uniformLocation("texOvr");

	m_csNeedInit = true;
}

void
GLRenderer::initializeGL()
{
	QMutexLocker l(&m_texMutex);

	initializeOpenGLFunctions();
	qDebug().nospace() << "GL API: OpenGL " << (format().renderableType() == QSurfaceFormat::OpenGLES ? "ES" : "Desktop")
		<< ' ' << format().majorVersion() << "." << format().minorVersion()
#ifdef USE_GLES
		<< " (compiled for OpenGL ES)";
#else
		<< " (compiled for OpenGL Desktop)";
#endif
	qDebug() << "OpenGL version:" << reinterpret_cast<const char *>(glGetString(GL_VERSION));
	qDebug() << "GLSL version:" << reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));

	if(m_vao.create())
		m_vao.bind();

	if(m_vaBuf) {
		asGL(glDeleteBuffers(A_SIZE, m_vaBuf));
	} else {
		delete[] m_vaBuf;
	}
	m_vaBuf = new GLuint[A_SIZE];
	asGL(glGenBuffers(A_SIZE, m_vaBuf));

	{
		static const GLfloat v[] = {
			-1.0f, 1.0f,
			1.0f, 1.0f,
			-1.0f, -1.0f,
			1.0f, -1.0f,
		};
		asGL(glBindBuffer(GL_ARRAY_BUFFER, m_vaBuf[AV_POS]));
		asGL(glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW));
		asGL(glVertexAttribPointer(AV_POS, 2, GL_FLOAT, GL_FALSE, 0, nullptr));
		asGL(glEnableVertexAttribArray(AV_POS));
	}

	{
		static const GLfloat v[] = {
			0.0f,  0.0f,
			1.0f,  0.0f,
			0.0f,  1.0f,
			1.0f,  1.0f,
		};
		asGL(glBindBuffer(GL_ARRAY_BUFFER, m_vaBuf[AV_VIDTEX]));
		asGL(glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW));
		asGL(glVertexAttribPointer(AV_VIDTEX, 2, GL_FLOAT, GL_FALSE, 0, nullptr));
		asGL(glEnableVertexAttribArray(AV_VIDTEX));
	}

	asGL(glBindBuffer(GL_ARRAY_BUFFER, m_vaBuf[AV_OVRTEX]));
	asGL(glVertexAttribPointer(AV_OVRTEX, 2, GL_FLOAT, GL_FALSE, 0, nullptr));
	asGL(glEnableVertexAttribArray(AV_OVRTEX));
	m_overlayPos[2] = 0.0f; // uploadSubtitle() will upload vertex data

	if(m_idTex) {
		asGL(glDeleteTextures(ID_SIZE, m_idTex));
	} else {
		m_idTex = new GLuint[ID_SIZE];
	}
	asGL(glGenTextures(ID_SIZE, m_idTex));

	asGL(glClearColor(.0f, .0f, .0f, .0f)); // background color

	asGL(glDisable(GL_DEPTH_TEST));

	m_texNeedInit = true;
	m_csNeedInit = true;
}

void
GLRenderer::resizeGL(int width, int height)
{
	QMutexLocker l(&m_texMutex);
	asGL(glViewport(0, 0, width, height));
	m_vpWidth = width;
	m_vpHeight = height;
	m_texNeedInit = true;
	m_overlay->setRenderScale(double(m_overlay->height()) / height);
	update();
}

void
GLRenderer::paintGL()
{
	QMutexLocker l(&m_texMutex);

	glClear(GL_COLOR_BUFFER_BIT);

	if(!m_bufYUV)
		return;

	if(m_texNeedInit) {
		asGL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
	}

	if(m_csNeedInit)
		initShader();

	uploadYUV();
	uploadSubtitle();

	asGL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

	m_texNeedInit = false;
	m_csNeedInit = false;
}

template<class T, int D>
void
GLRenderer::uploadMM(int texWidth, int texHeight, T *texBuf, const T *texSrc, int vpWidth, int vpHeight)
{
	for(;;) {
		const int srcStride = texWidth * D;
		const int srcStridePD = srcStride + D;
		const int newWidth = texWidth >> 1;
		const int newHeight = texHeight >> 1;
		if(newWidth < vpWidth && newHeight < vpHeight) {
			if(m_texNeedInit) {
				asGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
				asGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
				if(D == 1) {
					asGL(glTexImage2D(GL_TEXTURE_2D, 0, m_glFormat, texWidth, texHeight, 0, GL_RED, m_glType, texSrc));
				} else { // D == 4
					asGL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texWidth, texHeight, 0, TEXTURE_RGB_FORMAT, GL_UNSIGNED_BYTE, texSrc));
				}
			} else {
				if(D == 1) {
					asGL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texWidth, texHeight, GL_RED, m_glType, texSrc));
				} else { // D == 4
					asGL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texWidth, texHeight, TEXTURE_RGB_FORMAT, GL_UNSIGNED_BYTE, texSrc));
				}
			}
			break;
		}
		texWidth = newWidth;
		texHeight = newHeight;

		T *dst = texBuf;
		const int texStride = texWidth * D;
		for(int y = 0; y < texHeight; y++) {
			const T *dstEnd = dst + texStride;
			while(dst != dstEnd) {
				if(D == 1) { // if should get optimized away
					*dst++ = (texSrc[0] + texSrc[D] + texSrc[srcStride] + texSrc[srcStridePD]) >> 2;
					texSrc += 2;
				} else {
					*dst++ = (texSrc[0] + texSrc[D] + texSrc[srcStride] + texSrc[srcStridePD]) >> 2;
					texSrc++;
					*dst++ = (texSrc[0] + texSrc[D] + texSrc[srcStride] + texSrc[srcStridePD]) >> 2;
					texSrc++;
					*dst++ = (texSrc[0] + texSrc[D] + texSrc[srcStride] + texSrc[srcStridePD]) >> 2;
					texSrc++;
					*dst++ = (texSrc[0] + texSrc[D] + texSrc[srcStride] + texSrc[srcStridePD]) >> 2;
					texSrc += D + 1;
				}
			}
			texSrc += srcStride;
		}
		texSrc = texBuf;
	}
}

void
GLRenderer::uploadYUV()
{
	if(!m_texNeedInit && m_texUploaded)
		return;

	m_texUploaded = true;

	// load Y data
	asGL(glActiveTexture(GL_TEXTURE0 + ID_Y));
	asGL(glBindTexture(GL_TEXTURE_2D, m_idTex[ID_Y]));
	if(m_glType == GL_UNSIGNED_BYTE)
		uploadMM<quint8, 1>(m_bufWidth, m_bufHeight, m_mmYUV, m_pixels[0], m_vpWidth, m_vpHeight);
	else
		uploadMM<quint16, 1>(m_bufWidth, m_bufHeight, reinterpret_cast<quint16 *>(m_mmYUV), reinterpret_cast<quint16 *>(m_pixels[0]), m_vpWidth, m_vpHeight);
	if(m_texNeedInit) {
		asGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		asGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		asGL(glUniform1i(m_texY, ID_Y));
	}

	// load U data
	asGL(glActiveTexture(GL_TEXTURE0 + ID_U));
	asGL(glBindTexture(GL_TEXTURE_2D, m_idTex[ID_U]));
	if(m_glType == GL_UNSIGNED_BYTE)
		uploadMM<quint8, 1>(m_crWidth, m_crHeight, m_mmYUV, m_pixels[1], m_vpWidth, m_vpHeight);
	else
		uploadMM<quint16, 1>(m_crWidth, m_crHeight, reinterpret_cast<quint16 *>(m_mmYUV), reinterpret_cast<quint16 *>(m_pixels[1]), m_vpWidth, m_vpHeight);
	if(m_texNeedInit) {
		asGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		asGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		asGL(glUniform1i(m_texU, ID_U));
	}

	// load V data
	asGL(glActiveTexture(GL_TEXTURE0 + ID_V));
	asGL(glBindTexture(GL_TEXTURE_2D, m_idTex[ID_V]));
	if(m_glType == GL_UNSIGNED_BYTE)
		uploadMM<quint8, 1>(m_crWidth, m_crHeight, m_mmYUV, m_pixels[2], m_vpWidth, m_vpHeight);
	else
		uploadMM<quint16, 1>(m_crWidth, m_crHeight, reinterpret_cast<quint16 *>(m_mmYUV), reinterpret_cast<quint16 *>(m_pixels[2]), m_vpWidth, m_vpHeight);
	if(m_texNeedInit) {
		asGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		asGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		asGL(glUniform1i(m_texV, ID_V));
	}
}

void
GLRenderer::uploadSubtitle()
{
	if(!m_texNeedInit && !m_overlay->isDirty())
		return;

	const QImage &img = m_overlay->image();

	const GLfloat rs = qMin(1.0 / m_overlay->renderScale(), 1.0);
	if(rs != m_overlayPos[2]) {
		m_overlayPos[2] = m_overlayPos[6] = m_overlayPos[5] = m_overlayPos[7] = rs;
		asGL(glBindBuffer(GL_ARRAY_BUFFER, m_vaBuf[AV_OVRTEX]));
		asGL(glBufferData(GL_ARRAY_BUFFER, sizeof(m_overlayPos), m_overlayPos, GL_STATIC_DRAW));
		asGL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	// overlay
	asGL(glActiveTexture(GL_TEXTURE0 + ID_OVR));
	asGL(glBindTexture(GL_TEXTURE_2D, m_idTex[ID_OVR]));
	uploadMM<quint8, 4>(img.width(), img.height(), m_mmOvr, img.constBits(), m_vpWidth / rs, m_vpHeight / rs);
	if(m_texNeedInit) {
		asGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
		asGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
		static const float borderColor[] = { .0f, .0f, .0f, .0f };
		asGL(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor));
		asGL(glUniform1i(m_texOvr, ID_OVR));
	}
}
