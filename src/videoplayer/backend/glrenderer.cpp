/*
 * Copyright (c) 2020 Mladen Milinkovic <max@smoothware.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "glrenderer.h"

#include <QOpenGLShader>
#include <QMutexLocker>
#include <QStringBuilder>

#include "videoplayer/backend/ffplayer.h"
#include "videoplayer/videoplayer.h"
#include "videoplayer/subtitletextoverlay.h"
#include "videoplayer/backend/glcolorspace.h"

extern "C" {
#include "libavutil/pixdesc.h"
}

#define DEBUG_GL 0
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

using namespace SubtitleComposer;

enum { ID_Y, ID_U, ID_V, ID_OVR, ID_SIZE };
enum { AV_POS, AV_VIDTEX, AV_OVRTEX, A_SIZE };

GLRenderer::GLRenderer(QWidget *parent)
	: QOpenGLWidget(parent),
	  m_overlay(nullptr),
	  m_mmOvr(nullptr),
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
	delete[] m_bufYUV;
	delete[] m_mmYUV;
	delete[] m_mmOvr;
}

void
GLRenderer::setupProfile()
{
	QSurfaceFormat format(QSurfaceFormat::defaultFormat());
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
	m_overlay = overlay;
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
	m_glFormat = compBytes == 1 ? GL_R8 : GL_R16;

	delete[] m_bufYUV;
	m_bufSize = bufSize;
	m_bufYUV = new quint8[m_bufSize];

	delete[] m_mmYUV;
	m_mmYUV = new quint8[(m_bufWidth >> 1) * (m_bufHeight >> 1) * compBytes];

	m_overlay->setImageSize(width, height);
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
	const bool isYUV = ~fd->flags & AV_PIX_FMT_FLAG_RGB;

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
		"#version 120\n"
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

	success = m_fragShader->compileSourceCode(QStringLiteral("#version 120\n"
		"varying vec2 vfVidTex;"
		"varying vec2 vfOvrTex;"
		"uniform sampler2D texY;"
		"uniform sampler2D texU;"
		"uniform sampler2D texV;"
		"uniform sampler2D texOvr;"
		"float toLinear(float vExp) {") % m_ctfIn % QStringLiteral("}"
		"float toDisplay(float vLin) {") % m_ctfOut % QStringLiteral("}"
		"void main(void) {"
			"vec3 yuv;"
			"yuv.x = texture2D(texY, vfVidTex).r;"
			"yuv.y = texture2D(texU, vfVidTex).r;"
			"yuv.z = texture2D(texV, vfVidTex).r;"
			"mat4 texCS = mat4(") % csms % QStringLiteral(");"
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
	qDebug() << "OpenGL version: " << reinterpret_cast<const char *>(glGetString(GL_VERSION));
	qDebug() << "GLSL version: " << reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));

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
		asGL(glPixelStorei(GL_UNPACK_ALIGNMENT, m_bufWidth % 4 == 0 ? 4 : 1));
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
GLRenderer::uploadMM(int texWidth, int texHeight, T *texBuf, const T *texSrc)
{
	int level = 0;
	for(;;) {
		if(m_texNeedInit) {
			if(D == 1) {
				asGL(glTexImage2D(GL_TEXTURE_2D, level, m_glFormat, texWidth, texHeight, 0, GL_RED, m_glType, texSrc));
			} else { // D == 4
				asGL(glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA8, texWidth, texHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, texSrc));
			}
		} else {
			if(D == 1) {
				asGL(glTexSubImage2D(GL_TEXTURE_2D, level, 0, 0, texWidth, texHeight, GL_RED, m_glType, texSrc));
			} else { // D == 4
				asGL(glTexSubImage2D(GL_TEXTURE_2D, level, 0, 0, texWidth, texHeight, GL_BGRA, GL_UNSIGNED_BYTE, texSrc));
			}
		}

		const int srcStride = texWidth * D;
		const int srcStridePD = srcStride + D;
		texWidth >>= 1;
		texHeight >>= 1;
		if(texWidth < m_vpWidth && texHeight < m_vpHeight) {
			if(m_texNeedInit) {
				asGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
				asGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
				asGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0));
				asGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, level));
			}
			break;
		}
		level++;

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
	// load Y data
	asGL(glActiveTexture(GL_TEXTURE0 + ID_Y));
	asGL(glBindTexture(GL_TEXTURE_2D, m_idTex[ID_Y]));
	if(m_glType == GL_UNSIGNED_BYTE)
		uploadMM<quint8, 1>(m_bufWidth, m_bufHeight, m_mmYUV, m_pixels[0]);
	else
		uploadMM<quint16, 1>(m_bufWidth, m_bufHeight, reinterpret_cast<quint16 *>(m_mmYUV), reinterpret_cast<quint16 *>(m_pixels[0]));
	if(m_texNeedInit) {
		asGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		asGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		asGL(glUniform1i(m_texY, ID_Y));
	}

	// load U data
	asGL(glActiveTexture(GL_TEXTURE0 + ID_U));
	asGL(glBindTexture(GL_TEXTURE_2D, m_idTex[ID_U]));
	if(m_glType == GL_UNSIGNED_BYTE)
		uploadMM<quint8, 1>(m_crWidth, m_crHeight, m_mmYUV, m_pixels[1]);
	else
		uploadMM<quint16, 1>(m_crWidth, m_crHeight, reinterpret_cast<quint16 *>(m_mmYUV), reinterpret_cast<quint16 *>(m_pixels[1]));
	if(m_texNeedInit) {
		asGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		asGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		asGL(glUniform1i(m_texU, ID_U));
	}

	// load V data
	asGL(glActiveTexture(GL_TEXTURE0 + ID_V));
	asGL(glBindTexture(GL_TEXTURE_2D, m_idTex[ID_V]));
	if(m_glType == GL_UNSIGNED_BYTE)
		uploadMM<quint8, 1>(m_crWidth, m_crHeight, m_mmYUV, m_pixels[2]);
	else
		uploadMM<quint16, 1>(m_crWidth, m_crHeight, reinterpret_cast<quint16 *>(m_mmYUV), reinterpret_cast<quint16 *>(m_pixels[2]));
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

	const float ratio = float(img.height()) / float(m_bufHeight);
	const float scaleV = m_overlay->renderScale() - 1.0f;
	const float scaleH = scaleV * ratio / 2.0f;

	// fix subtitle aspect ratio as image heights can differ
	const float ratioDiff = (ratio - 1.) / 2.0f;
	m_overlayPos[0] = m_overlayPos[4] = 0.0f - ratioDiff + scaleH;
	m_overlayPos[2] = m_overlayPos[6] = 1.0f + ratioDiff - scaleH;

	// subtitle offset + margin
	const float hr = float(m_overlay->textSize().height()) / float(img.height()) + .03f;
	m_overlayPos[1] = m_overlayPos[3] = -1.0f + hr + scaleV;
	m_overlayPos[5] = m_overlayPos[7] = hr;

	asGL(glBindBuffer(GL_ARRAY_BUFFER, m_vaBuf[AV_OVRTEX]));
	asGL(glBufferData(GL_ARRAY_BUFFER, sizeof(m_overlayPos), m_overlayPos, GL_DYNAMIC_DRAW));
	asGL(glBindBuffer(GL_ARRAY_BUFFER, 0));

	// overlay
	asGL(glActiveTexture(GL_TEXTURE0 + ID_OVR));
	asGL(glBindTexture(GL_TEXTURE_2D, m_idTex[ID_OVR]));
	uploadMM<quint8, 4>(img.width(), img.height(), m_mmOvr, img.bits());
	if(m_texNeedInit) {
		asGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
		asGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
		static const float borderColor[] = { .0f, .0f, .0f, .0f };
		asGL(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor));
		asGL(glUniform1i(m_texOvr, ID_OVR));
	}
}
