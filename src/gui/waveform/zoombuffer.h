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

#ifndef ZOOMBUFFER_H
#define ZOOMBUFFER_H

#include <list>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>

#include "gui/waveform/wavebuffer.h"

namespace SubtitleComposer {
class ZoomBuffer : public QThread
{
	Q_OBJECT

private:
	explicit ZoomBuffer(WaveBuffer *parent);
	virtual ~ZoomBuffer();

	void resume();
	void run() override;
	void setWaveform(const SAMPLE_TYPE * const *waveform);
	quint32 updateZoomRange(quint32 start, quint32 end);
	void stopAndClear();

public:
	void setZoomScale(quint32 samplesPerPixel);
	quint32 zoomedBuffer(quint32 timeStart, quint32 timeEnd, WaveZoomData **buffers);

private:
	WaveBuffer *m_waveBuffer;

	quint32 m_samplesPerPixel;
	WaveZoomData **m_waveformZoomed;
	quint32 m_waveformZoomedSize;
	const SAMPLE_TYPE * const *m_waveform;

	QMutex m_publicMutex;

	struct DataRange {
		quint32 start;
		quint32 end;
	};
	QMutex m_rangeMutex;
	std::list<DataRange> m_ranges;
	std::list<DataRange>::iterator m_requestedRange = m_ranges.end();
	bool m_restartProcessing;

	friend class WaveBuffer;
};
}

#endif // ZOOMBUFFER_H
