/*
    SPDX-FileCopyrightText: 2010-2020 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "subtitletextoverlay.h"

#include "core/richdocument.h"

#include <QAbstractTextDocumentLayout>
#include <QPainter>
#include <QTextCharFormat>
#include <QTextLayout>

#include "scconfig.h"

using namespace SubtitleComposer;

SubtitleTextOverlay::SubtitleTextOverlay()
	: m_fontSize(SCConfig::fontSize())
{
	m_font.setStyleStrategy(QFont::PreferAntialias);
}

void
SubtitleTextOverlay::drawText()
{
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

	// fix top outline being cut
	const int topOffset = m_textOutline.width() / 2;
	painter.translate(0, topOffset);

	// text shadow
//	painter.translate(5, 5 + topOffset);
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

	m_textSize = QSize(doc.idealWidth(), doc.size().height() + topOffset);

	painter.end();
}

void
SubtitleTextOverlay::drawDoc()
{
	QPainter painter(&m_image);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform
						   | QPainter::HighQualityAntialiasing | QPainter::NonCosmeticDefaultPen, true);

	QTextLayout textLayout(QString(), m_font, painter.device());
	QTextOption layoutTextOption;
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
			line.setLineWidth(m_image.width());
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
		line.setLineWidth(m_image.width());
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

	if(m_doc && !m_doc->isEmpty())
		drawDoc();
	else if(!m_text.isEmpty())
		drawText();

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
	setDirty();

	setFontSize(m_fontSize);
	setOutlineWidth(m_outlineWidth);
}

void
SubtitleTextOverlay::setImageSize(QSize size)
{
	setImageSize(size.width(), size.height());
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
	if(m_text == text)
		return;
	m_text = text;
	if(m_doc) {
		disconnect(m_doc, nullptr, this, nullptr);
		m_doc = nullptr;
	}
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
	m_text.clear();
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
	m_fontSize = fontSize;
	const int pixelSize = m_fontSize * m_image.height() / 300;
	if(pixelSize == m_font.pixelSize())
		return;
	m_font.setPixelSize(pixelSize);
	setDirty();
}

void
SubtitleTextOverlay::setTextColor(const QColor &color)
{
	if(m_textColor == color)
		return;
	m_textColor = color;
	setDirty();
}

void
SubtitleTextOverlay::setOutlineColor(const QColor &color)
{
	if(m_textOutline.color() == color)
		return;
	m_textOutline.setColor(color);
	setDirty();
}

void
SubtitleTextOverlay::setOutlineWidth(int width)
{
	m_outlineWidth = width;
	const int pixelWidth = m_outlineWidth * m_image.height() / 300;
	if(m_textOutline.width() == pixelWidth)
		return;
	m_textOutline.setWidth(pixelWidth);
	setDirty();
}

