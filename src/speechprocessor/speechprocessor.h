#ifndef SPEECHPROCESSOR_H
#define SPEECHPROCESSOR_H

/**
 * Copyright (C) 2010-2016 Mladen Milinkovic <max@smoothware.net>
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

#include "core/time.h"
#include "core/subtitle.h"
#include "videoplayer/waveformat.h"
#include "streamprocessor/streamprocessor.h"

#include <QList>

QT_FORWARD_DECLARE_CLASS(QWidget)
QT_FORWARD_DECLARE_CLASS(QProgressBar)

typedef struct ps_decoder_s ps_decoder_t;
typedef struct cmd_ln_s cmd_ln_t;
typedef struct SpeexPreprocessState_ SpeexPreprocessState;

namespace SubtitleComposer {
class SpeechProcessor : public QObject
{
	Q_OBJECT

public:
	explicit SpeechProcessor(QWidget *parent = NULL);
	virtual ~SpeechProcessor();

	QWidget * progressWidget();

public slots:
	void setSubtitle(Subtitle *subtitle = NULL);
	void setAudioStream(const QString &mediaFile, int audioStream);
	void clearAudioStream();

signals:
	void onError(const QString &message);

private slots:
	void onStreamProgress(quint64 msecPos, quint64 msecLength);
	void onStreamError(int code, const QString &message, const QString &debug);
	void onStreamFinished();
	void onStreamData(const void *buffer, const qint32 size, const WaveFormat *waveFormat, const quint64 msecStart, const quint64 msecDuration);

private:
	void processSamples(qint16 *sampleData, qint16 sampleCount, const quint64 msecStart);

private:
	QString m_mediaFile;
	int m_streamIndex;

	StreamProcessor *m_stream;
	Subtitle *m_subtitle;

	quint32 m_audioDuration;

	QWidget *m_progressWidget;
	QProgressBar *m_progressBar;

	SpeexPreprocessState *m_speexPP;
	cmd_ln_t *m_psConfig;
	ps_decoder_t *m_psDecoder;

	qint16 *m_sampleFrame;
	qint16 m_sampleFrameLen;
	bool m_lastFrameWasSilence;

	double m_timeStart;
	double m_timeDuration;
};
}

#endif // SPEECHPROCESSOR_H
