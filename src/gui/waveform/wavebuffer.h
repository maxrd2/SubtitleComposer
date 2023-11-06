/*
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WAVEBUFFER_H
#define WAVEBUFFER_H

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
class ZoomBuffer;

struct WaveZoomData {
	quint32 min;
	quint32 max;
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

	inline quint32 lengthMillis() const { return m_waveformChannelSize * 1000 / m_samplesSec; }
	inline quint32 lengthSamples() const { return m_waveformChannelSize; }

	/**
	 * @brief sampleRateMillis
	 * @return number of samples per second
	 */
	inline quint32 sampleRate() const { return m_samplesSec; }

	quint32 millisPerPixel() const;

	inline quint16 channels() const { return m_waveformChannels; }

	inline bool isDecoding() const { return m_wfFrame != nullptr; }

	/**
	 * @brief MAX_WINDOW_ZOOM
	 * @return FIXME:
	 */
	inline static quint32 MAX_WINDOW_ZOOM() { return 3000; }

	quint32 samplesAvailable() const;

	void setAudioStream(const QString &mediaFile, int audioStream);
	void setNullAudioStream(quint64 msecVideoLength);
	void clearAudioStream();

	inline ZoomBuffer * zoomBuffer() const { return m_zoomBuffer; }

signals:
	void waveformUpdated();

private:
	void onStreamData(const void *buffer, qint32 size, const WaveFormat *waveFormat, const qint64 msecStart, const qint64 msecDuration);
	void onStreamProgress(quint64 msecPos, quint64 msecLength);
	void onStreamFinished();

private:
	WaveformWidget *m_wfWidget;

	StreamProcessor *m_stream;

	quint32 m_waveformDuration; // FIXME: change to msec
	quint16 m_waveformChannels;
	quint32 m_waveformChannelSize;
	SAMPLE_TYPE **m_waveform;

	quint32 m_samplesSec;

	struct WaveformFrame *m_wfFrame;

	ZoomBuffer *m_zoomBuffer;
};
}

#endif // WAVEBUFFER_H
