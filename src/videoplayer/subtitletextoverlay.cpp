/*
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

#include "subtitletextoverlay.h"

#include <QAbstractTextDocumentLayout>
#include <QPainter>
#include <QTextCharFormat>
#include <QTextDocument>

using namespace SubtitleComposer;

SubtitleTextOverlay::SubtitleTextOverlay()
	: m_dirty(true)
{
	m_font.setStyleStrategy(QFont::PreferAntialias);
}

void
SubtitleTextOverlay::drawImage()
{
	m_image.fill(Qt::transparent);

	QTextDocument doc;
	doc.setDefaultStyleSheet(QStringLiteral("p { margin:0; padding:0; display:block; white-space:pre; }"));
	doc.setTextWidth(m_image.width());
	doc.setHtml(QStringLiteral("<p>") + m_text + QStringLiteral("</p>"));

	QTextOption textOption;
	textOption.setAlignment(Qt::AlignTop | Qt::AlignHCenter);
	textOption.setWrapMode(QTextOption::NoWrap);
	doc.setDefaultTextOption(textOption);

	doc.setDefaultFont(m_font);

	QPainter painter(&m_image);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform
						   | QPainter::HighQualityAntialiasing | QPainter::NonCosmeticDefaultPen, true);

	QAbstractTextDocumentLayout::PaintContext context;
	context.palette.setColor(QPalette::Text, m_textColor);

	QTextCursor cur(&doc);
	cur.select(QTextCursor::Document);

	QTextCharFormat fmt;

	// text shadow
//	painter.translate(5, 5);
//	{
//		const QColor shadowColor(0, 0, 0, 100);
//		const QPen shadowOutline(QBrush(shadowColor), m_textOutline.widthF());

//		QTextDocument *doc2 = doc.clone();

//		QTextCharFormat fmt;
//		fmt.setTextOutline(shadowOutline);
//		fmt.setForeground(shadowColor);
//		QTextCursor cur(doc2);
//		cur.select(QTextCursor::Document);
//		cur.mergeCharFormat(fmt);

//		doc2->documentLayout()->draw(&painter, context);

//		delete doc2;
//	}
//	painter.end();
//	painter.begin(&m_image);
//	painter.translate(0, 0);

	if(m_textOutline.width()) {
		// draw text outline
		fmt.setTextOutline(m_textOutline);
		cur.mergeCharFormat(fmt);

		doc.documentLayout()->draw(&painter, context);
	}

	// draw rich text
	fmt.setTextOutline(QPen(Qt::transparent, 0));
	cur.mergeCharFormat(fmt);

	doc.documentLayout()->draw(&painter, context);

	m_textSize = QSize(doc.idealWidth(), doc.size().height());

	painter.end();

	m_dirty = false;
}

const QImage &
SubtitleTextOverlay::image()
{
	if(m_dirty)
		drawImage();

	return m_image;
}

const QSize &
SubtitleTextOverlay::textSize()
{
	if(m_dirty)
		drawImage();

	return m_textSize;
}

void
SubtitleTextOverlay::setImageSize(int width, int height)
{
	if(height < 500)
		height = 500;

	if(m_image.width() == width && m_image.height() == height)
		return;

	m_image = QImage(width, height, QImage::Format_ARGB32);
	m_dirty = true;
}

void
SubtitleTextOverlay::setImageSize(QSize size)
{
	setImageSize(size.width(), size.height());
}

void
SubtitleTextOverlay::setText(const QString &text)
{
	m_text = text;
	m_dirty = true;
}

void
SubtitleTextOverlay::setFontFamily(const QString &family)
{
	m_font.setFamily(family);
	m_dirty = true;
}

void
SubtitleTextOverlay::setFontSizePt(int pointSize)
{
	m_font.setPointSize(pointSize);
	m_dirty = true;
}

void
SubtitleTextOverlay::setFontSizePtF(qreal pointSizeF)
{
	m_font.setPointSizeF(pointSizeF);
	m_dirty = true;
}

void
SubtitleTextOverlay::setFontSizePx(int pixelSize)
{
	m_font.setPixelSize(pixelSize);
	m_dirty = true;
}

void
SubtitleTextOverlay::setTextColor(const QColor &color)
{
	m_textColor = color;
	m_dirty = true;
}

void
SubtitleTextOverlay::setOutlineColor(const QColor &color)
{
	m_textOutline.setColor(color);
	m_dirty = true;
}

void
SubtitleTextOverlay::setOutlineWidth(int width)
{
	m_textOutline.setWidth(width);
	m_dirty = true;
}

