/*
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "zoombuffer.h"

#include <list>

struct DataRange {
	quint32 start;
	quint32 end;
};
typedef std::list<DataRange> RangeList;

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
		m_reqLen = nullptr;
	}
}

void
ZoomBuffer::start()
{
	if(!m_samplesPerPixel || !m_waveform)
		return;

	Q_ASSERT(m_waveformZoomed == nullptr);

	// alloc memory for zoomed pixel data
	m_waveformZoomedSize = (m_waveBuffer->lengthSamples() + m_samplesPerPixel - 1) / m_samplesPerPixel;
	m_waveformZoomed = new WaveZoomData *[m_waveBuffer->channels()];
	for(quint32 i = 0; i < m_waveBuffer->channels(); i++)
		m_waveformZoomed[i] = new WaveZoomData[m_waveformZoomedSize];
	emit zoomedBufferReady();

	QThread::start();
}

void
ZoomBuffer::run()
{
	Q_ASSERT(m_waveform != nullptr);
	Q_ASSERT(m_waveformZoomed != nullptr);
	Q_ASSERT(m_samplesPerPixel > 0);
	Q_ASSERT(m_waveBuffer->lengthSamples() > 0);

	m_restartProcessing = false;

	RangeList ranges;
	quint32 lastProcessed = 0;
	RangeList::iterator range = ranges.end();
	for(;;) {
		// wait for more samples if there aren't any
		if(ranges.empty()) {
			while(!isInterruptionRequested()) {
				const bool decoding = m_waveBuffer->isDecoding();
				const quint32 lastAvailable = m_waveBuffer->samplesAvailable() / m_samplesPerPixel;
				if(lastProcessed != lastAvailable) {
					ranges.push_back(DataRange{lastProcessed, lastAvailable});
					break;
				}
				if(!decoding)
					break;
				msleep(100);
			}
		}

		// process requested range
		if(m_reqLen) {
			QMutexLocker l(&m_reqMutex);

			m_restartProcessing = false;

			RangeList::iterator reqRange = ranges.begin();
			while(reqRange != ranges.end()) {
				if(reqRange->start >= m_reqEnd || m_reqStart >= reqRange->end) {
					++reqRange;
					continue;
				}
				if(reqRange->start < m_reqStart) {
					// there are samples before requested range
					ranges.insert(reqRange, DataRange{reqRange->start, m_reqStart});
					reqRange->start = m_reqStart;
				}
				if(reqRange->end > m_reqEnd) {
					// there are samples after requested range
					ranges.insert(std::next(reqRange), DataRange{m_reqEnd, reqRange->end});
					reqRange->end = m_reqEnd;
				}
				*m_reqLen = reqRange->start - m_reqStart;
				emit zoomedBufferReady();
				break;
			}

			if(reqRange == ranges.end()) {
				const quint32 lastAvailable = m_waveBuffer->samplesAvailable() / m_samplesPerPixel;
				if(lastAvailable < m_reqEnd) {
					*m_reqLen = lastAvailable - qMin(m_reqStart, lastAvailable);
				} else {
					// got whole range... do not check no more
					*m_reqLen = m_reqEnd - m_reqStart;
					m_reqLen = nullptr;
				}
				emit zoomedBufferReady();
			} else {
				range = reqRange;
			}
		}

		// there's nothing left
		if(ranges.empty() || isInterruptionRequested())
			break;

		// otherwise process what is left
		if(range == ranges.end())
			range = ranges.begin();

		Q_ASSERT(range != ranges.end());

		updateZoomRange(&range->start, range->end);
		lastProcessed = range->start;

		// whole range was processed
		if(range->start == range->end)
			range = ranges.erase(range);
	}
}

void
ZoomBuffer::updateZoomRange(quint32 *start, quint32 end)
{
	Q_ASSERT(end <= m_waveformZoomedSize);

	const quint16 channels = m_waveBuffer->channels();

	while(*start < end) {
		quint32 i = *start * m_samplesPerPixel;

		for(quint16 ch = 0; ch < channels; ch++) {
			const quint32 val = qAbs(qint32(m_waveform[ch][i]) - SAMPLE_MIN - (SAMPLE_MAX - SAMPLE_MIN) / 2);
			m_waveformZoomed[ch][*start].min = val / m_samplesPerPixel;
			m_waveformZoomed[ch][*start].max = val;
		}

		const quint32 iEnd = i + m_samplesPerPixel;
		while(++i < iEnd) {
			for(quint16 ch = 0; ch < channels; ch++) {
				const quint32 val = qAbs(qint32(m_waveform[ch][i]) - SAMPLE_MIN - (SAMPLE_MAX - SAMPLE_MIN) / 2);
				m_waveformZoomed[ch][*start].min += val / m_samplesPerPixel;
				if(m_waveformZoomed[ch][*start].max < val)
					m_waveformZoomed[ch][*start].max = val;
			}
		}

		(*start)++;

		if(isInterruptionRequested() || m_restartProcessing)
			break;
	}
}

void
ZoomBuffer::setWaveform(const SAMPLE_TYPE * const *waveform)
{
	QMutexLocker l(&m_publicMutex);

	stopAndClear();

	m_waveform = waveform;

	start();
}

void
ZoomBuffer::setZoomScale(quint32 samplesPerPixel)
{
	QMutexLocker l(&m_publicMutex);

	if(m_samplesPerPixel == samplesPerPixel)
		return;

	stopAndClear();

	m_samplesPerPixel = samplesPerPixel;

	start();
}

void
ZoomBuffer::zoomedBuffer(quint32 timeStart, quint32 timeEnd, WaveZoomData **buffers, quint32 *bufLen)
{
	*bufLen = 0;

	if(!m_waveform || !m_waveformZoomed)
		return;

	QMutexLocker l(&m_reqMutex);

	m_reqStart = qMin(timeStart * m_waveBuffer->sampleRate() / m_samplesPerPixel / 1000, m_waveformZoomedSize);
	m_reqEnd = qMin(timeEnd * m_waveBuffer->sampleRate() / m_samplesPerPixel / 1000, m_waveformZoomedSize);
	m_reqLen = bufLen;

	for(quint16 ch = 0; ch < m_waveBuffer->channels(); ch++)
		buffers[ch] = &m_waveformZoomed[ch][m_reqStart];

	if(isFinished()) {
		const quint32 lastAvailable = m_waveBuffer->samplesAvailable() / m_samplesPerPixel;
		*bufLen = qMin(m_reqEnd, lastAvailable) - qMin(m_reqStart, lastAvailable);
	} else {
		*bufLen = 0;
		m_restartProcessing = true;
	}
}
