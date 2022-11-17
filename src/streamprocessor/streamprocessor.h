/*
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STREAMPROCESSOR_H
#define STREAMPROCESSOR_H

#include "videoplayer/waveformat.h"

#include <QThread>
#include <QString>
#include <QStringList>
#include <QPixmap>

QT_FORWARD_DECLARE_CLASS(QTimer)

QT_FORWARD_DECLARE_STRUCT(AVFormatContext)
typedef struct AVFormatContext AVFormatContext;
QT_FORWARD_DECLARE_STRUCT(AVCodecContext)
typedef struct AVCodecContext AVCodecContext;
QT_FORWARD_DECLARE_STRUCT(AVStream)
typedef struct AVStream AVStream;
QT_FORWARD_DECLARE_STRUCT(SwrContext)
typedef struct SwrContext SwrContext;

namespace SubtitleComposer {

class StreamProcessor : public QThread
{
	Q_OBJECT

public:
	StreamProcessor(QObject *parent=NULL);
	virtual ~StreamProcessor();

	bool open(const QString &filename);
	bool initAudio(int streamIndex, const WaveFormat &waveFormat);
	bool initImage(int streamIndex);
	bool initText(int streamIndex);
	Q_INVOKABLE void close();

	QStringList listAudio();
	QStringList listText();
	QStringList listImage();

	bool start();

signals:
	void audioDataAvailable(const void *buffer, const qint32 size, const WaveFormat *waveFormat, const qint64 msecStart, const qint64 msecDuration);
	void textDataAvailable(const QString &text, const quint64 msecStart, const quint64 msecDuration);
	void imageDataAvailable(const QImage &image, const quint64 msecStart, const quint64 msecDuration);
	void streamProgress(quint64 msecPosition, quint64 msecLength);
	void streamError(int code, const QString &message, const QString &debug);
	void streamFinished();

protected:
	int findStream(int streamType, int streamIndex, bool imageSub);
	void processAudio();
	void processText();
    virtual void run() override;

private:
	bool m_opened;
	QString m_filename;

	bool m_audioReady;
	int m_audioStreamIndex;
	int m_audioStreamCurrent;
	WaveFormat m_audioStreamFormat;

	bool m_imageReady;
	int m_imageStreamIndex;
	int m_imageStreamCurrent;

	bool m_textReady;
	int m_textStreamIndex;
	int m_textStreamCurrent;

	quint64 m_streamPos;
	quint64 m_streamLen;

	AVFormatContext *m_avFormat;
	AVStream *m_avStream;
	AVCodecContext *m_codecCtx;
	SwrContext *m_swResample;
	int m_audioSampleFormat;
	uint64_t m_audioChannelLayout;
};

}

#endif // STREAMPROCESSOR_H
