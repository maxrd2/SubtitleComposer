/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2020 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "textoverlaywidget.h"

#include <QCoreApplication>
#include <QPainter>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QResizeEvent>

using namespace SubtitleComposer;

TextOverlayWidget::TextOverlayWidget(QWidget *parent)
	: QWidget(parent)
{
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
