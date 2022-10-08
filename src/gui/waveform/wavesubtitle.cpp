/*
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "wavesubtitle.h"

#include "core/richtext/richdocument.h"
#include "core/subtitleline.h"
#include "gui/waveform/waverenderer.h"

#include <QPainter>
#include <QTextBlock>
#include <QTextLayout>
#include <QTextLine>

using namespace SubtitleComposer;

WaveSubtitle::WaveSubtitle(SubtitleLine *line, WaveRenderer *parent)
	: QObject(parent),
	  m_line(line),
	  m_rend(parent),
	  m_image(1, 1, QImage::Format_ARGB32_Premultiplied)
{
	const RichDocument *doc = m_rend->showTranslation() ? m_line->secondaryDoc() : m_line->primaryDoc();
	connect(doc, &RichDocument::contentsChanged, this, [&](){ m_imageDirty = true; });
}

WaveSubtitle::~WaveSubtitle()
{
}

DragPosition
WaveSubtitle::draggableAt(double time, double *msTolerance) const
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
WaveSubtitle::image() const
{
	if(!m_imageDirty)
		return m_image;

	const RichDocument *doc = m_rend->showTranslation() ? m_line->secondaryDoc() : m_line->primaryDoc();
	const RichDocumentLayout *layout = doc->documentLayout();

	qreal width = 0., height = 0.;
	QTextLayout *layouts = new QTextLayout[doc->blockCount()];
	QTextLayout *bl = layouts;
	for(QTextBlock bi = doc->begin(); bi != doc->end(); bi = bi.next()) {
		bl->setCacheEnabled(true);
		bl->setFont(m_rend->fontText());
		bl->setText(bi.text());
		bl->setFormats(layout->applyCSS(bi.textFormats()));
		bl->beginLayout();
		QTextOption option = bi.layout()->textOption();
		option.setAlignment(Qt::AlignTop | Qt::AlignLeft | Qt::AlignAbsolute);
		bl->setTextOption(option);
		for(;;) {
			QTextLine line = bl->createLine();
			if(!line.isValid())
				break;
			line.setLeadingIncluded(true);
			line.setLineWidth(10000);
			line.setPosition(QPointF(0., height));
			height += line.height();
			width = qMax(width, line.naturalTextWidth());
		}
		bl->endLayout();
		bl++;
	}

	m_image = QImage(QSize(width, height), QImage::Format_ARGB32_Premultiplied);
	m_image.fill(Qt::transparent);
	QPainter *painter = new QPainter(&m_image);
	if(!painter->isActive()) {
		delete[] layouts;
		delete painter;
		return m_image;
	}
	painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
	painter->setFont(m_rend->fontText());
	painter->setPen(m_rend->subTextColor());

	while(bl-- != layouts) {
		const int n = bl->lineCount();
		for(int i = 0; i < n; i++) {
			const QTextLine &tl = bl->lineAt(i);
			const QPointF pos((width - tl.naturalTextWidth()) / 2., 0.);
			tl.draw(painter, pos);
		}
	}

	delete[] layouts;

	painter->end();
	delete painter;

	m_imageDirty = false;
	return m_image;
}
