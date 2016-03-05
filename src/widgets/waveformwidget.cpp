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

#include <QDebug>

using namespace SubtitleComposer;

WaveformWidget::WaveformWidget(QWidget *parent) :
	QWidget(parent),
	m_subtitle(NULL),
	m_timeStart(0),
	m_timeCurrent(5000),
	m_timeEnd(10000),
	m_waveformChannels(0),
	m_waveform(NULL)
{
	connect(VideoPlayer::instance(), SIGNAL(positionChanged(double)), this, SLOT(onPlayerPositionChanged(double)));
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

void
WaveformWidget::setAudioStream(const QString &mediaFile, int audioStream)
{
	clearAudioStream();

	m_waveformChannels = 1;
	m_waveform = new qreal*[m_waveformChannels];
	for(int i = 0; i < m_waveformChannels; i++)
		m_waveform[i] = new qreal[10000];

	// TODO: generate waveform from stream
}

void
WaveformWidget::clearAudioStream()
{
	for(int i = 0; i < m_waveformChannels; i++)
		delete[] m_waveform[i];
	m_waveformChannels = 0;

	delete[] m_waveform;
	m_waveform = NULL;
}

void
WaveformWidget::paintEvent(QPaintEvent *e)
{
	QPainter painter(this);
	painter.fillRect(e->rect(), Qt::black);

	const static QFont fontBig("helvetica", 10);
	const static int fontBigHeight = QFontMetrics(fontBig).height();
	const static QFont fontSubText("helvetica", 8);

	const static QColor subtitleBg(0, 0, 150, 100);
	const static QPen subtitleBorder(QColor(0, 0, 255, 150), 0, Qt::SolidLine);
	const static QPen textWhite(Qt::white, 0, Qt::SolidLine);

	int msWindowSize = windowSize().toMillis();
	int widgetHeight = height();
	int widgetWidth = width();

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

				painter.setPen(textWhite);

				painter.setFont(fontSubText);
				box.adjust(fontBigHeight, fontBigHeight / 2, -fontBigHeight, -fontBigHeight / 2);
				painter.drawText(box, Qt::AlignCenter, sub->primaryText().string());

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

		Time windowSize = this->windowSize();

		if(m_timeCurrent > m_timeEnd || m_timeCurrent < m_timeStart) {
			m_timeStart.setMsecondsTime((m_timeCurrent.toMillis() / windowSize.toMillis()) * windowSize.toMillis());
			m_timeEnd = m_timeStart + windowSize;
		}

		update();
	}
}
