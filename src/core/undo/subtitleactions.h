#ifndef SUBTITLEACTIONS_H
#define SUBTITLEACTIONS_H

/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "core/range.h"
#include "core/rangelist.h"
#include "core/undo/undoaction.h"
#include "core/subtitle.h"

#include <QString>
#include <QList>

namespace SubtitleComposer {

class SubtitleAction : public UndoAction
{
public:
	SubtitleAction(Subtitle *subtitle, UndoStack::DirtyMode dirtyMode, const QString &description = QString());
	virtual ~SubtitleAction();

	inline void setLineSubtitle(SubtitleLine *line)
	{
		line->m_subtitle = m_subtitle;
	}

	inline void clearLineSubtitle(SubtitleLine *line)
	{
		line->m_subtitle = nullptr;
	}
};

class SetFramesPerSecondAction : public SubtitleAction
{
public:
	SetFramesPerSecondAction(Subtitle *subtitle, double framesPerSecond);
	virtual ~SetFramesPerSecondAction();

	inline int id() const override { return UndoAction::SetFramesPerSecond; }

protected:
	void redo() override;

private:
	double m_framesPerSecond;
};

class InsertLinesAction : public SubtitleAction
{
public:
	InsertLinesAction(Subtitle *subtitle, const QList<SubtitleLine *> &lines, int insertIndex = -1);
	virtual ~InsertLinesAction();

	inline int id() const override { return UndoAction::InsertLines; }
	bool mergeWith(const QUndoCommand *command) override;

protected:
	void redo() override;
	void undo() override;

private:
	int m_insertIndex;
	int m_lastIndex;
	QList<SubtitleLine *> m_lines;
};

class RemoveLinesAction : public SubtitleAction
{
public:
	RemoveLinesAction(Subtitle *subtitle, int firstIndex, int lastIndex = -1);
	virtual ~RemoveLinesAction();

	inline int id() const override { return UndoAction::RemoveLines; }
	bool mergeWith(const QUndoCommand *command) override;

protected:
	void redo() override;
	void undo() override;

private:
	int m_firstIndex;
	int m_lastIndex;
	QList<SubtitleLine *> m_lines;
};

class MoveLineAction : public SubtitleAction
{
public:
	MoveLineAction(Subtitle *subtitle, int fromIndex, int toIndex = -1);
	virtual ~MoveLineAction();

	inline int id() const override { return UndoAction::MoveLine; }
	bool mergeWith(const QUndoCommand *command) override;

protected:
	void redo() override;
	void undo() override;

private:
	int m_fromIndex;
	int m_toIndex;
};

class SwapLinesTextsAction : public SubtitleAction
{
public:
	SwapLinesTextsAction(Subtitle *subtitle, const RangeList &ranges);
	virtual ~SwapLinesTextsAction();

	inline int id() const override { return UndoAction::SwapLinesTexts; }

protected:
	void redo() override;

private:
	const RangeList m_ranges;
};
}

#endif
