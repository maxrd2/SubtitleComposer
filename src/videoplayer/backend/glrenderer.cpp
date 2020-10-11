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

#include "videoplayer/backend/ffplayer.h"
#include "videoplayer/videoplayer.h"
#include "videoplayer/subtitletextoverlay.h"

#define ATTR_VIDEO_VPOS 0
#define ATTR_VIDEO_VCOL 1
#define ATTR_SUBTITLE_VCOL 2

using namespace SubtitleComposer;

GLRenderer::GLRenderer(QWidget *parent)
	: QOpenGLWidget(parent),
	  m_overlay(nullptr),
	  m_bufYUV(nullptr),
	  m_bufSize(0),
	  m_bufWidth(0),
	  m_bufHeight(0),
	  m_vertShader(nullptr),
	  m_fragShader(nullptr),
	  m_shaderProg(nullptr),
	  m_texNeedInit(true)
{
	setUpdateBehavior(NoPartialUpdate);
}

GLRenderer::~GLRenderer()
{
	delete[] m_bufYUV;
}

void
GLRenderer::setOverlay(SubtitleTextOverlay *overlay)
{
	m_overlay = overlay;
}

void
GLRenderer::displayVideoFrame(unsigned char *yuvBuf, int width, int height)
{
	setFrameSize(width, height);
	memcpy(m_bufYUV, yuvBuf, m_bufSize);
	update();
}

void
GLRenderer::setFrameSize(int width, int height)
{
	if(m_bufWidth == width && m_bufHeight == height)
		return;

	m_bufWidth = width;
	m_bufHeight = height;

	delete[] m_bufYUV;
	m_bufSize = width * height * 3 / 2;
	m_bufYUV = new quint8[m_bufSize];

	m_overlay->setImageSize(width, height);

	m_texNeedInit = true;

	emit resolutionChanged();
}

void
GLRenderer::getFrameYUV(quint8 **pixels, quint32 *pitch)
{
	// Y buffer
	quint8 *data = m_bufYUV;
	*pixels++ = data;
	*pitch++ = m_bufWidth;

	// U buffer
	data += m_bufWidth * m_bufHeight;
	*pixels++ = data;
	*pitch++ = m_bufWidth / 2;

	// V buffer
	data += m_bufWidth * m_bufHeight / 4;
	*pixels++ = data;
	*pitch++ = m_bufWidth / 2;

	// no alpha
	*pixels++ = nullptr;
	*pitch++ = 0;
}

void
GLRenderer::setFrameY(quint8 *buf, quint32 pitch)
{
	quint8 *dbuf = m_bufYUV;
	for(int i = 0; i < m_bufHeight; i++) {
		memcpy(dbuf, buf, m_bufWidth);
		dbuf += m_bufWidth;
		buf += pitch;
	}
}

void
GLRenderer::setFrameU(quint8 *buf, quint32 pitch)
{
	quint8 *dbuf = m_bufYUV + m_bufWidth * m_bufHeight;
	for(int i = 0; i < m_bufHeight; i += 2) {
		memcpy(dbuf, buf, m_bufWidth / 2);
		dbuf += m_bufWidth / 2;
		buf += pitch;
	}
}

void
GLRenderer::setFrameV(quint8 *buf, quint32 pitch)
{
	quint8 *dbuf = m_bufYUV + m_bufWidth * m_bufHeight * 5 / 4;
	for(int i = 0; i < m_bufHeight; i += 2) {
		memcpy(dbuf, buf, m_bufWidth / 2);
		dbuf += m_bufWidth / 2;
		buf += pitch;
	}
}

void
GLRenderer::initializeGL()
{
	Q_ASSERT(m_bufYUV);

	initializeOpenGLFunctions();

	glEnable(GL_DEPTH_TEST);

	delete m_vertShader;
	m_vertShader = new QOpenGLShader(QOpenGLShader::Vertex, this);
	bool success = m_vertShader->compileSourceCode(
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
		qWarning() << "Failed compiling vertex shader code";
		return;
	}

	delete m_fragShader;
	m_fragShader = new QOpenGLShader(QOpenGLShader::Fragment, this);
	success = m_fragShader->compileSourceCode(
#ifdef QT_OPENGL_ES_2
		"precision mediump float;"
#endif
		"varying vec2 vfVidTex;"
		"varying vec2 vfOvrTex;"
		"uniform sampler2D texY;"
		"uniform sampler2D texU;"
		"uniform sampler2D texV;"
		"uniform sampler2D texOvr;"
		"void main(void) {"
			"vec3 yuv;"
			"yuv.x = texture2D(texY, vfVidTex).r;"
			"yuv.y = texture2D(texU, vfVidTex).r - 0.5;"
			"yuv.z = texture2D(texV, vfVidTex).r - 0.5;"
			"vec3 rgb = mat3(1, 1, 1,"
							"0, -0.39465, 2.03211,"
							"1.13983, -0.58060, 0) * yuv;"
			"vec4 o = texture2D(texOvr, vfOvrTex);"
			"gl_FragColor = vec4(mix(rgb, o.rgb, o.w), 1);"
		"}");
	if(!success) {
		qWarning() << "Failed compiling fragment shader code";
		return;
	}

	delete m_shaderProg;
	m_shaderProg = new QOpenGLShaderProgram(this);
	m_shaderProg->addShader(m_fragShader);
	m_shaderProg->addShader(m_vertShader);

	m_shaderProg->bindAttributeLocation("vPos", ATTR_VIDEO_VPOS);
	m_shaderProg->bindAttributeLocation("vVidTex", ATTR_VIDEO_VCOL);
	m_shaderProg->bindAttributeLocation("vOvrTex", ATTR_SUBTITLE_VCOL);
	m_shaderProg->link();
	m_shaderProg->bind();

	m_texY = m_shaderProg->uniformLocation("texY");
	m_texU = m_shaderProg->uniformLocation("texU");
	m_texV = m_shaderProg->uniformLocation("texV");
	m_texOvr = m_shaderProg->uniformLocation("texOvr");

	{
		static const GLfloat v[] = {
			-1.0f, 1.0f,
			1.0f, 1.0f,
			-1.0f, -1.0f,
			1.0f, -1.0f,
		};
		glVertexAttribPointer(ATTR_VIDEO_VPOS, 2, GL_FLOAT, 0, 0, v);
	}
	glEnableVertexAttribArray(ATTR_VIDEO_VPOS);

	{
		static const GLfloat v[] = {
			0.0f,  0.0f,
			1.0f,  0.0f,
			0.0f,  1.0f,
			1.0f,  1.0f,
		};
		glVertexAttribPointer(ATTR_VIDEO_VCOL, 2, GL_FLOAT, 0, 0, v);
	}
	glEnableVertexAttribArray(ATTR_VIDEO_VCOL);

	glVertexAttribPointer(ATTR_SUBTITLE_VCOL, 2, GL_FLOAT, 0, 0, m_overlayPos);
	glEnableVertexAttribArray(ATTR_SUBTITLE_VCOL);

	glGenTextures(1, &m_idY);
	glGenTextures(1, &m_idU);
	glGenTextures(1, &m_idV);
	glGenTextures(1, &m_idOvr);

	glClearColor(.0f, .0f, .0f, .0f); // background color

	glPixelStorei(GL_UNPACK_ALIGNMENT, m_bufWidth % 4 == 0 ? 4 : 1);

	m_texNeedInit = true;
}

void
GLRenderer::resizeGL(int width, int height)
{
	glViewport(0, 0, width, height);
	update();
}

void
GLRenderer::paintGL()
{
	Q_ASSERT(m_bufYUV);

	uploadYUV();
	uploadSubtitle();

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	m_texNeedInit = false;
}

void
GLRenderer::uploadYUV()
{
	const quint8 *yuvData = m_bufYUV;

	// load Y data
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_idY);

	if(m_texNeedInit) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, m_bufWidth, m_bufHeight, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, yuvData);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glUniform1i(m_texY, 0); // GL_TEXTURE0
	} else {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_bufWidth, m_bufHeight, GL_LUMINANCE, GL_UNSIGNED_BYTE, yuvData);
	}

	// skip Y data
	yuvData += m_bufWidth * m_bufHeight;

	// load U data
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_idU);

	if(m_texNeedInit) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, m_bufWidth / 2, m_bufHeight / 2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, yuvData);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glUniform1i(m_texU, 1); // GL_TEXTURE1
	} else {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_bufWidth / 2, m_bufHeight / 2, GL_LUMINANCE, GL_UNSIGNED_BYTE, yuvData);
	}

	// skip U data
	yuvData += m_bufWidth * m_bufHeight / 4;

	// load V data
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_idV);

	if(m_texNeedInit) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, m_bufWidth / 2, m_bufHeight / 2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, yuvData);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glUniform1i(m_texV, 2); // GL_TEXTURE2
	} else {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_bufWidth / 2, m_bufHeight / 2, GL_LUMINANCE, GL_UNSIGNED_BYTE, yuvData);
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

	// overlay
	glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_idOvr);

	if(m_texNeedInit) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, img.width(), img.height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, img.bits());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		static const float borderColor[] = { .0f, .0f, .0f, .0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		glUniform1i(m_texOvr, 3); // GL_TEXTURE3
	} else {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img.width(), img.height(), GL_BGRA, GL_UNSIGNED_BYTE, img.bits());
	}
}
