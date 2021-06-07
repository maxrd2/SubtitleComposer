/*
 * Copyright (c) 2003 Fabrice Bellard
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

#ifndef GLRENDERER_H
#define GLRENDERER_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <QMutex>

extern "C" {
#include <libavutil/frame.h>
}

QT_FORWARD_DECLARE_CLASS(QOpenGLShader)
QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

namespace SubtitleComposer {
class SubtitleTextOverlay;

class GLRenderer : public QOpenGLWidget, private QOpenGLFunctions
{
	Q_OBJECT

	friend class FFPlayer;

	explicit GLRenderer(QWidget *parent = nullptr);

public:
	~GLRenderer();

	static void setupProfile();
	void reset();

	void setFrameFormat(int width, int height, int compBits, int crWidthShift, int crHeightShift);
	void setColorspace(const AVFrame *frame);
	void setFrameY(quint8 *buf, quint32 pitch);
	void setFrameU(quint8 *buf, quint32 pitch);
	void setFrameV(quint8 *buf, quint32 pitch);
	void setOverlay(SubtitleTextOverlay *overlay);

	inline QMutex * mutex() { return &m_texMutex; }

signals:
	void resolutionChanged();

protected:
    void initializeGL() override;
	void initShader();
    void resizeGL(int width, int height) override;
    void paintGL() override;

private:
	template<class T, int D> void uploadMM(int texWidth, int texHeight, T *texBuf, const T *texSrc);
	void uploadYUV();
	void uploadSubtitle();

private:
	SubtitleTextOverlay *m_overlay;
	GLfloat m_overlayPos[8];
	quint8 *m_mmOvr;

	QOpenGLVertexArrayObject m_vao;

	quint8 *m_bufYUV, *m_mmYUV;
	quint32 m_bufSize;
	GLsizei m_bufWidth, m_bufHeight;
	GLsizei m_crWidth, m_crHeight;
	quint8 *m_pixels[3];
	quint32 m_pitch[3];
	QMutex m_texMutex;

	bool m_csNeedInit;
	QString m_ctfOut, m_ctfIn;
	QMatrix4x4 m_csCM;

	QOpenGLShader *m_vertShader;
	QOpenGLShader *m_fragShader;
	QOpenGLShaderProgram *m_shaderProg;

	bool m_texNeedInit;
	int m_vpWidth, m_vpHeight;
	int m_texY, m_texU, m_texV, m_texOvr;
	GLuint *m_idTex;
	GLuint *m_vaBuf;

	GLenum m_glType, m_glFormat;
};
}

#endif // GLRENDERER_H
