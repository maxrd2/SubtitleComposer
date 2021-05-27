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

#include "zoombuffer.h"

using namespace SubtitleComposer;

ZoomBuffer::ZoomBuffer(WaveBuffer *parent)
	: QThread(parent),
	  m_waveBuffer(parent),
	  m_samplesPerPixel(0),
	  m_waveformZoomed(nullptr),
	  m_waveformZoomedSize(0),
	  m_waveform(nullptr)
{
}

ZoomBuffer::~ZoomBuffer()
{
	stopAndClear();
}

void
ZoomBuffer::stopAndClear()
{
	requestInterruption();
	wait();

	if(m_waveformZoomed) {
		m_waveformZoomedSize = 0;
		for(quint16 ch = 0; ch < m_waveBuffer->channels(); ch++)
			delete[] m_waveformZoomed[ch];
		delete[] m_waveformZoomed;
		m_waveformZoomed = nullptr;
	}

	// thread is stopped as iterators are invalidated
	m_ranges.clear();
	m_requestedRange = m_ranges.end();
}

void
ZoomBuffer::run()
{
	Q_ASSERT(m_waveform != nullptr);
	Q_ASSERT(m_samplesPerPixel > 0);
	Q_ASSERT(m_waveBuffer->lengthSamples() > 0);

	if(!m_waveformZoomed) {
		// alloc memory for zoomed pixel data
		m_waveformZoomedSize = (m_waveBuffer->lengthSamples() + m_samplesPerPixel - 1) / m_samplesPerPixel;
		m_waveformZoomed = new WaveZoomData *[m_waveBuffer->channels()];
		for(quint32 i = 0; i < m_waveBuffer->channels(); i++)
			m_waveformZoomed[i] = new WaveZoomData[m_waveformZoomedSize];
	}

	m_rangeMutex.lock();
	std::list<DataRange>::iterator range = m_ranges.end();
	for(;;) {
		m_restartProcessing = false;

		if(m_requestedRange != m_ranges.end())
			range = m_requestedRange;
		if(range == m_ranges.end())
			range = m_ranges.begin();

		// unlock here - nothing will invalidate existing iterator
		m_rangeMutex.unlock();

		if(range == m_ranges.end())
			break;
		const quint32 last = updateZoomRange(range->start, range->end);
		if(isInterruptionRequested())
			break;

		m_rangeMutex.lock();

		for(;;) {
			if(range == m_ranges.end()) {
				msleep(25); // wait a bit to get more samples and avoid thread restart
				const quint32 lastAvailable = m_waveBuffer->samplesAvailable() / m_samplesPerPixel;
				if(last != lastAvailable)
					m_ranges.push_back(DataRange{last, lastAvailable});
				break;
			}
			if(range->end <= last) {
				// whole range was processed
				if(range == m_requestedRange)
					m_requestedRange = m_ranges.end();
				range = m_ranges.erase(range);
			} else {
				if(range->start < last)
					range->start = last;
				break;
			}
		}
	}
}

void
ZoomBuffer::setWaveform(const SAMPLE_TYPE * const *waveform)
{
	QMutexLocker l(&m_publicMutex);

	stopAndClear();

	m_waveform = waveform;
}

void
ZoomBuffer::setZoomScale(quint32 samplesPerPixel)
{
	QMutexLocker l(&m_publicMutex);

	if(m_samplesPerPixel == samplesPerPixel)
		return;

	stopAndClear();

	Q_ASSERT(!isRunning());

	m_samplesPerPixel = samplesPerPixel;

	m_ranges.push_back(DataRange{0, m_waveBuffer->samplesAvailable() / m_samplesPerPixel});

	if(m_waveform)
		start();
}

void
ZoomBuffer::resume()
{
	QMutexLocker l(&m_publicMutex);

	if(m_samplesPerPixel && m_waveform)
		start();
}

quint32
ZoomBuffer::updateZoomRange(quint32 start, quint32 end)
{
	Q_ASSERT(end <= m_waveformZoomedSize);

	const quint16 channels = m_waveBuffer->channels();

	while(start < end) {
		quint32 i = start * m_samplesPerPixel;

		for(quint16 ch = 0; ch < channels; ch++) {
			const quint32 val = qAbs(qint32(m_waveform[ch][i]) - SAMPLE_MIN - (SAMPLE_MAX - SAMPLE_MIN) / 2);
			m_waveformZoomed[ch][start].min = val / m_samplesPerPixel;
			m_waveformZoomed[ch][start].max = val;
		}

		const quint32 iEnd = i + m_samplesPerPixel;
		while(++i < iEnd) {
			for(quint16 ch = 0; ch < channels; ch++) {
				const quint32 val = qAbs(qint32(m_waveform[ch][i]) - SAMPLE_MIN - (SAMPLE_MAX - SAMPLE_MIN) / 2);
				m_waveformZoomed[ch][start].min += val / m_samplesPerPixel;
				if(m_waveformZoomed[ch][start].max < val)
					m_waveformZoomed[ch][start].max = val;
			}
		}

		start++;

		if(isInterruptionRequested() || m_restartProcessing)
			break;
	}

	return start;
}

quint32
ZoomBuffer::zoomedBuffer(quint32 timeStart, quint32 timeEnd, WaveZoomData **buffers)
{
	if(!m_waveform || !m_waveformZoomed)
		return 0;

	const quint32 samplesAvailable = m_waveBuffer->samplesAvailable();
	const quint32 zoomStart = qMin(timeStart * m_waveBuffer->sampleRateMillis(), samplesAvailable) / m_samplesPerPixel;
	quint32 zoomEnd = qMin(timeEnd * m_waveBuffer->sampleRateMillis(), samplesAvailable) / m_samplesPerPixel;

	if(zoomStart == zoomEnd)
		return 0;

	m_rangeMutex.lock();
	m_requestedRange = m_ranges.begin();
	while(m_requestedRange != m_ranges.end()) {
		if(m_requestedRange->start > zoomEnd || zoomStart >= m_requestedRange->end) {
			++m_requestedRange;
			continue;
		}
		if(m_requestedRange->start < zoomStart) {
			// there are samples before requested range
			m_ranges.insert(m_requestedRange, DataRange{m_requestedRange->start, zoomStart});
			m_requestedRange->start = zoomStart;
		}
		if(m_requestedRange->end > zoomEnd) {
			// there are samples after requested range
			m_ranges.insert(std::next(m_requestedRange), DataRange{zoomEnd, m_requestedRange->end});
			m_requestedRange->end = zoomEnd;
		}
		m_restartProcessing = true;
		zoomEnd = m_requestedRange->start;
		break;
	}
	m_rangeMutex.unlock();

	for(quint16 ch = 0; ch < m_waveBuffer->channels(); ch++)
		buffers[ch] = &m_waveformZoomed[ch][zoomStart];

	return zoomEnd - zoomStart;
}
