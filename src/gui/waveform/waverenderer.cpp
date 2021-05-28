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

#include "waverenderer.h"

#include "application.h"
#include "scconfig.h"
#include "core/richdocument.h"
#include "gui/waveform/wavebuffer.h"
#include "gui/waveform/waveformwidget.h"
#include "gui/waveform/wavesubtitle.h"
#include "gui/waveform/zoombuffer.h"

#include <QBoxLayout>
#include <QPainter>
#include <QScrollBar>
#include <QTextLayout>

using namespace SubtitleComposer;


WaveRenderer::WaveRenderer(WaveformWidget *parent)
	: QWidget(parent),
	  m_wfw(parent)
{
	setAttribute(Qt::WA_OpaquePaintEvent, true);
	setAttribute(Qt::WA_NoSystemBackground, true);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setMouseTracking(true);

	connect(SCConfig::self(), &SCConfig::configChanged, this, &WaveRenderer::onConfigChanged);
	onConfigChanged();
}

bool
WaveRenderer::showTranslation() const
{
	return m_wfw->m_showTranslation;
}

void
WaveRenderer::onConfigChanged()
{
	m_fontNumber = QFont(SCConfig::wfFontFamily(), SCConfig::wfSubNumberFontSize());
	m_fontNumberHeight = QFontMetrics(m_fontNumber).height();
	m_fontText = QFont(SCConfig::wfFontFamily(), SCConfig::wfSubTextFontSize());

	m_subBorderWidth = SCConfig::wfSubBorderWidth();

	m_subNumberColor = QPen(QColor(SCConfig::wfSubNumberColor()), 0, Qt::SolidLine);
	m_subTextColor = QPen(QColor(SCConfig::wfSubTextColor()), 0, Qt::SolidLine);

	// FIXME: instead of using devicePixelRatioF() for pen width we should draw waveform in higher resolution
	m_waveInner = QPen(QColor(SCConfig::wfInnerColor()), devicePixelRatioF(), Qt::SolidLine);
	m_waveOuter = QPen(QColor(SCConfig::wfOuterColor()), devicePixelRatioF(), Qt::SolidLine);

	m_subtitleBack = QColor(SCConfig::wfSubBackground());
	m_subtitleBorder = QColor(SCConfig::wfSubBorder());

	m_selectedBack = QColor(SCConfig::wfSelBackground());
	m_selectedBorder = QColor(SCConfig::wfSelBorder());

	m_playColor = QPen(QColor(SCConfig::wfPlayLocation()), 0, Qt::SolidLine);
	m_mouseColor = QPen(QColor(SCConfig::wfMouseLocation()), 0, Qt::DotLine);
}

bool
WaveRenderer::event(QEvent *evt)
{
	switch(evt->type()) {
	case QEvent::Resize: {
		bool vertical = height() > width();
		if(m_vertical != vertical) {
			m_vertical = vertical;
			m_wfw->onWaveformRotate(m_vertical);
		}
		m_wfw->onWaveformResize(span());
		break;
	}

	case QEvent::Paint: {
		QPainter painter(this);
		painter.fillRect(static_cast<QPaintEvent *>(evt)->rect(), Qt::black);
		paintGraphics(painter);
		return true;
	}

	case QEvent::MouseMove: {
		QMouseEvent *mouse = static_cast<QMouseEvent *>(evt);
		m_wfw->updatePointerTime(m_vertical ? mouse->y() : mouse->x());
		update();
		break;
	}

	case QEvent::MouseButtonDblClick: {
		QMouseEvent *mouse = static_cast<QMouseEvent *>(evt);
		emit m_wfw->doubleClick(m_wfw->timeAt(m_vertical ? mouse->y() : mouse->x()));
		return true;
	}

	case QEvent::MouseButtonPress: {
		QMouseEvent *mouse = static_cast<QMouseEvent *>(evt);
		int y = m_vertical ? mouse->y() : mouse->x();
		if(m_wfw->mousePress(y, mouse->button()))
			return true;
		break;
	}

	case QEvent::MouseButtonRelease: {
		QMouseEvent *mouse = static_cast<QMouseEvent *>(evt);
		int y = m_vertical ? mouse->y() : mouse->x();
		if(m_wfw->mouseRelease(y, mouse->button(), mouse->globalPos()))
			return true;
		break;
	}

	default:
		break;
	}

	return QWidget::event(evt);
}

void
WaveRenderer::paintWaveform(QPainter &painter, quint32 widgetWidth, quint32 widgetHeight)
{
	const quint16 chans = m_wfw->m_wfBuffer->channels();
	if(!chans || !m_wfw->m_zoomDataLen)
		return;

	const quint32 chHalfWidth = (m_vertical ? widgetWidth : widgetHeight) / chans / 2;

	for(quint16 ch = 0; ch < chans; ch++) {
		const qint32 chCenter = (ch * 2 + 1) * chHalfWidth;
		for(quint32 y = 0; y < m_wfw->m_zoomDataLen; y++) {
			const qint32 xMin = m_wfw->m_zoomData[ch][y].min * chHalfWidth / SAMPLE_MAX;
			const qint32 xMax = m_wfw->m_zoomData[ch][y].max * chHalfWidth / SAMPLE_MAX;

			painter.setPen(m_waveOuter);
			if(m_vertical)
				painter.drawLine(chCenter - xMax, y, chCenter + xMax, y);
			else
				painter.drawLine(y, chCenter - xMax, y, chCenter + xMax);
			painter.setPen(m_waveInner);
			if(m_vertical)
				painter.drawLine(chCenter - xMin, y, chCenter + xMin, y);
			else
				painter.drawLine(y, chCenter - xMin, y, chCenter + xMin);
		}
	}
}

void
WaveRenderer::paintGraphics(QPainter &painter)
{
	const quint32 msWindowSize = m_wfw->windowSize();
	const quint32 widgetHeight = height();
	const quint32 widgetWidth = width();
	const quint32 widgetSpan = m_vertical ? widgetHeight : widgetWidth;

	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

	if(widgetSpan)
		paintWaveform(painter, widgetWidth, widgetHeight);

	m_wfw->updateVisibleLines();

	const RangeList &selection = app()->linesWidget()->selectionRanges();
	foreach(WaveSubtitle *sub, m_wfw->m_visibleLines) {
		const Time timeShow = sub->showTime();
		const Time timeHide = sub->hideTime();
		if(timeShow > m_wfw->m_timeEnd || m_wfw->m_timeStart > timeHide)
			continue;

		const bool selected = selection.contains(sub->line()->index());
		const int showY = widgetSpan * (timeShow.toMillis() - m_wfw->m_timeStart.toMillis()) / msWindowSize;
		const int hideY = widgetSpan * (timeHide.toMillis() - m_wfw->m_timeStart.toMillis()) / msWindowSize;
		QRect box;

		// draw background
		if(m_vertical)
			box = QRect(2, showY + m_subBorderWidth, widgetWidth - 4, hideY - showY - 2 * m_subBorderWidth);
		else
			box = QRect(showY + m_subBorderWidth, 2, hideY - showY - 2 * m_subBorderWidth, widgetHeight - 4);

		if(!m_wfw->m_subtitle || !m_wfw->m_subtitle->hasAnchors() || m_wfw->m_subtitle->isLineAnchored(sub->line()))
			painter.setOpacity(1.);
		else
			painter.setOpacity(.5);

		painter.fillRect(box, selected ? m_selectedBack : m_subtitleBack);

		// draw border lines
		if(m_subBorderWidth) {
			if(m_vertical) {
				painter.fillRect(0, showY, widgetWidth, m_subBorderWidth, selected ? m_selectedBorder : m_subtitleBorder);
				painter.fillRect(0, hideY - m_subBorderWidth, widgetWidth, m_subBorderWidth, selected ? m_selectedBorder : m_subtitleBorder);
			} else {
				painter.fillRect(showY, 0, m_subBorderWidth, widgetHeight, selected ? m_selectedBorder : m_subtitleBorder);
				painter.fillRect(hideY - m_subBorderWidth, 0, m_subBorderWidth, widgetHeight, selected ? m_selectedBorder : m_subtitleBorder);
			}
		}

		// draw text
		painter.save();
		painter.setClipRect(box);
		painter.translate(box.center());
		if(!m_vertical) // TODO: make rotation angle configurable
			painter.rotate(-45.);
		const QImage &st = sub->image();
		painter.drawImage(st.width() / -2, st.height() / -2, st, 0, 0, -1, -1);
		painter.restore();

		// draw subtitle index
		painter.setPen(m_subNumberColor);
		painter.setFont(m_fontNumber);
		if(m_vertical)
			painter.drawText(m_fontNumberHeight / 2, showY + m_fontNumberHeight + 2, QString::number(sub->line()->number()));
		else
			painter.drawText(showY + m_fontNumberHeight / 2, m_fontNumberHeight + 2, QString::number(sub->line()->number()));

		// draw anchor icon
		if(m_wfw->m_subtitle && m_wfw->m_subtitle->isLineAnchored(sub->line())) {
			static QFont fontAnchor("sans-serif", 12);
			painter.setFont(fontAnchor);
			if(m_vertical)
				painter.drawText(box, Qt::AlignTop | Qt::AlignRight, QStringLiteral("\u2693"));
			else
				painter.drawText(box, Qt::AlignBottom | Qt::AlignLeft, QStringLiteral("\u2693"));
		}
	}

	if(m_wfw->m_RMBDown) {
		int showY = widgetSpan * (m_wfw->m_timeRMBPress.toMillis() - m_wfw->m_timeStart.toMillis()) / msWindowSize;
		int hideY = widgetSpan * (m_wfw->m_timeRMBRelease.toMillis() - m_wfw->m_timeStart.toMillis()) / msWindowSize;

		QRect box;
		if(m_vertical)
			box = QRect(0, showY + m_subBorderWidth, widgetWidth, hideY - showY - 2 * m_subBorderWidth);
		else
			box = QRect(showY + m_subBorderWidth, 0, hideY - showY - 2 * m_subBorderWidth, widgetHeight);

		painter.fillRect(box, m_selectedBack);
	}

	int playY = widgetSpan * (m_wfw->m_timeCurrent - m_wfw->m_timeStart).toMillis() / msWindowSize;
	painter.setPen(m_playColor);
	if(m_vertical)
		painter.drawLine(0, playY, widgetWidth, playY);
	else
		painter.drawLine(playY, 0, playY, widgetHeight);

	painter.setPen(m_subTextColor);
	painter.setFont(m_fontText);
	if(m_vertical) {
		QRect textRect(6, 4, widgetWidth - 12, widgetHeight - 8);
		painter.drawText(textRect, Qt::AlignRight | Qt::AlignTop, m_wfw->m_timeStart.toString());
		painter.drawText(textRect, Qt::AlignRight | Qt::AlignBottom, m_wfw->m_timeEnd.toString());
	} else {
		QRect textRect(4, 6, widgetWidth - 8, widgetHeight - 12);
		painter.drawText(textRect, Qt::AlignLeft | Qt::AlignTop, m_wfw->m_timeStart.toString());
		painter.drawText(textRect, Qt::AlignRight | Qt::AlignTop, m_wfw->m_timeEnd.toString());
	}

	painter.setPen(m_mouseColor);
	playY = widgetSpan * (m_wfw->m_pointerTime - m_wfw->m_timeStart).toMillis() / msWindowSize;
	if(m_vertical)
		painter.drawLine(0, playY, widgetWidth, playY);
	else
		painter.drawLine(playY, 0, playY, widgetHeight);
}
