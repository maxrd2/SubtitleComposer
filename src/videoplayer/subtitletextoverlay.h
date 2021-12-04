/*
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SUBTITLETEXTOVERLAY_H
#define SUBTITLETEXTOVERLAY_H

#include <QFont>
#include <QColor>
#include <QPen>
#include <QImage>

#include "core/richtext/richdocument.h"

namespace SubtitleComposer {
struct SubtitleRect;
struct DrawDocData;

class SubtitleTextOverlay : public QObject
{
	Q_OBJECT

public:
	SubtitleTextOverlay();

	inline QString text() const { return m_text->toPlainText(); }
	inline QString fontFamily() const { return m_font.family(); }
	inline int fontSize() const { return m_font.pixelSize(); }
	inline QColor textColor() const { return m_textColor; }
	inline QColor outlineColor() const { return m_textOutline.color(); }
	inline int outlineWidth() const { return m_textOutline.width(); }

	inline int width() const { return m_image.width(); }
	inline int height() const { return m_image.height(); }

	const QImage & image();
	const QSize & textSize();
	inline bool isDirty() const { return m_dirty; }
	inline double renderScale() const { return m_renderScale; }
	void invertPixels(bool invert);

private:
	void drawImage();
	void drawDoc();
	DrawDocData * drawDocPrepare(QPainter *painter);
	void setDirty();

signals:
	void repaintNeeded();

public slots:
	void setImageSize(int width, int height);
	inline void setImageSize(QSize size) { setImageSize(size.width(), size.height()); }
	void setText(const QString &text);
	void setDoc(const RichDocument *doc);
	void setDocRect(const SubtitleRect *pos);
	void setRenderScale(double scale);
	void setBottomPadding(int padding);
	void setFontFamily(const QString &family);
	void setFontSize(int fontSize);
	void setTextColor(QColor color);
	void setOutlineColor(QColor color);
	void setOutlineWidth(int width);

private:
	bool m_invertPixels;
	RichDocument *m_text = nullptr;
	const RichDocument *m_doc = nullptr;
	const SubtitleRect *m_pos = nullptr;
	QFont m_font;
	QColor m_textColor;
	QPen m_textOutline;

	QImage m_image;
	QSize m_textSize;
	double m_renderScale = 1.0;
	int m_bottomPadding = 0;

	bool m_dirty = true;
};
}

#endif // SUBTITLETEXTOVERLAY_H
