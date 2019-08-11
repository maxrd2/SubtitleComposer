#ifndef STREAMPROCESSOR_H
#define STREAMPROCESSOR_H

/*
 * Copyright (C) 2010-2019 Mladen Milinkovic <max@smoothware.net>
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
