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

QTextLayout **
SubtitleTextOverlay::drawDocPrepare(QPainter *painter)
{
	QTextLayout **layoutData = new QTextLayout*[2 * m_doc->blockCount() + 1];
	QTextLayout **res = layoutData;

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

	RichDocumentLayout *docLayout = m_doc->documentLayout();
	for(QTextBlock bi = m_doc->begin(); bi != m_doc->end(); bi = bi.next()) {
		const QString &text = bi.text();
		QVector<QTextLayout::FormatRange> fmtRanges = docLayout->applyCSS(bi.textFormats());

		QTextLayout *tlNormal = new QTextLayout(text, m_font, painter->device());
		*layoutData++ = tlNormal;
		tlNormal->setCacheEnabled(true);
		tlNormal->setTextOption(layoutTextOption);
		tlNormal->setFormats(fmtRanges);

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

		if(m_textOutline.width()) {
			QTextLayout *tlOutline = new QTextLayout(text, m_font, painter->device());
			*layoutData++ = tlOutline;
			tlOutline->setCacheEnabled(true);
			tlOutline->setTextOption(layoutTextOption);
			for(QTextLayout::FormatRange &r: fmtRanges)
				r.format.setTextOutline(m_textOutline);
			tlOutline->setFormats(fmtRanges);

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
		} else {
			*layoutData++ = nullptr;
		}
	}

	*layoutData = nullptr;

	m_textSize = QSize(maxLineWidth, qMax(height, heightOutline));

	return res;
}

void
SubtitleTextOverlay::drawDoc()
{
	if(m_image.isNull())
		return;
	QPainter painter(&m_image);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform, true);
	painter.setFont(m_font);
	painter.setPen(m_textColor);

	QTextLayout **d = drawDocPrepare(&painter);

	const float imgWidth = m_renderScale > 1.f ? float(m_image.width()) / m_renderScale : m_image.width();
	const float imgHeight = (m_renderScale > 1.f ? float(m_image.height()) / m_renderScale : m_image.height()) - m_bottomPadding;
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

	for(int i = 0; d[i]; i += 2) {
		if(d[i + 1]) {
			d[i + 1]->draw(&painter, drawPos);
			delete d[i + 1];
		}
		d[i]->draw(&painter, drawPos);
		delete d[i];
	}
	delete[] d;

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
		m_text->setStylesheet(new RichCSS(this));
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
	if(m_doc) {
		disconnect(m_doc, nullptr, this, nullptr);
		disconnect(m_doc->stylesheet(), nullptr, this, nullptr);
	}
	m_doc = doc;
	if(m_doc) {
		connect(m_doc, &RichDocument::contentsChanged, this, &SubtitleTextOverlay::setDirty);
		connect(m_doc->stylesheet(), &RichCSS::changed, this, &SubtitleTextOverlay::setDirty);
	}
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
SubtitleTextOverlay::setBottomPadding(int padding)
{
	if(m_bottomPadding == padding)
		return;
	m_bottomPadding = padding;
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

