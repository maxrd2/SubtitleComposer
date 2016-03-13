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

#include "waveformwidget.h"
#include "../core/subtitleline.h"
#include "../videoplayer/videoplayer.h"

#include <QRect>
#include <QPainter>
#include <QPaintEvent>
#include <QRegion>
#include <QPolygon>
#include <QThread>

#include <QProgressBar>
#include <QLabel>
#include <QBoxLayout>

#include <QDebug>

#include <KLocalizedString>

using namespace SubtitleComposer;

WaveformWidget::WaveformWidget(QWidget *parent)
	: QWidget(parent),
	  m_mediaFile(QString()),
	  m_streamIndex(-1),
	  m_stream(new StreamProcessor()),
	  m_subtitle(NULL),
	  m_timeStart(0),
	  m_timeCurrent(3000),
	  m_timeEnd(6000),
	  m_waveformChannels(0),
	  m_waveform(NULL),
	  m_progressWidget(new QWidget(this))
{
	m_progressWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	m_progressWidget->hide();

	QLabel *label = new QLabel(i18n("Generating waveform"), m_progressWidget);

	m_progressBar = new QProgressBar(m_progressWidget);
	m_progressBar->setMinimumWidth(300);
	m_progressBar->setTextVisible(true);

	QLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight, m_progressWidget);
	layout->setContentsMargins(1, 0, 1, 0);
	layout->setSpacing(1);
	layout->addWidget(label);
	layout->addWidget(m_progressBar);

	connect(VideoPlayer::instance(), &VideoPlayer::positionChanged, this, &WaveformWidget::onPlayerPositionChanged);
	connect(m_stream, &StreamProcessor::streamProgress, this, &WaveformWidget::onStreamProgress);
	connect(m_stream, &StreamProcessor::streamFinished, this, &WaveformWidget::onStreamFinished);
	// Using Qt::DirectConnection here makes WaveformWidget::onStreamData() to execute in GStreamer's thread
	connect(m_stream, &StreamProcessor::audioDataAvailable, this, &WaveformWidget::onStreamData, Qt::DirectConnection);
}

WaveformWidget::~WaveformWidget()
{
	clearAudioStream();
}

Time
WaveformWidget::windowSize() const
{
	return m_timeEnd - m_timeStart;
}

void
WaveformWidget::setWindowSize(const Time &size)
{
	if(size != windowSize()) {
		m_timeEnd = m_timeStart + size;
		update();
	}
}

void
WaveformWidget::setSubtitle(Subtitle *subtitle)
{
	m_subtitle = subtitle;
}

QWidget *
WaveformWidget::progressWidget()
{
	return m_progressWidget;
}

void
WaveformWidget::setAudioStream(const QString &mediaFile, int audioStream)
{
	if(m_mediaFile == mediaFile && audioStream == m_streamIndex)
		return;

	clearAudioStream();

	m_mediaFile = mediaFile;
	m_streamIndex = audioStream;

	m_waveformDuration = 0;
	m_waveformDataOffset = 0;

	static WaveFormat waveFormat(8000, 0, BYTES_PER_SAMPLE * 8, true);
	if(m_stream->open(mediaFile) && m_stream->initAudio(audioStream, waveFormat))
		m_stream->start();
}

void
WaveformWidget::clearAudioStream()
{
	m_stream->close();;

	m_mediaFile.clear();
	m_streamIndex = -1;

	for(int i = 0; i < m_waveformChannels; i++)
		delete[] m_waveform[i];
	m_waveformChannels = 0;

	delete[] m_waveform;
	m_waveform = NULL;
}

void
WaveformWidget::onStreamProgress(quint64 msecPos, quint64 msecLength)
{
	if(!m_waveformDuration) {
		m_waveformDuration = msecLength / 1000;
		m_progressBar->setRange(0, m_waveformDuration);
		m_progressWidget->show();
	}
	m_progressBar->setValue(msecPos / 1000);
}

void
WaveformWidget::onStreamFinished()
{
	m_progressWidget->hide();
}

void
WaveformWidget::onStreamData(const void *buffer, const qint32 size, const WaveFormat *waveFormat)
{
	// make sure WaveformWidget::onStreamProgress() signal was processed since we're in different thread
	while(!m_waveformDuration)
		QThread::yieldCurrentThread();

	if(!m_waveformChannels) {
		m_waveformChannels = waveFormat->channels();
		int channelSize = waveFormat->sampleRate() * (m_waveformDuration + 60); // FIXME: added 60sec not to overflow below
		m_waveform = new SAMPLE_TYPE *[m_waveformChannels];
		for(int i = 0; i < m_waveformChannels; i++)
			m_waveform[i] = new SAMPLE_TYPE[channelSize];
	}

	Q_ASSERT(m_waveformDataOffset + size < waveFormat->sampleRate() * (m_waveformDuration + 60) * BYTES_PER_SAMPLE * m_waveformChannels);
	Q_ASSERT(waveFormat->bitsPerSample() == BYTES_PER_SAMPLE * 8);
	Q_ASSERT(waveFormat->sampleRate() == 8000);
	Q_ASSERT(size % BYTES_PER_SAMPLE == 0);

	const SAMPLE_TYPE *sample = reinterpret_cast<const SAMPLE_TYPE *>(buffer);
	int len = size / BYTES_PER_SAMPLE;
	int i = m_waveformDataOffset / BYTES_PER_SAMPLE / m_waveformChannels;
	while(len > 0) {
		for(int c = m_waveformDataOffset / BYTES_PER_SAMPLE % m_waveformChannels; len > 0 && c < m_waveformChannels; c++) {
			qint32 val = *sample++;
			if(i > 0) {
				// simple lowpass filter
				val = (val + m_waveform[c][i - 1]) / 2;
			}
			val &= BYTES_PER_SAMPLE == 1 ? 0x000000ff : 0x0000ffff;
			m_waveform[c][i] = val;
			len--;
		}
		i++;
	}
	m_waveformDataOffset += size;
}

void
WaveformWidget::paintEvent(QPaintEvent *e)
{
	QPainter painter(this);
	painter.fillRect(e->rect(), Qt::black);

	const static QFont fontBig("helvetica", 10);
	const static int fontBigHeight = QFontMetrics(fontBig).height();
	const static QFont fontSubText("helvetica", 9);

	const static QColor subtitleBg(0, 0, 150, 100);
	const static QPen subtitleBorder(QColor(0, 0, 255, 150), 0, Qt::SolidLine);
	const static QPen textWhite(Qt::white, 0, Qt::SolidLine);
	const static QPen textShadow(QColor(0, 0, 0, 192), 0, Qt::SolidLine);
	const static QPen waveDark(QColor(100, 100, 100, 255), 0, Qt::SolidLine);
	const static QPen waveLight(QColor(150, 150, 150, 255), 0, Qt::SolidLine);

	int msWindowSize = windowSize().toMillis();
	int widgetHeight = height();
	int widgetWidth = width();

	// FIXME: draw waveform and overlay text from/to separate buffer
	// FIXME: make visualization types configurable? Min/Max/Avg/RMS

	if(m_waveform) {
		int spp = SAMPLE_RATE_MILIS * msWindowSize / widgetHeight;
		int yMin = (SAMPLE_RATE_MILIS * m_timeStart.toMillis() / spp) * spp;
		int yMax = (SAMPLE_RATE_MILIS * m_timeEnd.toMillis() / spp) * spp;
		int yLimit = m_waveformDataOffset / m_waveformChannels;
		qint32 xMin = 65535, xMax = 0;
//		qint32 xAvg = 0;
//		qreal xRMS = 0;

		qint32 chHalfWidth = widgetWidth / m_waveformChannels / 2;

		for(int ch = 0; ch < m_waveformChannels; ch++) {
			qint32 chCenter = (ch * 2 + 1) * chHalfWidth;
			for(int i = yMin; i < yMax; i++) {
				qint32 val = i >= yLimit ? 0 : (qint32)m_waveform[ch][i] + SIGNED_PAD;
				if(xMin > val)
					xMin = val;
				if(xMax < val)
					xMax = val;
//				xAvg += val;
//				xRMS += val * val;

				if(i % spp == spp - 1) {
//					xAvg /= spp;
//					xRMS = sqrt(xRMS / spp);
					qint32 y = (i - yMin) / spp;
					xMax = xMax * 9 / 5 * chHalfWidth / SAMPLE_MAX;
					xMin = xMin * 5 / 3 * chHalfWidth / SAMPLE_MAX;
/*
					painter.setPen(waveDark);
					painter.drawLine(chCenter + xMax, y, chCenter + xMin, y);
/*/
					painter.setPen(waveDark);
					painter.drawLine(chCenter - xMax, y, chCenter + xMax, y);
					painter.setPen(waveLight);
					painter.drawLine(chCenter - xMin, y, chCenter + xMin, y);
//*/
					xMin = 65535;
					xMax = 0;
//					xAvg = 0;
//					xRMS = 0.0;
				}
			}
		}
	}

	if(m_subtitle) {
		for(int i = 0, n = m_subtitle->linesCount(); i < n; i++) {
			const SubtitleLine *sub = m_subtitle->line(i);
			const Time timeShow = sub->showTime();
			const Time timeHide = sub->hideTime();
			if(timeShow <= m_timeEnd && m_timeStart <= timeHide) {
				int showY = widgetHeight * (timeShow.toMillis() - m_timeStart.toMillis()) / msWindowSize;
				int hideY = widgetHeight * (timeHide.toMillis() - m_timeStart.toMillis()) / msWindowSize;
				QRect box(0, showY, widgetWidth, hideY - showY + 1);

				painter.fillRect(box, subtitleBg);

				painter.setPen(subtitleBorder);
				painter.drawLine(0, showY, widgetWidth, showY);
				painter.drawLine(0, hideY, widgetWidth, hideY);

				painter.setFont(fontSubText);
				painter.setPen(textShadow);
				box.adjust(fontBigHeight + 1, fontBigHeight / 2 + 1, -fontBigHeight, -fontBigHeight / 2);
				painter.drawText(box, Qt::AlignCenter, sub->primaryText().string());
				painter.setPen(textWhite);
				box.adjust(-1, -1, 0, 0);
				painter.drawText(box, Qt::AlignCenter, sub->primaryText().string());

				painter.setPen(textWhite);
				painter.setFont(fontBig);
				painter.drawText(fontBigHeight / 2, showY + fontBigHeight * 3 / 2, QString::number(i));
			}
		}
	}

	int playY = widgetHeight * (m_timeCurrent - m_timeStart).toMillis() / msWindowSize;
	painter.setPen(textWhite);
	painter.drawLine(0, playY, width(), playY);

	QRect textRect(rect().left() + 6, rect().top() + 4, rect().width() - 12, rect().height() - 8);
	painter.setPen(textWhite);
	painter.setFont(fontSubText);
	painter.drawText(textRect, Qt::AlignRight | Qt::AlignTop, m_timeStart.toString());
	painter.drawText(textRect, Qt::AlignRight | Qt::AlignBottom, m_timeEnd.toString());
}

void
WaveformWidget::resizeEvent(QResizeEvent *e)
{
	QWidget::resizeEvent(e);

	update();
}

void
WaveformWidget::onPlayerPositionChanged(double seconds)
{
	Time playingPosition;
	playingPosition.setSecondsTime(seconds);

	if(m_timeCurrent != playingPosition) {
		m_timeCurrent = playingPosition;

		int windowSize = this->windowSize().toMillis(),
			windowPadding = windowSize / 8, // scroll when we reach padding
			windowSizePad = windowSize - 2 * windowPadding;

		if(m_timeCurrent > m_timeEnd.shifted(-windowPadding) || m_timeCurrent < m_timeStart.shifted(windowPadding)) {
			m_timeStart.setMsecondsTime((m_timeCurrent.toMillis() / windowSizePad) * windowSizePad);
			m_timeEnd = m_timeStart.shifted(windowSize);
		}

		update();
	}
}
