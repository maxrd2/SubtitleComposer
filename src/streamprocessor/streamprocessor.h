#ifndef STREAMPROCESSOR_H
#define STREAMPROCESSOR_H

/*
 * Copyright (C) 2010-2017 Mladen Milinkovic <max@smoothware.net>
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

#include <QObject>
#include <QString>

#include <gst/gst.h>

QT_FORWARD_DECLARE_CLASS(QTimer)

namespace SubtitleComposer {

class StreamProcessor : public QObject
{
	Q_OBJECT

public:
	StreamProcessor(QObject *parent=NULL);
	virtual ~StreamProcessor();

	bool open(const QString &filename);
	bool initAudio(const int streamIndex, const WaveFormat &waveFormat);
	bool initText(const int streamIndex);
	void close();

	bool start();

signals:
	void audioDataAvailable(const void *buffer, const qint32 size, const WaveFormat *waveFormat, const quint64 msecStart, const quint64 msecDuration);
	void textDataAvailable(const QString &text, const quint64 msecStart, const quint64 msecDuration);
	void streamProgress(quint64 msecPosition, quint64 msecLength);
	void streamError(int code, const QString &message, const QString &debug);
	void streamFinished();

private:
	static void onAudioDataReady(GstElement *fakesrc, GstBuffer *buffer, GstPad *pad, gpointer userData);
	static void onTextDataReady(GstElement *fakesrc, GstBuffer *buffer, GstPad *pad, gpointer userData);
	static void onPadAdded(GstElement *decodebin, GstPad *pad, gpointer userData);
	static gboolean onPadCheck(GstElement *decodebin, GstPad *pad, GstCaps *caps, gpointer userData);
	void decoderMessageProc();

private:
	bool m_opened;
	QString m_filename;

	bool m_audioReady;
	int m_audioStreamIndex;
	int m_audioStreamCurrent;
	WaveFormat m_audioStreamFormat;

	bool m_textReady;
	int m_textStreamIndex;
	int m_textStreamCurrent;

	quint64 m_streamPos;
	quint64 m_streamLen;

	GstPipeline *m_decodingPipeline;
	GstBus *m_decodingBus;
	QTimer *m_decodingTimer;
};

}

#endif // STREAMPROCESSOR_H
