/*
    SPDX-FileCopyrightText: 2010-2021 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ZOOMBUFFER_H
#define ZOOMBUFFER_H

#include <QMutex>
#include <QThread>
#include <QWaitCondition>

#include "gui/waveform/wavebuffer.h"

namespace SubtitleComposer {
class ZoomBuffer : public QThread
{
	Q_OBJECT

public:
	explicit ZoomBuffer(WaveBuffer *parent);
	virtual ~ZoomBuffer();

	void setWaveform(const SAMPLE_TYPE * const *waveform);
	void setZoomScale(quint32 samplesPerPixel);
	void zoomedBuffer(quint32 timeStart, quint32 timeEnd, WaveZoomData **buffers, quint32 *bufLen);

	inline quint32 samplesPerPixel() const { return m_samplesPerPixel; }

private:
	void run() override;
	void updateZoomRange(quint32 *start, quint32 end);
	void stopAndClear();
	void start();

signals:
	void zoomedBufferReady();

private:
	WaveBuffer *m_waveBuffer;

	quint32 m_samplesPerPixel;
	WaveZoomData **m_waveformZoomed;
	quint32 m_waveformZoomedSize;
	const SAMPLE_TYPE * const *m_waveform;

	QMutex m_publicMutex;

	bool m_restartProcessing;

	QMutex m_reqMutex;
	quint32 m_reqStart = 0;
	quint32 m_reqEnd = 0;
	quint32 *m_reqLen = nullptr;
};
}

#endif // ZOOMBUFFER_H
