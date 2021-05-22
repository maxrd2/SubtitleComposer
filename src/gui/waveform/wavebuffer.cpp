#include "wavebuffer.h"

#include "application.h"
#include "gui/waveform/waveformwidget.h"

#include <QProgressBar>
#include <QScrollBar>
#include <QtMath>

#define MAX_WINDOW_ZOOM 3000 // TODO: calculate this when receiving stream data and do sample rate conversion
//#define SAMPLE_RATE 8000
//#define SAMPLE_RATE_MILLIS (SAMPLE_RATE / 1000)
//#define DRAG_TOLERANCE (double(10 * m_samplesPerPixel / SAMPLE_RATE_MILLIS))


namespace SubtitleComposer {
struct WaveformFrame {
	explicit WaveformFrame(quint8 shift, quint8 channels)
		: offset(0),
		  sampleShift(shift),
		  frameSize((1 << sampleShift) * channels),
		  overflow(0),
		  val(new qint32[channels])
	{
	}

	~WaveformFrame() {
		delete[] val;
	}

	quint32 offset;
	quint8 sampleShift;
	quint16 frameSize;
	quint16 overflow;
	qint32 *val;
};
}


using namespace SubtitleComposer;

WaveBuffer::WaveBuffer(WaveformWidget *parent)
	: QObject(parent),
	  m_wfWidget(parent),
	  m_stream(new StreamProcessor(this)),
	  m_waveformDuration(0),
	  m_waveformChannels(0),
	  m_waveform(nullptr),
	  m_samplesMsec(0),
	  m_wfFrame(nullptr),
	  m_samplesPerPixel(0),
	  m_waveformZoomed(nullptr),
	  m_waveformZoomedSize(0),
	  m_waveformZoomedOffsetMin(0),
	  m_waveformZoomedOffsetMax(0)
{
	connect(m_stream, &StreamProcessor::streamProgress, this, &WaveBuffer::onStreamProgress);
	connect(m_stream, &StreamProcessor::streamFinished, this, &WaveBuffer::onStreamFinished);
	// Using Qt::DirectConnection here makes WaveBuffer::onStreamData() to execute in SpeechProcessor's thread
	connect(m_stream, &StreamProcessor::audioDataAvailable, this, &WaveBuffer::onStreamData, Qt::DirectConnection);
}

quint32
WaveBuffer::millisPerPixel() const
{
	return m_samplesPerPixel / m_samplesMsec;
}

quint32
WaveBuffer::samplesAvailable() const
{
	return m_wfFrame ? m_wfFrame->offset : m_waveformChannelSize;
}

void
WaveBuffer::setAudioStream(const QString &mediaFile, int audioStream)
{
	m_waveformDuration = 0;

	static WaveFormat waveFormat(0, 0, sizeof(SAMPLE_TYPE) * 8, true);
	if(m_stream->open(mediaFile) && m_stream->initAudio(audioStream, waveFormat))
		m_stream->start();
}

void
WaveBuffer::setNullAudioStream(quint64 msecVideoLength)
{
	clearAudioStream();

	m_waveformDuration = msecVideoLength / 1000;

	emit waveformUpdated();
}

void
WaveBuffer::clearAudioStream()
{
	m_stream->close();

	if(m_waveformZoomed) {
		for(quint32 i = 0; i < m_waveformChannels; i++)
			delete[] m_waveformZoomed[i];
		delete[] m_waveformZoomed;
		m_waveformZoomed = nullptr;
	}

	if(m_waveform) {
		for(quint32 i = 0; i < m_waveformChannels; i++)
			delete[] m_waveform[i];
		delete[] m_waveform;
		m_waveform = nullptr;
		m_waveformChannelSize = 0;
		m_waveformChannels = 0;
		m_samplesMsec = 0;
	}
}

void
WaveBuffer::onStreamProgress(quint64 msecPos, quint64 msecLength)
{
	if(!m_waveformDuration) {
		m_waveformDuration = msecLength / 1000;
		m_wfWidget->m_progressBar->setRange(0, m_waveformDuration);
		m_wfWidget->m_progressWidget->show();
		m_wfWidget->m_scrollBar->setRange(0, m_waveformDuration * 1000 - m_wfWidget->windowSizeInner());
	}
	m_wfWidget->m_progressBar->setValue(msecPos / 1000);
}

void
WaveBuffer::onStreamFinished()
{
	m_wfWidget->m_progressWidget->hide();
	if(m_wfFrame) {
		m_waveformChannelSize = m_wfFrame->offset;
		delete m_wfFrame;
		m_wfFrame = nullptr;
	}
}

inline static SAMPLE_TYPE
scaleSample(SAMPLE_TYPE sample)
{
	static const qreal valMax = qreal(SAMPLE_MAX - SAMPLE_MIN) / 2.;
	return qSqrt(qreal(sample) / valMax) * SAMPLE_MAX;
}

void
WaveBuffer::onStreamData(const void *buffer, qint32 size, const WaveFormat *waveFormat, const qint64 msecStart, const qint64 /*msecDuration*/)
{
	// make sure WaveBuffer::onStreamProgress() signal was processed since we're in different thread
	while(!m_waveformDuration) {
		QThread::yieldCurrentThread();
		if(m_stream->isInterruptionRequested())
			return;
	}

	if(!m_waveformChannels) {
		m_samplesSec = waveFormat->sampleRate();
		quint8 sampleShift = 0;
		while(m_samplesSec > MAX_WINDOW_ZOOM) {
			m_samplesSec >>= 1;
			sampleShift++;
		}
		m_samplesMsec = m_samplesSec / 1000;
		m_waveformChannels = waveFormat->channels();
		m_waveformChannelSize = m_samplesSec * (m_waveformDuration + 60); // added 60sec as duration might be wrong
		m_waveform = new SAMPLE_TYPE *[m_waveformChannels];
		for(quint32 i = 0; i < m_waveformChannels; i++)
			m_waveform[i] = new SAMPLE_TYPE[m_waveformChannelSize];

		m_wfFrame = new WaveformFrame(sampleShift, m_waveformChannels);

		emit waveformUpdated();
	}

	Q_ASSERT(waveFormat->bitsPerSample() == sizeof(SAMPLE_TYPE) * 8);

	const SAMPLE_TYPE *sample = reinterpret_cast<const SAMPLE_TYPE *>(buffer);

	{ // handle overlaps and holes between buffers (tested streams had ~2ms error - might be useless)
		Q_ASSERT(msecStart >= 0);
		const quint32 inStartOffset = msecStart * m_samplesSec / 1000;
		if(inStartOffset < m_wfFrame->offset) {
			// overwrite part of local buffer
			m_wfFrame->offset = inStartOffset;
		} else if(inStartOffset > m_wfFrame->offset) {
			// pad hole in local buffer
			quint32 i = m_wfFrame->offset;
			if(i > 0) {
				for(; i < inStartOffset; i++) {
					for(quint32 c = 0; c < m_waveformChannels; c++)
						m_waveform[c][i] = m_waveform[c][i - 1];
				}
			} else {
				for(quint32 c = 0; c < m_waveformChannels; c++)
					memset(m_waveform[c], 0, inStartOffset * sizeof(SAMPLE_TYPE));
			}
			m_wfFrame->offset = inStartOffset;
		}
	}

	quint32 len = size / sizeof(SAMPLE_TYPE);

	if(m_wfFrame->overflow) {
		const quint32 overflowFrameSize = qMin(m_wfFrame->overflow + len, quint32(m_wfFrame->frameSize));

		quint32 c = m_wfFrame->overflow;
		for(; c < m_waveformChannels; c++)
			m_wfFrame->val[c] = *sample++;
		for(; c < overflowFrameSize; c++)
			m_wfFrame->val[c % m_waveformChannels] += *sample++;
		for(c = 0; c < m_waveformChannels; c++)
			m_waveform[c][m_wfFrame->offset] = scaleSample(m_wfFrame->val[c] >> m_wfFrame->sampleShift);

		len -= overflowFrameSize - m_wfFrame->overflow;
		if(overflowFrameSize < m_wfFrame->frameSize) {
			// no more data
			Q_ASSERT(len == 0);
			m_wfFrame->overflow = overflowFrameSize;
			return;
		}
		m_wfFrame->offset++;
	}

	if(m_wfFrame->offset + (len >> m_wfFrame->sampleShift) >= m_waveformChannelSize) // make sure we don't overflow
		len = (m_waveformChannelSize - m_wfFrame->offset - 1) << m_wfFrame->sampleShift;

	while(len > m_wfFrame->frameSize) {
		quint32 c = 0;
		for(; c < m_waveformChannels; c++)
			m_wfFrame->val[c] = *sample++;
		for(; c < m_wfFrame->frameSize; c++)
			m_wfFrame->val[c % m_waveformChannels] += *sample++;
		for(c = 0; c < m_waveformChannels; c++)
			m_waveform[c][m_wfFrame->offset] = scaleSample(m_wfFrame->val[c] >> m_wfFrame->sampleShift);
		len -= m_wfFrame->frameSize;
		m_wfFrame->offset++;
	}
	m_wfFrame->overflow = len;
}


void
WaveBuffer::setZoomScale(quint32 samplesPerPixel)
{
	if(m_samplesPerPixel == samplesPerPixel)
		return;

	m_samplesPerPixel = samplesPerPixel;
	m_waveformZoomedOffsetMin = m_waveformZoomedOffsetMax = 0;
	m_waveformZoomedSize = 0;
	if(m_waveformZoomed) {
		for(quint16 ch = 0; ch < m_waveformChannels; ch++)
			delete[] m_waveformZoomed[ch];
		delete[] m_waveformZoomed;
		m_waveformZoomed = nullptr;
	}

	// FIXME: start generating data in worker thread
}

void
WaveBuffer::updateZoomDataRange(quint32 iMin, quint32 iMax)
{
	Q_ASSERT(iMax / m_samplesPerPixel < m_waveformZoomedSize);

	qDebug() << "updateZoomData [" << iMin << "," << iMax << "] on " << (thread() == app()->thread() ? "app" : "some") << " thread";

	qint32 xMax = -65535, xSum = 0;

	for(quint16 ch = 0; ch < m_waveformChannels; ch++) {
		for(quint32 i = iMin; i < iMax; i++) {
			qint32 val = qAbs(qint32(m_waveform[ch][i]) - SAMPLE_MIN - (SAMPLE_MAX - SAMPLE_MIN) / 2);
			if(xMax < val)
				xMax = val;
			xSum += val;

			if(i % m_samplesPerPixel == m_samplesPerPixel - 1) {
				const int zi = i / m_samplesPerPixel;
				m_waveformZoomed[ch][zi].min = xSum / m_samplesPerPixel;
				m_waveformZoomed[ch][zi].max = xMax;

				xMax = -65535;
				xSum = 0;
			}
		}
	}
}

quint32
WaveBuffer::zoomedBuffer(quint32 timeStart, quint32 timeEnd, WaveZoomData **buffers)
{
	if(!m_waveformChannels || !m_waveform || !m_samplesPerPixel)
		return 0;

	if(!m_waveformZoomed) {
		// alloc memory for zoomed pixel data
		m_waveformZoomedOffsetMin = m_waveformZoomedOffsetMax = 0;
		m_waveformZoomedSize = m_waveformChannelSize / m_samplesPerPixel;
		if(m_waveformChannelSize % m_samplesPerPixel)
			m_waveformZoomedSize++;
		m_waveformZoomed = new WaveZoomData *[m_waveformChannels];
		for(quint32 i = 0; i < m_waveformChannels; i++)
			m_waveformZoomed[i] = new WaveZoomData[m_waveformZoomedSize];
	}

	const quint32 samplesAvailable = m_wfFrame ? m_wfFrame->offset : m_waveformChannelSize;
	Q_ASSERT(samplesAvailable <= m_waveformChannelSize);
	const quint32 sampleStart = timeStart * m_samplesMsec;
	const quint32 sampleEnd = qMin(timeEnd * m_samplesMsec, samplesAvailable);
	const quint32 zoomStart = sampleStart / m_samplesPerPixel;
	const quint32 zoomEnd = sampleEnd / m_samplesPerPixel;

	if(m_waveformZoomedOffsetMin == m_waveformZoomedOffsetMax) {
		updateZoomDataRange(sampleStart, sampleEnd);
		m_waveformZoomedOffsetMin = zoomStart;
		m_waveformZoomedOffsetMax = zoomEnd;
	} else {
		if(zoomStart < m_waveformZoomedOffsetMin) {
			updateZoomDataRange(sampleStart, m_waveformZoomedOffsetMin * m_samplesPerPixel);
			m_waveformZoomedOffsetMin = zoomStart;
		}
		if(zoomEnd > m_waveformZoomedOffsetMax) {
			updateZoomDataRange(m_waveformZoomedOffsetMax * m_samplesPerPixel, sampleEnd);
			m_waveformZoomedOffsetMax = zoomEnd;
		}
	}

	for(quint16 ch = 0; ch < m_waveformChannels; ch++)
		buffers[ch] = &m_waveformZoomed[ch][zoomStart];

	return zoomEnd - zoomStart;
}
