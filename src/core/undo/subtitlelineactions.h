#ifndef SUBTITLELINEACTIONS_H
#define SUBTITLELINEACTIONS_H

/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "core/undo/undoaction.h"
#include "core/time.h"
#include "core/sstring.h"
#include "core/subtitleline.h"

#include <QString>

namespace SubtitleComposer {
class SubtitleLineAction : public UndoAction
{
public:
	SubtitleLineAction(SubtitleLine *line, UndoStack::DirtyMode dirtyMode, const QString &desc = QString());
	virtual ~SubtitleLineAction();

protected:
	SubtitleLine *m_line;

	template<class T>
	const T * tryCastToThisLineAction(const QUndoCommand *prevAction)
	{
		const T *castedPrevAction = dynamic_cast<const T *>(prevAction);
		return castedPrevAction ? (&castedPrevAction->m_line == &m_line ? castedPrevAction : 0) : 0;
	}
};

class SetLinePrimaryTextAction : public SubtitleLineAction
{
	friend class SetLineTextsAction;

public:
	SetLinePrimaryTextAction(SubtitleLine *line, RichDocument *primaryDoc);
	virtual ~SetLinePrimaryTextAction();

	inline int id() const override { return UndoAction::SetLinePrimaryText; }
	bool mergeWith(const QUndoCommand *command) override;

protected:
	void undo() override;
	void redo() override;

private:
	RichDocument *m_primaryDoc;
	int m_primaryDocState = -1;
};

class SetLineSecondaryTextAction : public SubtitleLineAction
{
	friend class SetLineTextsAction;

public:
	SetLineSecondaryTextAction(SubtitleLine *line, RichDocument *secondaryDoc);
	virtual ~SetLineSecondaryTextAction();

	inline int id() const override { return UndoAction::SetLineSecondaryText; }
	bool mergeWith(const QUndoCommand *command) override;

protected:
	void undo() override;
	void redo() override;

private:
	RichDocument *m_secondaryDoc;
	int m_secondaryDocState = -1;
};

class SetLineShowTimeAction : public SubtitleLineAction
{
	friend class SetLineTimesAction;

public:
	SetLineShowTimeAction(SubtitleLine *line, const Time &showTime);
	virtual ~SetLineShowTimeAction();

	inline int id() const override { return UndoAction::SetLineShowTime; }
	bool mergeWith(const QUndoCommand *command) override;

protected:
	void redo() override;

private:
	Time m_showTime;
};

class SetLineHideTimeAction : public SubtitleLineAction
{
	friend class SetLineTimesAction;

public:
	SetLineHideTimeAction(SubtitleLine *line, const Time &hideTime);
	virtual ~SetLineHideTimeAction();

	inline int id() const override { return UndoAction::SetLineHideTime; }
	bool mergeWith(const QUndoCommand *command) override;

protected:
	void redo() override;

private:
	Time m_hideTime;
};

class SetLineTimesAction : public SubtitleLineAction
{
public:
	SetLineTimesAction(SubtitleLine *line, const Time &showTime, const Time &hideTime, QString description = QString());
	virtual ~SetLineTimesAction();

	inline int id() const override { return UndoAction::SetLineTimes; }
	bool mergeWith(const QUndoCommand *command) override;

protected:
	void redo() override;

private:
	typedef enum { ShowTime = 0x1, HideTime = 0x2 } SignalFlags;

	Time m_showTime;
	Time m_hideTime;
};

class SetLineErrorsAction : public SubtitleLineAction
{
public:
	SetLineErrorsAction(SubtitleLine *line, int errorFlags);
	virtual ~SetLineErrorsAction();

	inline int id() const override { return UndoAction::SetLineErrors; }
	bool mergeWith(const QUndoCommand *command) override;

protected:
	void redo() override;

private:
	int m_errorFlags;
};
}

#endif
