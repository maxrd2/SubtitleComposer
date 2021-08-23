/*
    SPDX-FileCopyrightText: 2010-2021 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "wavesubtitle.h"

#include "core/richdocument.h"
#include "core/subtitleline.h"
#include "gui/waveform/waverenderer.h"

#include <QPainter>
#include <QTextLayout>

using namespace SubtitleComposer;

#define IMG_WIDTH 500

WaveSubtitle::WaveSubtitle(SubtitleLine *line, WaveRenderer *parent)
	: QObject(parent),
	  m_line(line),
	  m_rend(parent),
	  m_textLayout(new QTextLayout()),
	  m_image(IMG_WIDTH, 1, QImage::Format_ARGB32_Premultiplied)
{
	m_textLayout->setFont(m_rend->fontText());
	QTextOption layoutTextOption;
	layoutTextOption.setWrapMode(QTextOption::NoWrap);
	layoutTextOption.setAlignment(Qt::AlignCenter);
	m_textLayout->setTextOption(layoutTextOption);
	m_textLayout->setCacheEnabled(true);
}

WaveSubtitle::~WaveSubtitle()
{
	delete m_textLayout;
}

DragPosition
WaveSubtitle::draggableAt(double time, double *msTolerance)
{
	if(!m_line->intersectsTimespan(time - *msTolerance, time + *msTolerance))
		return DRAG_NONE;

	const bool hasAnchors = m_line->subtitle() && m_line->subtitle()->hasAnchors();
	if(hasAnchors && !m_line->subtitle()->isLineAnchored(m_line))
		return DRAG_FORBIDDEN;

	const double showDistance = qAbs(m_line->showTime().toMillis() - time);
	if(*msTolerance > showDistance) {
		*msTolerance = showDistance;
		return hasAnchors ? DRAG_LINE : DRAG_SHOW;
	}

	const double hideDistance = qAbs(m_line->hideTime().toMillis() - time);
	if(*msTolerance > hideDistance) {
		*msTolerance = hideDistance;
		return hasAnchors ? DRAG_LINE : DRAG_HIDE;
	}

	return DRAG_LINE;
}

void
WaveSubtitle::dragStart(DragPosition dragMode, double dragTime)
{
	if(dragMode == DRAG_FORBIDDEN) {
		m_dragTime = 0.;
		m_dragMode = DRAG_NONE;
		return;
	}

	m_dragTime = dragTime;
	m_dragMode = dragMode;

	if(dragMode == DRAG_LINE || dragMode == DRAG_SHOW)
		m_dragTimeOffset = dragTime - m_line->showTime().toMillis();
	else if(dragMode == DRAG_HIDE)
		m_dragTimeOffset = dragTime - m_line->hideTime().toMillis();
}

DragPosition
WaveSubtitle::dragEnd(double dragTime)
{
	DragPosition mode = m_dragMode;

	const Time newTime(dragTime - m_dragTimeOffset);
	if(m_dragMode == DRAG_LINE)
		m_line->setTimes(newTime, newTime.shifted(m_line->duration()));
	else if(m_dragMode == DRAG_SHOW)
		m_line->setShowTime(newTime);
	else if(m_dragMode == DRAG_HIDE)
		m_line->setHideTime(newTime);

	m_dragMode = DRAG_NONE;
	m_dragTime = 0.;

	return mode;
}

Time
WaveSubtitle::showTime() const
{
	Time newTime(m_dragTime - m_dragTimeOffset);

	switch(m_dragMode) {
	case DRAG_LINE:
		return newTime;

	case DRAG_SHOW:
		return qMin(newTime, m_line->hideTime());

	case DRAG_HIDE:
		return qMin(newTime, m_line->showTime());

	default:
		return m_line->showTime();
	}
}

Time
WaveSubtitle::hideTime() const
{
	Time newTime(m_dragTime - m_dragTimeOffset);

	switch(m_dragMode) {
	case DRAG_LINE:
		return newTime.shifted(m_line->duration());

	case DRAG_SHOW:
		return qMax(newTime, m_line->hideTime());

	case DRAG_HIDE:
		return qMax(newTime, m_line->showTime());

	default:
		return m_line->hideTime();
	}
}

const QImage &
WaveSubtitle::image()
{
	if(!m_imageDirty)
		return m_image;

	RichDocument *doc = m_rend->showTranslation() ? m_line->secondaryDoc() : m_line->primaryDoc();

	QPainter *painter = nullptr;
	const QFont &font = m_rend->fontText();
	const QFontMetrics fontMetrics(font, &m_image);
	qreal height = 0., heightTotal = -1.;

	for(QTextBlock bi = doc->begin(); bi != doc->end(); bi = bi.next()) {
		QString text;
		QVector<QTextLayout::FormatRange> ranges;
		for(QTextBlock::iterator it = bi.begin(); !it.atEnd(); ++it) {
			if(!it.fragment().isValid())
				continue;
			const QString &t = it.fragment().text();
			ranges.push_back(QTextLayout::FormatRange{text.length(), t.length(), it.fragment().charFormat()});
			text.append(t);
		}

		m_textLayout->setText(text);
		m_textLayout->setFormats(ranges);

		m_textLayout->beginLayout();
		QTextLine line = m_textLayout->createLine();
		line.setLineWidth(IMG_WIDTH);
		height += fontMetrics.leading();
		line.setPosition(QPointF(0., height));
		height += line.height();
		m_textLayout->endLayout();

		if(heightTotal < 0.) {
			const int nLines = doc->blockCount();
			heightTotal = nLines * height;
			m_image = QImage(IMG_WIDTH, heightTotal, QImage::Format_ARGB32_Premultiplied);
			m_image.fill(Qt::transparent);
			painter = new QPainter(&m_image);
			painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
			painter->setFont(font);
			painter->setPen(m_rend->subTextColor());
		}

		QRectF br = m_textLayout->boundingRect();
		br.setBottom(br.top() + heightTotal);
		m_textLayout->draw(painter, QPointF());
	}

	if(painter) {
		painter->end();
		delete painter;
	}

	m_imageDirty = false;
	return m_image;
}
