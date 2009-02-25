#ifndef TEXTOVERLAYWIDGET_H
#define TEXTOVERLAYWIDGET_H

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

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include <QtGui/QWidget>
#include <QtGui/QFont>
#include <QtGui/QPen>
#include <QtGui/QColor>
#include <QtGui/QPixmap>
#include <QtGui/QBitmap>

class QTextDocument;

class TextOverlayWidget : public QWidget
{
	public:

		TextOverlayWidget( QWidget *parent=0 );
		virtual ~TextOverlayWidget();

		QString text() const;
		void setText( const QString& text );

		bool richTextMode() const;
		void setRichTextMode( bool value );

		int alignment() const;
		void setAlignment( int alignment );

		int pointSize() const;
		void setPointSize( int pointSize );

		qreal pointSizeF() const;
		void setPointSizeF( qreal pointSizeF );

		int pixelSize() const;
		void setPixelSize( int pixelSize );

		QString family() const;
		void setFamily( const QString& family );

		QColor primaryColor() const;
		void setPrimaryColor( const QColor& color );

		unsigned outlineWidth() const;
		void setOutlineWidth( unsigned with );

		QColor outlineColor() const;
		void setOutlineColor( const QColor& color );

		virtual QSize minimumSizeHint() const;

	protected:

		virtual void paintEvent( QPaintEvent* );
		virtual void resizeEvent( QResizeEvent* e );

		void setDirty( bool updateRichText, bool updateTransColor );

		void updateColors();
		void updateContents();

		void setMaskAndOutline( QRect& textRect, unsigned width );

	private:

		QString m_text;

		int m_alignment;
		QFont m_font; // font family and size are stored here
		QColor m_primaryColor;
		QRgb m_primaryRGB;

		unsigned m_outlineWidth;
		QColor m_outlineColor;
		QRgb m_outlineRGB;

		QColor m_transColor;
		QRgb m_transRGB;

		bool m_richTextMode;
		QTextDocument* m_textDocument;

		QBitmap m_noTextMask;
		QPixmap m_bgPixmap;

		bool m_dirty;
};

#endif
