#ifndef TEXTOVERLAYWIDGET_H
#define TEXTOVERLAYWIDGET_H

/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
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

#include <QWidget>
#include <QFont>
#include <QPen>
#include <QColor>
#include <QImage>
#include <QBitmap>

#include "videoplayer/subtitletextoverlay.h"

QT_FORWARD_DECLARE_CLASS(QTextDocument)

namespace SubtitleComposer {
class TextOverlayWidget : public QWidget
{
	Q_OBJECT

public:
	TextOverlayWidget(QWidget *parent = 0);
	virtual ~TextOverlayWidget();

	QString text() const { return m_overlay.text(); }

	int alignment() const { return 0; }
	int fontSize() const { return m_overlay.fontSize(); }
	QString family() const { return m_overlay.fontFamily(); }
	QColor primaryColor() const { return m_overlay.textColor(); }
	int outlineWidth() const { return m_overlay.outlineWidth(); }
	QColor outlineColor() const { return m_overlay.outlineColor(); }

	QSize minimumSizeHint() const override;

	bool eventFilter(QObject *object, QEvent *event) override;

public slots:
	void setText(const QString &text) { m_overlay.setText(text); update(); }
	void setFontSize(int fontSize) { m_overlay.setFontSize(fontSize); update(); }
	void setFamily(const QString &family) { m_overlay.setFontFamily(family); update(); }
	void setPrimaryColor(const QColor &color) { m_overlay.setTextColor(color); update(); }
	void setOutlineWidth(int width) { m_overlay.setOutlineWidth(width); update(); }
	void setOutlineColor(const QColor &color) { m_overlay.setOutlineColor(color); update(); }

protected:
	void paintEvent(QPaintEvent *event) override;

private:
	mutable SubtitleTextOverlay m_overlay;
};
}

#endif
