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

#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QTextDocument>
#include <QtGui/QAbstractTextDocumentLayout>
#include <QtGui/QResizeEvent>

#include <KDebug>

TextOverlayWidget::TextOverlayWidget( QWidget* parent ):
	QWidget( parent, 0 ),
	m_text(),
	m_alignment( Qt::AlignVCenter|Qt::AlignHCenter ),
	m_font(),
	m_primaryColor( Qt::yellow ),
	m_primaryRGB( m_primaryColor.rgb() ),
	m_outlineWidth( 1 ),
	m_outlineColor( Qt::black ),
	m_outlineRGB( m_outlineColor.rgb() ),
	m_transColor( 0, 0, 0 ),
	m_transRGB( m_transColor.rgb() ),
	m_richTextMode( true ),
	m_textDocument( new QTextDocument() ),
	m_dirty( true )
{
	// NOTE: this is only supported in X11 but if we don't enable it,
	// setting the widget mask causes a very disgusting flicker.
	// Hopefully the flicker won't be present on other platforms...
	setAttribute( Qt::WA_PaintOnScreen, true );
	setAttribute( Qt::WA_StaticContents, true );
	setAttribute( Qt::WA_OpaquePaintEvent, true );
	setAttribute( Qt::WA_NoSystemBackground, true );

	m_font.setPointSize( 15 );
	m_textDocument->setDefaultFont( m_font );

	QTextOption textOption;
	textOption.setAlignment( (Qt::Alignment)m_alignment );
	textOption.setWrapMode( QTextOption::NoWrap );
	m_textDocument->setDefaultTextOption( textOption );

	m_textDocument->setDefaultStyleSheet( "p { display: block; white-space: pre; }" );

	m_textDocument->setTextWidth( width() );

	m_noTextMask = QBitmap( 1, 1 );
	m_noTextMask.fill( Qt::color1 );

	updateColors();
	updateContents();
}

TextOverlayWidget::~TextOverlayWidget()
{
	delete m_textDocument;
}

void TextOverlayWidget::setDirty( bool updateRichText, bool updateColors )
{
	if ( updateRichText && m_richTextMode )
		m_textDocument->setHtml( "<p>" + m_text + "</p>" );

	if ( updateColors )
		this->updateColors();

	if ( ! m_dirty )
	{
		m_dirty = true;
		clearMask();
	}
	update();
}

QString TextOverlayWidget::text() const
{
	return m_text;
}

void TextOverlayWidget::setText( const QString& text )
{
	if ( m_text != text )
	{
		m_text = text;
		setDirty( true, false );
	}
}

bool TextOverlayWidget::richTextMode() const
{
	return m_richTextMode;
}

void TextOverlayWidget::setRichTextMode( bool value )
{
	if ( m_richTextMode != value )
	{
		m_richTextMode = value;
		if ( m_richTextMode )
		{
			m_textDocument->setHtml( "<p>" + m_text + "</p>" );
			m_textDocument->setTextWidth( width() );
		}
	}
}

int TextOverlayWidget::alignment() const
{
	return m_alignment;
}

void TextOverlayWidget::setAlignment( int alignment )
{
	if ( m_alignment != alignment )
	{
		m_alignment = alignment;

		QTextOption textOption = m_textDocument->defaultTextOption();
		textOption.setAlignment( (Qt::Alignment)m_alignment );
		m_textDocument->setDefaultTextOption( textOption );

		setDirty( false, false );
	}
}

int TextOverlayWidget::pointSize() const
{
	return m_font.pointSize();
}

void TextOverlayWidget::setPointSize( int pointSize )
{
	if ( m_font.pointSize() != pointSize )
	{
		m_font.setPointSize( pointSize );
		m_textDocument->setDefaultFont( m_font );
		setDirty( false, false );
	}
}

qreal TextOverlayWidget::pointSizeF() const
{
	return m_font.pointSizeF();
}

void TextOverlayWidget::setPointSizeF( qreal pointSizeF )
{
	if ( m_font.pointSizeF() != pointSizeF )
	{
		m_font.setPointSizeF( pointSizeF );
		m_textDocument->setDefaultFont( m_font );
		setDirty( false, false );
	}
}

int TextOverlayWidget::pixelSize() const
{
	return m_font.pixelSize();
}

void TextOverlayWidget::setPixelSize( int pixelSize )
{
	if ( m_font.pixelSize() != pixelSize )
	{
		m_font.setPixelSize( pixelSize );
		m_textDocument->setDefaultFont( m_font );
		setDirty( false, false );
	}
}

QString TextOverlayWidget::family() const
{
	return m_font.family();
}

void TextOverlayWidget::setFamily( const QString& family )
{
	if ( m_font.family() != family )
	{
		m_font.setFamily( family );
		m_textDocument->setDefaultFont( m_font );
		setDirty( false, false );
	}
}

QColor TextOverlayWidget::primaryColor() const
{
	return m_primaryColor;
}

void TextOverlayWidget::setPrimaryColor( const QColor& color )
{
	if ( m_primaryColor != color )
	{
		m_primaryColor = color;
		setDirty( false, true );
	}
}

unsigned TextOverlayWidget::outlineWidth() const
{
	return m_outlineWidth;
}

void TextOverlayWidget::setOutlineWidth( unsigned width )
{
	if ( m_outlineWidth != width )
	{
		m_outlineWidth = width;
		setDirty( false, false );
	}
}

QColor TextOverlayWidget::outlineColor() const
{
	return m_outlineColor;
}

void TextOverlayWidget::setOutlineColor( const QColor& color )
{
	if ( m_outlineColor != color )
	{
		m_outlineColor = color;
		setDirty( false, true );
	}
}

QSize TextOverlayWidget::minimumSizeHint() const
{
	if ( m_richTextMode )
		return QSize( (int)m_textDocument->idealWidth(), (int)m_textDocument->size().height() );
	else
		return QFontMetrics( m_font ).boundingRect( 0, 0, 10000, 10000, m_alignment, m_text ).size();
}

void TextOverlayWidget::paintEvent( QPaintEvent* /*event*/ )
{
	if ( m_dirty )
	{
		// static QTime time;
		// time.start();
		updateContents();
		//kDebug() << "updateContents took" << time.elapsed();
	}

	QPainter painter( this );
	if ( m_text.isEmpty() )
		painter.drawPoint( 0, 0 );
	else
		painter.drawPixmap( 0, 0, m_bgPixmap );
}

void TextOverlayWidget::resizeEvent( QResizeEvent* /*e*/ )
{
	m_dirty = true;
}

void TextOverlayWidget::updateColors()
{
	// the outline drawing algorithm expect different primary and outline colors
	// so we have to change one if they are the same
	if ( m_outlineColor == m_primaryColor )
	{
		QRgb rgb = m_outlineColor.rgb();
		int blue = qBlue( rgb );
		m_outlineColor.setRgb( qRed( rgb ), qGreen( rgb ), blue > 0 ? blue - 1 : blue + 1 );
	}

	// the primary and outline colors can't be the same as the translucent color
	// or they won't be shown. It's also best if the translucent color is as dark
	// as possible.
	for ( int blue = 0; m_primaryColor == m_transColor || m_outlineColor == m_transColor; ++blue )
		m_transColor.setRgb( 0, 0, blue );

	m_primaryRGB = m_primaryColor.rgb();
	m_outlineRGB = m_outlineColor.rgb();
	m_transRGB = m_transColor.rgb();
}

void TextOverlayWidget::updateContents()
{
	if ( m_richTextMode )
		m_textDocument->setTextWidth( width() );

	if ( m_text.isEmpty() )
	{
		setMask( m_noTextMask );
	}
	else
	{
		m_bgPixmap = QPixmap( size() );

		QPainter painter( &m_bgPixmap );
		painter.setRenderHints(
			QPainter::Antialiasing|QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform|
			QPainter::HighQualityAntialiasing|QPainter::NonCosmeticDefaultPen,
			false
		);

		QRect rect( this->rect() );

		painter.fillRect( rect, m_transColor );
		painter.setFont( m_font );

		if ( m_richTextMode )
		{
			const QRect nullRect;

			int textHeight = (int)m_textDocument->size().height(), yoffset;
			if ( m_alignment & Qt::AlignBottom )
				yoffset = rect.height() - textHeight;
			else if ( m_alignment & Qt::AlignTop )
				yoffset = 0;
			else // if ( m_alignment & AlignVCenter || m_alignment & AlignCenter )
				yoffset = (rect.height() - textHeight) / 2;

			painter.translate( 0, yoffset );
			QAbstractTextDocumentLayout::PaintContext context;
			context.palette = palette();
			context.palette.setColor( QPalette::Text, m_primaryColor );
			m_textDocument->documentLayout()->draw( &painter, context );
			painter.end();

			if ( textHeight > rect.height() )
			{
				yoffset = 0;
				textHeight = rect.height();
			}

			int textWidth = (int)m_textDocument->idealWidth(), xoffset;
			if ( textWidth > rect.width() )
			{
				xoffset = 0;
				textWidth = rect.width();
			}
			else
			{
				if ( m_alignment & Qt::AlignLeft )
					xoffset = 0;
				else if ( m_alignment & Qt::AlignRight )
					xoffset = rect.width() - textWidth;
				else // if ( m_alignment & Qt::AlignHCenter || m_alignment & Qt::AlignCenter )
					xoffset = (rect.width() - textWidth) / 2;
			}


			QRect textRect( xoffset, yoffset, textWidth, textHeight );
			setMaskAndOutline( textRect, m_outlineWidth );
		}
		else
		{
			painter.setPen( m_primaryColor );
			painter.drawText( rect, m_alignment, m_text );
			painter.end();

			QRect textRect( painter.boundingRect( rect, m_alignment, m_text ) );
			textRect |= rect;
			setMaskAndOutline( textRect, m_outlineWidth );
		}
	}

	m_dirty = false;
}

void TextOverlayWidget::setMaskAndOutline( QRect& textRect, unsigned outlineWidth )
{
	static const QRgb color0 = QColor( Qt::color0 ).rgb(); // mask transparent
	static const QRgb color1 = QColor( Qt::color1 ).rgb(); // mask non transparent

	QImage bgImage = m_bgPixmap.toImage();

	// NOTE: we use a 32 bits image for the mask creation for performance reasons
	QImage maskImage( size(), QImage::Format_RGB32 );
	maskImage.fill( color0 );

	if ( outlineWidth )
	{
		int width = this->width();
		int maxImageX = width - 1, maxImageY = height() - 1;

		QRgb* bgImageScanLines[maxImageY+1];
		QRgb* maskImageScanLines[maxImageY+1];

		int outlineMinY = textRect.top() - outlineWidth;
		if ( outlineMinY < 0 )
			outlineMinY = 0;

		int outlineMaxY = textRect.bottom() + outlineWidth;
		if ( outlineMaxY > maxImageY )
			outlineMaxY = maxImageY;

// 		static QTime time;
// 		time.start();
// 		// we don't need all the values initialized, only the ones we will actually use
		for ( int y = outlineMinY; y <= outlineMaxY; ++y )
		{
			bgImageScanLines[y] = (QRgb*)bgImage.scanLine( y );
			maskImageScanLines[y] = (QRgb*)maskImage.scanLine( y );
		}
// 		kDebug() << time.elapsed();

		int lminX, lmaxX, lminY, lmaxY;
		for ( int y = textRect.top(), maxY = textRect.bottom(); y <= maxY; ++y )
		{
			for ( int x = textRect.left(), maxX = textRect.right(); x <= maxX; ++x )
			{
				if ( bgImageScanLines[y][x] == m_primaryRGB ) // draw the outline
				{
					lminY = y - outlineWidth;
					if ( lminY < 0 ) lminY = 0;
					lmaxY = y + outlineWidth;
					if ( lmaxY > maxImageY ) lmaxY = maxImageY;
					lminX = x - outlineWidth;
					if ( lminX < 0 ) lminX = 0;
					lmaxX = x + outlineWidth;
					if ( lmaxX > maxImageX ) lmaxX = maxImageX;

					for ( int ly = lminY; ly <= lmaxY; ++ly )
					{
						for ( int lx = lminX; lx <= lmaxX; ++lx )
						{
							maskImageScanLines[ly][lx] = color1;
							if ( bgImageScanLines[ly][lx] != m_primaryRGB )
								bgImageScanLines[ly][lx] = m_outlineRGB;
						}
					}
				}
			}
		}

		m_bgPixmap = m_bgPixmap.fromImage( bgImage );
	}
	else
	{
		QRgb* bgImageBits = (QRgb*)bgImage.bits();
		QRgb* maskImageBits = (QRgb*)maskImage.bits();

		for ( int index = 0, size = width()*height(); index < size; ++index )
		{
			if ( bgImageBits[index] != m_transRGB )
				maskImageBits[index] = color1;
		}
	}

	setMask( QBitmap::fromImage( maskImage, Qt::MonoOnly ) );
}

#include "textoverlaywidget.moc"
