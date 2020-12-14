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
class RichDocument;

class SubtitleTextOverlay
{
public:
	SubtitleTextOverlay();

	inline QString text() const { return m_text; }
	inline QString fontFamily() const { return m_font.family(); }
	inline int fontSize() const { return m_fontSize; }
	inline QColor textColor() const { return m_textColor; }
	inline QColor outlineColor() const { return m_textOutline.color(); }
	inline int outlineWidth() const { return m_textOutline.width(); }

	inline int width() const { return m_image.width(); }
	inline int height() const { return m_image.height(); }

	const QImage & image();
	const QSize & textSize();
	inline bool isDirty() const { return m_dirty; }
	inline double renderScale() const { return m_renderScale; }

private:
	void drawImage();
	void drawText();
	void drawDoc();

public slots:
	void setImageSize(int width, int height);
	void setImageSize(QSize size);
	void setText(const QString &text);
	void setDoc(const RichDocument *doc);
	void setFontFamily(const QString &family);
	void setFontSize(int fontSize);
	void setTextColor(const QColor &color);
	void setOutlineColor(const QColor &color);
	void setOutlineWidth(int width);
	inline void setRenderScale(double scale) { m_renderScale = scale; }

private:
	QString m_text;
	const RichDocument *m_doc = nullptr;
	QFont m_font;
	int m_fontSize;
	int m_outlineWidth;
	QColor m_textColor;
	QPen m_textOutline;

	QImage m_image;
	QSize m_textSize;
	double m_renderScale = 1.0;

	bool m_dirty = true;
};
}

#endif // SUBTITLETEXTOVERLAY_H
