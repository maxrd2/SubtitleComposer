#ifndef WAVEFORMWIDGET_H
#define WAVEFORMWIDGET_H

/**
 * Copyright (C) 2010-2016 Mladen Milinkovic <max@smoothware.net>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../core/time.h"
#include "../core/subtitle.h"

#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QRegion)
QT_FORWARD_DECLARE_CLASS(QPolygon)

namespace SubtitleComposer {
class WaveformWidget : public QWidget
{
	Q_OBJECT

public:
	WaveformWidget(QWidget *parent);
	virtual ~WaveformWidget();

	Time windowSize() const;
	void setWindowSize(const Time &size);

public slots:
	void setSubtitle(Subtitle *subtitle = 0);
	void setAudioStream(const QString &mediaFile, int audioStream);
	void clearAudioStream();

protected:
	virtual void paintEvent(QPaintEvent *);
	virtual void resizeEvent(QResizeEvent *e);

private slots:
	void onPlayerPositionChanged(double seconds);

private:
	Subtitle *m_subtitle;

	Time m_timeStart;
	Time m_timeCurrent;
	Time m_timeEnd;

	int m_waveformChannels;
	qreal **m_waveform;
};
}
#endif
