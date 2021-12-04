/*
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "subtitletextoverlay.h"

#include <QAbstractTextDocumentLayout>
#include <QPainter>
#include <QTextCharFormat>
#include <QTextLayout>

#include "scconfig.h"

using namespace SubtitleComposer;

SubtitleTextOverlay::SubtitleTextOverlay()
	: m_invertPixels(false)
{
	m_font.setStyleStrategy(QFont::PreferAntialias);
	m_font.setPixelSize(SCConfig::fontSize());
}

void
SubtitleTextOverlay::drawDoc()
{
	QPainter painter(&m_image);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform
						   | QPainter::HighQualityAntialiasing | QPainter::NonCosmeticDefaultPen, true);

	QTextLayout textLayout(QString(), m_font, painter.device());
	QTextOption layoutTextOption;
	const int imgWidth = m_renderScale > 1.f ? float(m_image.width()) / m_renderScale : m_image.width();
	layoutTextOption.setWrapMode(QTextOption::NoWrap);
	layoutTextOption.setAlignment(Qt::AlignHCenter);
	textLayout.setTextOption(layoutTextOption);
	textLayout.setCacheEnabled(true);

	painter.setFont(m_font);
	painter.setPen(m_textColor);
	const QFontMetrics &fontMetrics = painter.fontMetrics();

	qreal height = 0., heightOutline = 0.;
	qreal maxLineWidth = 0;

	for(QTextBlock bi = m_doc->begin(); bi != m_doc->end(); bi = bi.next()) {
		QString text;
		QVector<QTextLayout::FormatRange> fmtNormal, fmtOutline;

		for(QTextBlock::iterator it = bi.begin(); !it.atEnd(); ++it) {
			if(!it.fragment().isValid())
				continue;
			const QString &t = it.fragment().text();
			QTextCharFormat fmt = it.fragment().charFormat();
			fmtNormal.push_back(QTextLayout::FormatRange{text.length(), t.length(), fmt});
			if(m_textOutline.width()) {
				fmt.setTextOutline(m_textOutline);
				fmtOutline.push_back(QTextLayout::FormatRange{text.length(), t.length(), fmt});
			}
			text.append(t);
		}

		textLayout.setText(text);

		if(m_textOutline.width()) {
			textLayout.setFormats(fmtOutline);

			textLayout.beginLayout();
			QTextLine line = textLayout.createLine();
			line.setLineWidth(imgWidth);
			heightOutline += fontMetrics.leading();
			line.setPosition(QPointF(0., heightOutline));
			heightOutline += line.height();
			maxLineWidth = qMax(maxLineWidth, line.naturalTextWidth());
			textLayout.endLayout();

			textLayout.draw(&painter, QPointF());
		}

		textLayout.setFormats(fmtNormal);

		textLayout.beginLayout();
		QTextLine line = textLayout.createLine();
		line.setLineWidth(imgWidth);
		height += fontMetrics.leading();
		line.setPosition(QPointF(0., height));
		height += line.height();
		maxLineWidth = qMax(maxLineWidth, line.naturalTextWidth());
		textLayout.endLayout();

		textLayout.draw(&painter, QPointF());
	}

	m_textSize = QSize(maxLineWidth, qMax(height, heightOutline));

	painter.end();
}

void
SubtitleTextOverlay::drawImage()
{
	m_image.fill(Qt::transparent);
	if(m_doc)
		drawDoc();
	m_dirty = false;
}

const QImage &
SubtitleTextOverlay::image()
{
	if(m_dirty)
		drawImage();

	return m_image;
}

void
SubtitleTextOverlay::invertPixels(bool invert)
{
	if(m_invertPixels == invert)
		return;
	m_invertPixels = invert;
	setTextColor(m_textColor);
	setOutlineColor(m_textOutline.color());
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
	if(m_image.width() == width && m_image.height() == height)
		return;

	m_image = QImage(width, height, QImage::Format_ARGB32);
	setDirty();
}

void
SubtitleTextOverlay::setDirty()
{
	m_dirty = true;
	emit repaintNeeded();
}

void
SubtitleTextOverlay::setText(const QString &text)
{
	if(!m_text) {
		m_text = new RichDocument(this);
	} else if(m_text->toHtml() == text) {
		return;
	}
	m_text->setHtml(text, true);
	setDoc(m_text);
	setDirty();
}

void
SubtitleTextOverlay::setDoc(const RichDocument *doc)
{
	if(m_doc == doc)
		return;
	if(m_doc)
		disconnect(m_doc, nullptr, this, nullptr);
	m_doc = doc;
	if(m_doc)
		connect(m_doc, &RichDocument::contentsChanged, this, &SubtitleTextOverlay::setDirty);
	setDirty();
}

void
SubtitleTextOverlay::setFontFamily(const QString &family)
{
	if(m_font.family() == family)
		return;
	m_font.setFamily(family);
	setDirty();
}

void
SubtitleTextOverlay::setFontSize(int fontSize)
{
	if(fontSize == m_font.pixelSize())
		return;
	m_font.setPixelSize(fontSize);
	setDirty();
}

void
SubtitleTextOverlay::setTextColor(QColor color)
{
	if(m_invertPixels)
		color = QColor(color.blue(), color.green(), color.red(), color.alpha());
	if(m_textColor == color)
		return;
	m_textColor = color;
	setDirty();
}

void
SubtitleTextOverlay::setOutlineColor(QColor color)
{
	if(m_invertPixels)
		color = QColor(color.blue(), color.green(), color.red(), color.alpha());
	if(m_textOutline.color() == color)
		return;
	m_textOutline.setColor(color);
	setDirty();
}

void
SubtitleTextOverlay::setOutlineWidth(int width)
{
	if(m_textOutline.width() == width)
		return;
	m_textOutline.setWidth(width);
	setDirty();
}

