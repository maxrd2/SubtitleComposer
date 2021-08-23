/*
    SPDX-FileCopyrightText: 2010-2021 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WAVERENDERER_H
#define WAVERENDERER_H

#include "core/time.h"

#include <QWidget>
#include <QPen>
#include <QColor>
#include <QFont>

namespace SubtitleComposer {
class RichDocument;
class WaveformWidget;

class WaveRenderer : public QWidget
{
	Q_OBJECT

public:
	explicit WaveRenderer(WaveformWidget *parent);

	inline int span() const { return m_vertical ? height() : width(); }
	inline bool vertical() const { return m_vertical; }

	inline const QFont & fontText() const { return m_fontText; }
	inline const QPen & subTextColor() const { return m_subTextColor; }

	bool showTranslation() const;

private:
	bool event(QEvent *evt) override;

	void paintGraphics(QPainter &painter);
	void paintWaveform(QPainter &painter, quint32 widgetWidth, quint32 widgetHeight);

	void onConfigChanged();

private:
	WaveformWidget *m_wfw;

	bool m_vertical = false;

	QFont m_fontNumber;
	int m_fontNumberHeight;
	QFont m_fontText;

	int m_subBorderWidth;

	QPen m_subNumberColor;
	QPen m_subTextColor;

	QPen m_waveInner;
	QPen m_waveOuter;

	QColor m_subtitleBack;
	QColor m_subtitleBorder;

	QColor m_selectedBack;
	QColor m_selectedBorder;

	QPen m_playColor;
	QPen m_mouseColor;
};
}

#endif // WAVERENDERER_H
