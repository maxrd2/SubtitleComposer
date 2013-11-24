#ifndef SUBTITLEACTIONS_H
#define SUBTITLEACTIONS_H

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

#include "range.h"
#include "rangelist.h"
#include "subtitle.h"

#include <QtCore/QString>
#include <QtCore/QList>

namespace SubtitleComposer {
class CompositeAction;

class SubtitleAction : public Action
{
public:
	typedef enum {
		None,
		Primary,
		Secondary,
		Both
	} DirtyMode;

	SubtitleAction(Subtitle &subtitle, DirtyMode dirtyMode, const QString &description = QString());
	virtual ~SubtitleAction();

	inline void setLineSubtitle(SubtitleLine *line, int index)
	{
		line->m_subtitle = &m_subtitle;
		line->m_cachedIndex = index;
	}

	inline void clearLineSubtitle(SubtitleLine *line)
	{
		line->m_subtitle = 0;
		line->m_cachedIndex = -1;
	}

protected:
	virtual void _preRedo();
	virtual void _preUndo();

protected:
	Subtitle &m_subtitle;
	const DirtyMode m_dirtyMode;
};

class SetFramesPerSecondAction : public SubtitleAction
{
public:
	SetFramesPerSecondAction(Subtitle &subtitle, double framesPerSecond);
	virtual ~SetFramesPerSecondAction();

protected:
	virtual void _redo();
	virtual void _undo();

	virtual void _emitRedoSignals();
	virtual void _emitUndoSignals();

private:
	double m_framesPerSecond;
};

class InsertLinesAction : public SubtitleAction
{
public:
	InsertLinesAction(Subtitle &subtitle, const QList<SubtitleLine *> &lines, int insertIndex = -1);
	virtual ~InsertLinesAction();

protected:
	virtual bool mergeWithPrevious(Action *prevAction);

	virtual void _redo();
	virtual void _undo();

private:
	int m_insertIndex;
	int m_lastIndex;
	QList<SubtitleLine *> m_lines;
};

class RemoveLinesAction : public SubtitleAction
{
public:
	RemoveLinesAction(Subtitle &subtitle, int firstIndex, int lastIndex = -1);
	virtual ~RemoveLinesAction();

protected:
	virtual bool mergeWithPrevious(Action *prevAction);

	virtual void _redo();
	virtual void _undo();

private:
	const int m_firstIndex;
	/*const*/ int m_lastIndex;
	QList<SubtitleLine *> m_lines;
};

class MoveLineAction : public SubtitleAction
{
public:
	MoveLineAction(Subtitle &subtitle, int fromIndex, int toIndex = -1);
	virtual ~MoveLineAction();

protected:
	virtual bool mergeWithPrevious(Action *prevAction);

	virtual void _redo();
	virtual void _undo();

private:
	int m_fromIndex;
	int m_toIndex;
};

class SwapLinesTextsAction : public SubtitleAction
{
public:
	SwapLinesTextsAction(Subtitle &subtitle, const RangeList &ranges);
	virtual ~SwapLinesTextsAction();

protected:
	virtual void _redo();
	virtual void _undo();

	virtual void _emitRedoSignals();
	virtual void _emitUndoSignals();

private:
	const RangeList m_ranges;
};
}

#endif
