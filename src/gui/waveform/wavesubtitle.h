/*
 * Copyright (C) 2010-2021 Mladen Milinkovic <max@smoothware.net>
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

#ifndef WAVESUBTITLE_H
#define WAVESUBTITLE_H

#include <core/time.h>

#include <QImage>
#include <QObject>

QT_FORWARD_DECLARE_CLASS(QTextLayout)

namespace SubtitleComposer {
class RichDocument;
class SubtitleLine;
class WaveRenderer;

enum DragPosition {
	DRAG_NONE = 0,
	DRAG_FORBIDDEN,
	DRAG_SHOW,
	DRAG_LINE,
	DRAG_HIDE
};

class WaveSubtitle : public QObject
{
	Q_OBJECT

public:
	explicit WaveSubtitle(SubtitleLine *line, WaveRenderer *parent);

	virtual ~WaveSubtitle();

	inline SubtitleLine * line() const { return m_line; }

	DragPosition draggableAt(double time, double *msTolerance);

	void dragStart(DragPosition dragMode, double dragTime);
	inline void dragUpdate(double dragTime) { m_dragTime = dragTime; }
	DragPosition dragEnd(double dragTime);

	Time showTime() const;
	Time hideTime() const;

	const QImage & image();

private:
	SubtitleLine *m_line;
	WaveRenderer *m_rend;

	QTextLayout *m_textLayout;
	QImage m_image;
	bool m_imageDirty = true;

	DragPosition m_dragMode = DRAG_NONE;
	double m_dragTime = 0.;
	double m_dragTimeOffset;
};
}

#endif // WAVESUBTITLE_H
