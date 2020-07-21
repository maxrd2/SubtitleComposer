/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2020 Mladen Milinkovic <max@smoothware.net>
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

#include "textoverlaywidget.h"

#include <QCoreApplication>
#include <QPainter>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QResizeEvent>

#include "profiler.h"
#include <QDebug>

using namespace SubtitleComposer;

TextOverlayWidget::TextOverlayWidget(QWidget *parent) :
	QWidget(parent)
{
	m_overlay.setFontSizePt(15);
	m_overlay.setTextColor(Qt::yellow);
	m_overlay.setOutlineWidth(1);
	m_overlay.setOutlineColor(Qt::black);

	m_overlay.setImageSize(size());

	parent->installEventFilter(this);
}

TextOverlayWidget::~TextOverlayWidget()
{
}

QSize
TextOverlayWidget::minimumSizeHint() const
{
	return m_overlay.textSize();
}

bool
TextOverlayWidget::eventFilter(QObject *object, QEvent *event)
{
	if(object == parentWidget() && event->type() == QEvent::Resize)
		m_overlay.setImageSize(parentWidget()->size());

	return QWidget::eventFilter(object, event);
}

void
TextOverlayWidget::paintEvent(QPaintEvent * /*event */)
{
	QPainter painter(this);
	painter.fillRect(rect(), Qt::transparent);
	const int offY = height() - contentsMargins().bottom() - m_overlay.textSize().height();
	painter.drawImage(0, offY, m_overlay.image());
}
