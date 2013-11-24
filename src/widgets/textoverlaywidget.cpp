/***************************************************************************
 *   Copyright (C) 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#include "textoverlaywidget.h"

#include <QtCore/QCoreApplication>
#include <QtGui/QPainter>
#include <QtGui/QTextDocument>
#include <QtGui/QAbstractTextDocumentLayout>
#include <QtGui/QResizeEvent>

#include "../profiler.h"
#include <KDebug>

TextOverlayWidget::TextOverlayWidget(QWidget *parent) :
	QWidget(parent, 0),
	m_text(),
	m_alignment(Qt::AlignVCenter | Qt::AlignHCenter),
	m_font(),
	m_primaryColor(Qt::yellow),
	m_primaryRGB(m_primaryColor.rgb()),
	m_outlineWidth(1),
	m_outlineColor(Qt::black),
	m_outlineRGB(m_outlineColor.rgb()),
	m_transColor(0, 0, 0),
	m_transRGB(m_transColor.rgb()),
	m_textDocument(new QTextDocument()),
	m_dirty(true)
{
	// NOTE: this is only supported in X11 but if we don't enable it,
	// setting the widget mask causes a very disgusting flicker.
	// Hopefully the flicker won't be present on other platforms...
	setAttribute(Qt::WA_PaintOnScreen, true);
	setAttribute(Qt::WA_StaticContents, true);
	setAttribute(Qt::WA_OpaquePaintEvent, true);
	setAttribute(Qt::WA_NoSystemBackground, true);

	m_font.setPointSize(15);
	m_font.setStyleStrategy(QFont::NoAntialias);
	m_textDocument->setDefaultFont(m_font);

	QTextOption textOption;
	textOption.setAlignment((Qt::Alignment)m_alignment);
	textOption.setWrapMode(QTextOption::NoWrap);
	m_textDocument->setDefaultTextOption(textOption);

	m_textDocument->setDefaultStyleSheet("p { display: block; white-space: pre; }");

	m_textDocument->setTextWidth(width());

	m_noTextMask = QBitmap(1, 1);
	m_noTextMask.fill(Qt::color1);

	updateColors();
	updateContents();

	parent->installEventFilter(this);
}

TextOverlayWidget::~TextOverlayWidget()
{
	delete m_textDocument;
}

QString
TextOverlayWidget::text() const
{
	return m_text;
}

void
TextOverlayWidget::setText(const QString &text)
{
	if(m_text != text) {
		m_text = text;
		setDirty(true, false, true);
	}
}

int
TextOverlayWidget::alignment() const
{
	return m_alignment;
}

void
TextOverlayWidget::setAlignment(int alignment)
{
	if(m_alignment != alignment) {
		m_alignment = alignment;

		QTextOption textOption = m_textDocument->defaultTextOption();
		textOption.setAlignment((Qt::Alignment)m_alignment);
		m_textDocument->setDefaultTextOption(textOption);

		setDirty(false, false);
	}
}

int
TextOverlayWidget::pointSize() const
{
	return m_font.pointSize();
}

void
TextOverlayWidget::setPointSize(int pointSize)
{
	if(m_font.pointSize() != pointSize) {
		m_font.setPointSize(pointSize);
		m_textDocument->setDefaultFont(m_font);
		setDirty(false, false);
	}
}

qreal
TextOverlayWidget::pointSizeF() const
{
	return m_font.pointSizeF();
}

void
TextOverlayWidget::setPointSizeF(qreal pointSizeF)
{
	if(m_font.pointSizeF() != pointSizeF) {
		m_font.setPointSizeF(pointSizeF);
		m_textDocument->setDefaultFont(m_font);
		setDirty(false, false);
	}
}

int
TextOverlayWidget::pixelSize() const
{
	return m_font.pixelSize();
}

void
TextOverlayWidget::setPixelSize(int pixelSize)
{
	if(m_font.pixelSize() != pixelSize) {
		m_font.setPixelSize(pixelSize);
		m_textDocument->setDefaultFont(m_font);
		setDirty(false, false);
	}
}

QString
TextOverlayWidget::family() const
{
	return m_font.family();
}

void
TextOverlayWidget::setFamily(const QString &family)
{
	if(m_font.family() != family) {
		m_font.setFamily(family);
		m_textDocument->setDefaultFont(m_font);
		setDirty(false, false);
	}
}

QColor
TextOverlayWidget::primaryColor() const
{
	return m_primaryColor;
}

void
TextOverlayWidget::setPrimaryColor(const QColor &color)
{
	if(m_primaryColor != color) {
		m_primaryColor = color;
		setDirty(false, true);
	}
}

int
TextOverlayWidget::outlineWidth() const
{
	return m_outlineWidth;
}

void
TextOverlayWidget::setOutlineWidth(int width)
{
	if(m_outlineWidth != width) {
		m_outlineWidth = width;
		setDirty(false, false);
	}
}

QColor
TextOverlayWidget::outlineColor() const
{
	return m_outlineColor;
}

void
TextOverlayWidget::setOutlineColor(const QColor &color)
{
	if(m_outlineColor != color) {
		m_outlineColor = color;
		setDirty(false, true);
	}
}

QSize
TextOverlayWidget::minimumSizeHint() const
{
	return QSize((int)m_textDocument->idealWidth(), (int)m_textDocument->size().height());
}

QRect
TextOverlayWidget::calculateTextRect() const
{
	QRect parentRect(parentWidget()->rect());

	int textHeight = (int)m_textDocument->size().height(), yoffset;
	if(m_alignment & Qt::AlignBottom)
		yoffset = parentRect.height() - textHeight;
	else if(m_alignment & Qt::AlignTop)
		yoffset = 0;
	else // if ( m_alignment & AlignVCenter || m_alignment & AlignCenter )
		yoffset = (parentRect.height() - textHeight) / 2;

	if(textHeight > parentRect.height()) {
		yoffset = 0;
		textHeight = parentRect.height();
	}

	int textWidth = (int)m_textDocument->idealWidth(), xoffset;
	if(textWidth > parentRect.width()) {
		xoffset = 0;
		textWidth = parentRect.width();
	} else {
		if(m_alignment & Qt::AlignLeft)
			xoffset = 0;
		else if(m_alignment & Qt::AlignRight)
			xoffset = parentRect.width() - textWidth;
		else // if ( m_alignment & Qt::AlignHCenter || m_alignment & Qt::AlignCenter )
			xoffset = (parentRect.width() - textWidth) / 2;
	}

	return QRect(xoffset, yoffset, textWidth, textHeight);
}

void
TextOverlayWidget::setDirty(bool updateRichText, bool updateColors, bool flickerless)
{
	if(updateRichText)
		m_textDocument->setHtml("<p>" + m_text + "</p>");

	if(updateColors)
		this->updateColors();

	if(!m_dirty) {
		m_dirty = true;
		clearMask();
	}

	QRect textRect = calculateTextRect();
	if(flickerless)
		hide();
	resize(textRect.size());
	move(textRect.topLeft());
	if(flickerless) {
		setMask(m_noTextMask);
		show();
	}

	update();
}

bool
TextOverlayWidget::eventFilter(QObject *object, QEvent *event)
{
	if(object == parentWidget() && event->type() == QEvent::Resize) {
		QCoreApplication::postEvent(this, new QEvent(QEvent::User));
	}

	return QWidget::eventFilter(object, event);
}

void
TextOverlayWidget::customEvent(QEvent *event)
{
	if(event->type() == QEvent::User) {
		setDirty(false, false, true);
	}
}

void
TextOverlayWidget::paintEvent(QPaintEvent * /*event */)
{
	if(m_dirty) {
		// PROFILE();
		updateContents();
	}

	QPainter painter(this);
	if(m_text.isEmpty())
		painter.drawPoint(0, 0);
	else
		painter.drawImage(0, 0, m_bgImage);
}

void
TextOverlayWidget::updateColors()
{
	// the outline drawing algorithm expect different primary and outline colors
	// so we have to change one if they are the same
	if(m_outlineColor == m_primaryColor) {
		QRgb rgb = m_outlineColor.rgb();
		int blue = qBlue(rgb);
		m_outlineColor.setRgb(qRed(rgb), qGreen(rgb), blue > 0 ? blue - 1 : blue + 1);
	}
	// the primary and outline colors can't be the same as the translucent color
	// or they won't be shown. It's also best if the translucent color is as dark
	// as possible.
	for(int blue = 0; m_primaryColor == m_transColor || m_outlineColor == m_transColor; ++blue)
		m_transColor.setRgb(0, 0, blue);

	m_primaryRGB = m_primaryColor.rgb();
	m_outlineRGB = m_outlineColor.rgb();
	m_transRGB = m_transColor.rgb();
}

void
TextOverlayWidget::updateContents()
{
	m_textDocument->setTextWidth(width());

	if(m_text.isEmpty()) {
		setMask(m_noTextMask);
	} else {
		const QRect rect = this->rect();
		const QSize size = this->size();

		m_bgImage = QImage(size, QImage::Format_RGB32);

		QPainter painter(&m_bgImage);

		// Disable rendering hints to (hopefully) improve performance
		painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing | QPainter::NonCosmeticDefaultPen, false);

		painter.fillRect(rect, m_transColor);
		painter.setFont(m_font);

		painter.translate(0, 0);
		QAbstractTextDocumentLayout::PaintContext context;
		context.palette = palette();
		context.palette.setColor(QPalette::Text, m_primaryColor);
		m_textDocument->documentLayout()->draw(&painter, context);

		painter.end();

		setMaskAndOutline(m_outlineWidth);
	}

	m_dirty = false;
}

void
TextOverlayWidget::setMaskAndOutline(int outlineWidth)
{
	static const QRgb color0 = QColor(Qt::color0).rgb();    // mask transparent
	static const QRgb color1 = QColor(Qt::color1).rgb();    // mask non transparent

	const QSize size = this->size();

	// NOTE: we use a 32 bits image for the mask creation for performance reasons
	QImage maskImage(size, QImage::Format_RGB32);
	maskImage.fill(color0);

	if(outlineWidth > 0) {
		const int maxX = size.width() - 1;
		const int maxY = size.height() - 1;

		QRgb *bgImageScanLines[size.height()];
		QRgb *maskImageScanLines[size.height()];

		// we don't need all the values initialized, only the ones we will actually use
		for(int y = 0; y <= maxY; ++y) {
			bgImageScanLines[y] = (QRgb *)m_bgImage.scanLine(y);
			maskImageScanLines[y] = (QRgb *)maskImage.scanLine(y);
		}

		int lminX, lmaxX, lminY, lmaxY;
		for(int y = 0; y <= maxY; ++y) {
			for(int x = 0; x <= maxX; ++x) {
				if(bgImageScanLines[y][x] == m_primaryRGB) {    // draw the outline
					lminY = y - outlineWidth;
					if(lminY < 0)
						lminY = 0;
					lmaxY = y + outlineWidth;
					if(lmaxY > maxY)
						lmaxY = maxY;

					lminX = x - outlineWidth;
					if(lminX < 0)
						lminX = 0;
					lmaxX = x + outlineWidth;
					if(lmaxX > maxX)
						lmaxX = maxX;

					for(int ly = lminY; ly <= lmaxY; ++ly) {
						for(int lx = lminX; lx <= lmaxX; ++lx) {
							maskImageScanLines[ly][lx] = color1;
							if(bgImageScanLines[ly][lx] != m_primaryRGB)
								bgImageScanLines[ly][lx] = m_outlineRGB;
						}
					}
				}
			}
		}
	} else {
		QRgb *bgImageBits = (QRgb *)m_bgImage.bits();
		QRgb *maskImageBits = (QRgb *)maskImage.bits();

		for(int index = 0, count = size.width() * size.height(); index < count; ++index) {
			if(bgImageBits[index] != m_transRGB)
				maskImageBits[index] = color1;
		}
	}

	setMask(QBitmap::fromImage(maskImage, Qt::MonoOnly));
}

#include "textoverlaywidget.moc"
