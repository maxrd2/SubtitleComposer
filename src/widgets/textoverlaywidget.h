/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2020 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TEXTOVERLAYWIDGET_H
#define TEXTOVERLAYWIDGET_H

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
