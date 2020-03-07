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

#ifndef SUBTITLETEXTOVERLAY_H
#define SUBTITLETEXTOVERLAY_H

#include <QFont>
#include <QColor>
#include <QPen>
#include <QImage>

namespace SubtitleComposer {
class SubtitleTextOverlay
{
public:
	SubtitleTextOverlay();

	inline QString text() const { return m_text; }
	inline QString fontFamily() const { return m_font.family(); }
	inline int fontSizePt() const { return m_font.pointSize(); }
	inline qreal fontSizePtF() const { return m_font.pointSizeF(); }
	inline int fontSizePx() const { return m_font.pixelSize(); }
	inline QColor textColor() const { return m_textColor; }
	inline QColor outlineColor() const { return m_textOutline.color(); }
	inline int outlineWidth() const { return m_textOutline.width(); }

	const QImage & image();
	const QSize & textSize();

private:
	void drawImage();

public slots:
	void setImageSize(int width, int height);
	void setImageSize(QSize size);
	void setText(const QString &text);
	void setFontFamily(const QString &family);
	void setFontSizePt(int pointSize);
	void setFontSizePtF(qreal pointSizeF);
	void setFontSizePx(int pixelSize);
	void setTextColor(const QColor &color);
	void setOutlineColor(const QColor &color);
	void setOutlineWidth(int width);

private:
	QString m_text;
	QFont m_font;
	QColor m_textColor;
	QPen m_textOutline;

	QImage m_image;
	QSize m_textSize;

	bool m_dirty;
};
}

#endif // SUBTITLETEXTOVERLAY_H
