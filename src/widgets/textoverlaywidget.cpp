/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

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
}

TextOverlayWidget::~TextOverlayWidget()
{
}

QSize
TextOverlayWidget::minimumSizeHint() const
{
	return QSize(100, m_overlay.textSize().height());
}

QSize
TextOverlayWidget::sizeHint() const
{
	return m_overlay.textSize();
}

void
TextOverlayWidget::resizeEvent(QResizeEvent *event)
{
	m_overlay.setImageSize(event->size());
}

void
TextOverlayWidget::paintEvent(QPaintEvent * /*event */)
{
	QPainter painter(this);
	painter.fillRect(rect(), Qt::transparent);
	const QImage &img = m_overlay.image();
	const int imgCenterX = img.width() / 2;
	const int widgetHalfWidth = width() / 2;
	const QRect src(imgCenterX - widgetHalfWidth, 0, imgCenterX + widgetHalfWidth, height());
	painter.drawImage(rect(), img, src);
}
