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

#include "videoplayer/backend/ffplayer.h"
#include "videoplayer/videoplayer.h"
#include "videoplayer/subtitletextoverlay.h"

using namespace SubtitleComposer;

enum { ID_Y, ID_U, ID_V, ID_OVR, ID_SIZE };
enum { AV_POS, AV_VIDTEX, AV_OVRTEX, A_SIZE };

GLRenderer::GLRenderer(QWidget *parent)
	: QOpenGLWidget(parent),
	  m_overlay(nullptr),
	  m_bufYUV(nullptr),
	  m_bufSize(0),
	  m_bufWidth(0),
	  m_bufHeight(0),
	  m_crWidth(0),
	  m_crHeight(0),
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
	if(m_idTex) {
		glDeleteTextures(ID_SIZE, m_idTex);
		delete[] m_idTex;
	}
	if(m_vaBuf) {
		glDeleteBuffers(A_SIZE, m_vaBuf);
		delete[] m_vaBuf;
	}
	delete[] m_bufYUV;
}

void
GLRenderer::setOverlay(SubtitleTextOverlay *overlay)
{
	m_overlay = overlay;
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
	m_pixMult = double(1 << (compBytes << 3)) / double(1 << compBits);

	delete[] m_bufYUV;
	m_bufSize = bufSize;
	m_bufYUV = new quint8[m_bufSize];

	m_overlay->setImageSize(width, height);

	m_pitch[0] = m_bufWidth * compBytes;
	m_pitch[1] = m_pitch[2] = m_crWidth * compBytes;

	m_pixels[0] = m_bufYUV;
	m_pixels[1] = m_pixels[0] + m_pitch[0] * m_bufHeight;
	m_pixels[2] = m_pixels[1] + m_pitch[1] * m_crHeight;

	m_texNeedInit = true;

	emit resolutionChanged();
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
GLRenderer::initializeGL()
{
	QMutexLocker l(&m_texMutex);

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
		"uniform float pixMult;"
		"void main(void) {"
			"vec3 yuv;"
			"yuv.x = texture2D(texY, vfVidTex).r * pixMult;"
			"yuv.y = texture2D(texU, vfVidTex).r * pixMult - 0.5;"
			"yuv.z = texture2D(texV, vfVidTex).r * pixMult - 0.5;"
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

	m_shaderProg->bindAttributeLocation("vPos", AV_POS);
	m_shaderProg->bindAttributeLocation("vVidTex", AV_VIDTEX);
	m_shaderProg->bindAttributeLocation("vOvrTex", AV_OVRTEX);
	m_shaderProg->link();
	m_shaderProg->bind();

	m_texY = m_shaderProg->uniformLocation("texY");
	m_texU = m_shaderProg->uniformLocation("texU");
	m_texV = m_shaderProg->uniformLocation("texV");
	m_texOvr = m_shaderProg->uniformLocation("texOvr");
	m_pixMultLoc = m_shaderProg->uniformLocation("pixMult");

	delete[] m_vaBuf;
	m_vaBuf = new GLuint[A_SIZE];
	glGenBuffers(A_SIZE, m_vaBuf);

	{
		static const GLfloat v[] = {
			-1.0f, 1.0f,
			1.0f, 1.0f,
			-1.0f, -1.0f,
			1.0f, -1.0f,
		};
		glBindBuffer(GL_ARRAY_BUFFER, m_vaBuf[AV_POS]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
		glEnableVertexAttribArray(AV_POS);
		glVertexAttribPointer(AV_POS, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	}

	{
		static const GLfloat v[] = {
			0.0f,  0.0f,
			1.0f,  0.0f,
			0.0f,  1.0f,
			1.0f,  1.0f,
		};
		glBindBuffer(GL_ARRAY_BUFFER, m_vaBuf[AV_VIDTEX]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
		glEnableVertexAttribArray(AV_VIDTEX);
		glVertexAttribPointer(AV_VIDTEX, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_vaBuf[AV_OVRTEX]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_overlayPos), m_overlayPos, GL_STATIC_DRAW);
	glEnableVertexAttribArray(AV_OVRTEX);
	glVertexAttribPointer(AV_OVRTEX, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	delete[] m_idTex;
	m_idTex = new GLuint[ID_SIZE];
	glGenTextures(ID_SIZE, m_idTex);

	glClearColor(.0f, .0f, .0f, .0f); // background color

	m_texNeedInit = true;
}

void
GLRenderer::resizeGL(int width, int height)
{
	QMutexLocker l(&m_texMutex);
	glViewport(0, 0, width, height);
	m_texNeedInit = true;
	update();
}

void
GLRenderer::paintGL()
{
	QMutexLocker l(&m_texMutex);

	Q_ASSERT(m_bufYUV);

	uploadYUV();
	uploadSubtitle();

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	m_texNeedInit = false;
}

void
GLRenderer::uploadYUV()
{
	if(m_texNeedInit) {
		glUniform1f(m_pixMultLoc, m_pixMult);
		glPixelStorei(GL_UNPACK_ALIGNMENT, m_bufWidth % 4 == 0 ? 4 : 1);
	}

	// load Y data
	glActiveTexture(GL_TEXTURE0 + ID_Y);
	glBindTexture(GL_TEXTURE_2D, m_idTex[ID_Y]);
	if(m_texNeedInit) {
		glTexImage2D(GL_TEXTURE_2D, 0, m_glFormat, m_bufWidth, m_bufHeight, 0, GL_RED, m_glType, m_pixels[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glUniform1i(m_texY, ID_Y);
	} else {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_bufWidth, m_bufHeight, GL_RED, m_glType, m_pixels[0]);
	}

	// load U data
	glActiveTexture(GL_TEXTURE0 + ID_U);
	glBindTexture(GL_TEXTURE_2D, m_idTex[ID_U]);
	if(m_texNeedInit) {
		glTexImage2D(GL_TEXTURE_2D, 0, m_glFormat, m_crWidth, m_crHeight, 0, GL_RED, m_glType, m_pixels[1]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glUniform1i(m_texU, ID_U);
	} else {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_crWidth, m_crHeight, GL_RED, m_glType, m_pixels[1]);
	}

	// load V data
	glActiveTexture(GL_TEXTURE0 + ID_V);
	glBindTexture(GL_TEXTURE_2D, m_idTex[ID_V]);
	if(m_texNeedInit) {
		glTexImage2D(GL_TEXTURE_2D, 0, m_glFormat, m_crWidth, m_crHeight, 0, GL_RED, m_glType, m_pixels[2]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glUniform1i(m_texV, ID_V);
	} else {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_crWidth, m_crHeight, GL_RED, m_glType, m_pixels[2]);
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
	glActiveTexture(GL_TEXTURE0 + ID_OVR);
	glBindTexture(GL_TEXTURE_2D, m_idTex[ID_OVR]);
	if(m_texNeedInit) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, img.width(), img.height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, img.bits());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		static const float borderColor[] = { .0f, .0f, .0f, .0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		glUniform1i(m_texOvr, ID_OVR);
	} else {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img.width(), img.height(), GL_BGRA, GL_UNSIGNED_BYTE, img.bits());
	}
}
