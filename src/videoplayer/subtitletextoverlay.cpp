/*
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "subtitletextoverlay.h"

#include "core/subtitleline.h"

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

struct SubtitleComposer::DrawDocData {
	DrawDocData(QTextLayout *ln, QTextLayout *lo)
		: layoutNormal(ln),
		  layoutOutline(lo),
		  next(nullptr)
	{}
	~DrawDocData() {
		delete layoutNormal;
		delete layoutOutline;
	}

	QTextLayout *layoutNormal;
	QTextLayout *layoutOutline;
	DrawDocData *next;
};


DrawDocData *
SubtitleTextOverlay::drawDocPrepare(QPainter *painter)
{
	DrawDocData *data = nullptr;
	DrawDocData **d = &data;

	const QFontMetrics &fontMetrics = painter->fontMetrics();

	QTextOption layoutTextOption;
	const int imgWidth = m_renderScale > 1.f ? float(m_image.width()) / m_renderScale : m_image.width();
	int lineWidth;
	if(m_pos) {
		layoutTextOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
		if(m_pos->hAlign == SubtitleRect::START)
			layoutTextOption.setAlignment(Qt::AlignLeft);
		else if(m_pos->hAlign == SubtitleRect::END)
			layoutTextOption.setAlignment(Qt::AlignRight);
		else
			layoutTextOption.setAlignment(Qt::AlignHCenter);
		lineWidth = (m_pos->right - m_pos->left) * imgWidth / 100;
	} else {
		layoutTextOption.setWrapMode(QTextOption::NoWrap);
		layoutTextOption.setAlignment(Qt::AlignHCenter);
		lineWidth = imgWidth;
	}

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

		QTextLayout *tlOutline = nullptr;
		if(m_textOutline.width()) {
			tlOutline = new QTextLayout(text, m_font, painter->device());
			tlOutline->setCacheEnabled(true);
			tlOutline->setTextOption(layoutTextOption);
			tlOutline->setFormats(fmtOutline);

			tlOutline->beginLayout();
			for(;;) {
				QTextLine line = tlOutline->createLine();
				if(!line.isValid())
					break;
				line.setLineWidth(lineWidth);
				heightOutline += fontMetrics.leading();
				line.setPosition(QPointF(0., heightOutline));
				heightOutline += line.height();
				maxLineWidth = qMax(maxLineWidth, line.naturalTextWidth());
			}
			tlOutline->endLayout();
		}

		QTextLayout *tlNormal = new QTextLayout(text, m_font, painter->device());
		tlNormal->setCacheEnabled(true);
		tlNormal->setTextOption(layoutTextOption);
		tlNormal->setFormats(fmtNormal);

		tlNormal->beginLayout();
		for(;;) {
			QTextLine line = tlNormal->createLine();
			if(!line.isValid())
				break;
			line.setLineWidth(lineWidth);
			height += fontMetrics.leading();
			line.setPosition(QPointF(0., height));
			height += line.height();
			maxLineWidth = qMax(maxLineWidth, line.naturalTextWidth());
		}
		tlNormal->endLayout();

		*d = new DrawDocData(tlNormal, tlOutline);
		d = &(*d)->next;
	}

	m_textSize = QSize(maxLineWidth, qMax(height, heightOutline));

	return data;
}

void
SubtitleTextOverlay::drawDoc()
{
	QPainter painter(&m_image);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform
						   | QPainter::HighQualityAntialiasing | QPainter::NonCosmeticDefaultPen, true);
	painter.setFont(m_font);
	painter.setPen(m_textColor);

	DrawDocData *d = drawDocPrepare(&painter);

	const float imgWidth = m_renderScale > 1.f ? float(m_image.width()) / m_renderScale : m_image.width();
	const float imgHeight = m_renderScale > 1.f ? float(m_image.height()) / m_renderScale : m_image.height();
	QPointF drawPos;
	if(m_pos) {
		drawPos.setX(m_pos->left * imgWidth / 100.);
		if(m_pos->vAlign == SubtitleRect::TOP)
			drawPos.setY(m_pos->top * imgHeight / 100.);
		else
			drawPos.setY(m_pos->bottom * imgHeight / 100. - m_textSize.height());
	} else {
		drawPos.setY(imgHeight - m_textSize.height());
	}

	while(d) {
		DrawDocData *t = d;
		if(t->layoutOutline)
			t->layoutOutline->draw(&painter, drawPos);
		t->layoutNormal->draw(&painter, drawPos);
		d = d->next;
		delete t;
	}

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
SubtitleTextOverlay::setDocRect(const SubtitleRect *pos)
{
	if(m_pos == pos)
		return;
	m_pos = pos;
	setDirty();
}

void
SubtitleTextOverlay::setRenderScale(double scale)
{
	if(m_renderScale == scale)
		return;
	m_renderScale = scale;
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

