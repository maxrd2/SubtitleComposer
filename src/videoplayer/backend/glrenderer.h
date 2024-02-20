/*
    SPDX-FileCopyrightText: 2003 Fabrice Bellard
    SPDX-FileCopyrightText: 2020-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
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

struct AVPixFmtDescriptor;
struct SwsContext;

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
	int uploadTexture(AVFrame *frame);
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
	template<class T, int D> void uploadMM(int texWidth, int texHeight, T *texBuf, const T *texSrc, int vpWidth, int vpHeight);
	void uploadYUV();
	void uploadSubtitle();
	bool validTextureFormat(const AVPixFmtDescriptor *fd);

private:
	SubtitleTextOverlay *m_overlay;
	GLfloat m_overlayPos[8] = {0};
	quint8 *m_mmOvr;

	QOpenGLVertexArrayObject m_vao;

	SwsContext *m_frameConvCtx;
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
	bool m_texUploaded;
	int m_lastFormat;
	int m_vpWidth, m_vpHeight;
	int m_texY, m_texU, m_texV, m_texOvr;
	GLuint *m_idTex;
	GLuint *m_vaBuf;

	GLenum m_glType, m_glFormat;
};
}

#endif // GLRENDERER_H
