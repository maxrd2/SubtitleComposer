#ifndef SUBTITLEACTIONS_H
#define SUBTITLEACTIONS_H

/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2017 Mladen Milinkovic <max@smoothware.net>
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

#include "core/range.h"
#include "core/rangelist.h"
#include "core/undoaction.h"
#include "core/subtitle.h"

#include <QString>
#include <QList>

namespace SubtitleComposer {
class CompositeAction;

class SubtitleAction : public UndoAction
{
public:
	SubtitleAction(Subtitle &subtitle, UndoAction::DirtyMode dirtyMode, const QString &description = QString());
	virtual ~SubtitleAction();

	inline void setLineSubtitle(SubtitleLine *line)
	{
		line->m_subtitle = &m_subtitle;
	}

	inline void clearLineSubtitle(SubtitleLine *line)
	{
		line->m_subtitle = nullptr;
	}

protected:
	Subtitle &m_subtitle;
};

class SetFramesPerSecondAction : public SubtitleAction
{
public:
	SetFramesPerSecondAction(Subtitle &subtitle, double framesPerSecond);
	virtual ~SetFramesPerSecondAction();

	inline int id() const Q_DECL_OVERRIDE { return UndoAction::SetFramesPerSecond; }

protected:
	void redo() Q_DECL_OVERRIDE;

private:
	double m_framesPerSecond;
};

class InsertLinesAction : public SubtitleAction
{
public:
	InsertLinesAction(Subtitle &subtitle, const QList<SubtitleLine *> &lines, int insertIndex = -1);
	virtual ~InsertLinesAction();

	inline int id() const Q_DECL_OVERRIDE { return UndoAction::InsertLines; }
	bool mergeWith(const QUndoCommand *command) Q_DECL_OVERRIDE;

protected:
	void redo() Q_DECL_OVERRIDE;
	void undo() Q_DECL_OVERRIDE;

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

	inline int id() const Q_DECL_OVERRIDE { return UndoAction::RemoveLines; }
	bool mergeWith(const QUndoCommand *command) Q_DECL_OVERRIDE;

protected:
	void redo() Q_DECL_OVERRIDE;
	void undo() Q_DECL_OVERRIDE;

private:
	int m_firstIndex;
	int m_lastIndex;
	QList<SubtitleLine *> m_lines;
};

class MoveLineAction : public SubtitleAction
{
public:
	MoveLineAction(Subtitle &subtitle, int fromIndex, int toIndex = -1);
	virtual ~MoveLineAction();

	inline int id() const Q_DECL_OVERRIDE { return UndoAction::MoveLine; }
	bool mergeWith(const QUndoCommand *command) Q_DECL_OVERRIDE;

protected:
	void redo() Q_DECL_OVERRIDE;
	void undo() Q_DECL_OVERRIDE;

private:
	int m_fromIndex;
	int m_toIndex;
};

class SwapLinesTextsAction : public SubtitleAction
{
public:
	SwapLinesTextsAction(Subtitle &subtitle, const RangeList &ranges);
	virtual ~SwapLinesTextsAction();

	inline int id() const Q_DECL_OVERRIDE { return UndoAction::SwapLinesTexts; }

protected:
	void redo() Q_DECL_OVERRIDE;

private:
	const RangeList m_ranges;
};
}

#endif
