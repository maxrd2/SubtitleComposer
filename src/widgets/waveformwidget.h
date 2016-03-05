#ifndef WAVEFORMWIDGET_H
#define WAVEFORMWIDGET_H

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../core/time.h"
#include "../core/subtitle.h"
#include "../videoplayer/waveformat.h"
#include "../streamprocessor/streamprocessor.h"

#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QRegion)
QT_FORWARD_DECLARE_CLASS(QPolygon)
QT_FORWARD_DECLARE_CLASS(QProgressBar)

// FIXME: make sample size configurable or drop this
/*
#define BYTES_PER_SAMPLE 2
#define SAMPLE_TYPE qint16
#define SIGNED_PAD 0 // 32768
#define SAMPLE_MAX 32768 // 65535
/*/
#define BYTES_PER_SAMPLE 1
#define SAMPLE_TYPE quint8
#define SIGNED_PAD -128 // 0
#define SAMPLE_MAX 128 // 255
//*/
#define SAMPLE_RATE_MILIS (8000 / 1000)

namespace SubtitleComposer {
class WaveformWidget : public QWidget
{
	Q_OBJECT

public:
	WaveformWidget(QWidget *parent);
	virtual ~WaveformWidget();

	Time windowSize() const;
	void setWindowSize(const Time &size);

	QWidget *progressWidget();

public slots:
	void setSubtitle(Subtitle *subtitle = 0);
	void setAudioStream(const QString &mediaFile, int audioStream);
	void clearAudioStream();

protected:
	virtual void paintEvent(QPaintEvent *);
	virtual void resizeEvent(QResizeEvent *e);

private slots:
	void onPlayerPositionChanged(double seconds);
	void onStreamData(const void *buffer, const qint32 size, const WaveFormat *waveFormat);
	void onStreamProgress(quint64 msecPos, quint64 msecLength);
	void onStreamFinished();

private:
	QString m_mediaFile;
	int m_streamIndex;

	StreamProcessor *m_stream;
	Subtitle *m_subtitle;

	Time m_timeStart;
	Time m_timeCurrent;
	Time m_timeEnd;

	qint64 m_waveformDuration;
	quint32 m_waveformDataOffset;
	int m_waveformChannels;
	SAMPLE_TYPE **m_waveform;

	QWidget *m_progressWidget;
	QProgressBar *m_progressBar;
};
}
#endif
