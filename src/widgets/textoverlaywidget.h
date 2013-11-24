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
#include <QtGui/QImage>
#include <QtGui/QBitmap>

class QTextDocument;

class TextOverlayWidget : public QWidget
{
	Q_OBJECT

public:
	TextOverlayWidget(QWidget *parent = 0);
	virtual ~TextOverlayWidget();

	QString text() const;

	int alignment() const;
	int pointSize() const;
	qreal pointSizeF() const;
	int pixelSize() const;
	QString family() const;
	QColor primaryColor() const;
	int outlineWidth() const;
	QColor outlineColor() const;

	virtual QSize minimumSizeHint() const;

	virtual bool eventFilter(QObject *object, QEvent *event);

public slots:
	void setText(const QString &text);
	void setAlignment(int alignment);
	void setPointSize(int pointSize);
	void setPointSizeF(qreal pointSizeF);
	void setPixelSize(int pixelSize);
	void setFamily(const QString &family);
	void setPrimaryColor(const QColor &color);
	void setOutlineWidth(int with);
	void setOutlineColor(const QColor &color);

protected:
	virtual void customEvent(QEvent *event);
	virtual void paintEvent(QPaintEvent *event);

	void setDirty(bool updateRichText, bool updateTransColor, bool flickerless = false);

	void updateColors();
	void updateContents();

	QRect calculateTextRect() const;
	void setMaskAndOutline(int width);

private:
	QString m_text;

	int m_alignment;
	QFont m_font;                           // font family and size are stored here
	QColor m_primaryColor;
	QRgb m_primaryRGB;

	int m_outlineWidth;
	QColor m_outlineColor;
	QRgb m_outlineRGB;

	QColor m_transColor;
	QRgb m_transRGB;

	QTextDocument *m_textDocument;

	QBitmap m_noTextMask;
	QImage m_bgImage;

	bool m_dirty;
};

#endif
