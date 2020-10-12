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

QT_FORWARD_DECLARE_CLASS(QOpenGLShader)
QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

namespace SubtitleComposer {
class SubtitleTextOverlay;

class GLRenderer : public QOpenGLWidget, private QOpenGLFunctions
{
	Q_OBJECT

public:
    explicit GLRenderer(QWidget *parent = nullptr);
    ~GLRenderer();

	void setFrameFormat(int width, int height, int compBits, int crWidthShift, int crHeightShift);
	void setFrameY(quint8 *buf, quint32 pitch);
	void setFrameU(quint8 *buf, quint32 pitch);
	void setFrameV(quint8 *buf, quint32 pitch);
	void setOverlay(SubtitleTextOverlay *overlay);

signals:
	void resolutionChanged();

protected:
    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;

private:
	void uploadYUV();
	void uploadSubtitle();

private:
	SubtitleTextOverlay *m_overlay;
	GLfloat m_overlayPos[8];

	quint8 *m_bufYUV;
	quint32 m_bufSize;
	GLsizei m_bufWidth, m_bufHeight;
	GLsizei m_crWidth, m_crHeight;
	quint8 *m_pixels[3];
	quint32 m_pitch[3];

	QOpenGLShader *m_vertShader;
	QOpenGLShader *m_fragShader;
	QOpenGLShaderProgram *m_shaderProg;

	bool m_texNeedInit;
	GLuint m_idY, m_idU, m_idV, m_idOvr;
	int m_texY, m_texU, m_texV, m_texOvr, m_pixMultLoc;

	GLenum m_glType, m_glFormat;
	GLfloat m_pixMult;
};
}

#endif // GLRENDERER_H
