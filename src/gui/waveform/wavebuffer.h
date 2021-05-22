#ifndef WAVEBUFFER_H
#define WAVEBUFFER_H
/*
 * Copyright (C) 2010-2021 Mladen Milinkovic <max@smoothware.net>
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

#include "streamprocessor/streamprocessor.h"

#include <QObject>

// FIXME: make sample size configurable or drop this
//*
#define SAMPLE_TYPE qint16
#define SAMPLE_MIN -32768
#define SAMPLE_MAX 32767
/*/
#define SAMPLE_TYPE quint8
#define SAMPLE_MIN 0
#define SAMPLE_MAX 255
//*/

namespace SubtitleComposer {
class WaveformWidget;

struct WaveZoomData {
	qint32 min;
	qint32 max;
};

class WaveBuffer : public QObject
{
	Q_OBJECT

public:
	explicit WaveBuffer(WaveformWidget *parent = nullptr);

	/**
	 * @brief waveformDuration
	 * @return seconds FIXME: make it milliseconds
	 */
	inline quint32 waveformDuration() const { return m_waveformDuration; }

	inline quint32 lengthMillis() const { return m_waveformChannelSize / m_samplesMsec; }
	inline quint32 lengthSamples() const { return m_waveformChannelSize; }

	/**
	 * @brief sampleRateMillis
	 * @return number of samples per millisecond
	 */
	inline quint32 sampleRateMillis() const { return m_samplesMsec; }

	quint32 millisPerPixel() const;

	inline quint16 channels() const { return m_waveformChannels; }

	/**
	 * @brief MAX_WINDOW_ZOOM
	 * @return FIXME:
	 */
	inline static quint32 MAX_WINDOW_ZOOM() { return 3000; }

	quint32 samplesAvailable() const;

	void setAudioStream(const QString &mediaFile, int audioStream);
	void setNullAudioStream(quint64 msecVideoLength);
	void clearAudioStream();

	void setZoomScale(quint32 samplesPerPixel);

	quint32 zoomedBuffer(quint32 timeStart, quint32 timeEnd, WaveZoomData **buffers);

signals:
	void waveformUpdated();

private:
	void onStreamData(const void *buffer, qint32 size, const WaveFormat *waveFormat, const qint64 msecStart, const qint64 msecDuration);
	void onStreamProgress(quint64 msecPos, quint64 msecLength);
	void onStreamFinished();

	void updateZoomDataRange(quint32 iMin, quint32 iMax);

private:
	WaveformWidget *m_wfWidget;

	StreamProcessor *m_stream;

	quint32 m_waveformDuration; // FIXME: change to msec
	quint16 m_waveformChannels;
	quint32 m_waveformChannelSize;
	SAMPLE_TYPE **m_waveform;

	quint32 m_samplesMsec;
	quint32 m_samplesSec;

	struct WaveformFrame *m_wfFrame;

	quint32 m_samplesPerPixel;
	WaveZoomData **m_waveformZoomed;
	quint32 m_waveformZoomedSize;
	quint32 m_waveformZoomedOffsetMin;
	quint32 m_waveformZoomedOffsetMax;
};
}

#endif // WAVEBUFFER_H
