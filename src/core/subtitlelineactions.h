#ifndef SUBTITLELINEACTIONS_H
#define SUBTITLELINEACTIONS_H

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

#include "core/undoaction.h"
#include "core/time.h"
#include "core/sstring.h"
#include "core/undoaction.h"
#include "core/subtitleline.h"

#include <QString>

namespace SubtitleComposer {
class SubtitleLineAction : public UndoAction
{
public:
	SubtitleLineAction(SubtitleLine &line, UndoAction::DirtyMode dirtyMode, const QString &desc = QString());
	virtual ~SubtitleLineAction();

protected:
	SubtitleLine &m_line;

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
	SetLinePrimaryTextAction(SubtitleLine &line, const SString &primaryText);
	virtual ~SetLinePrimaryTextAction();

	inline int id() const Q_DECL_OVERRIDE { return UndoAction::SetLinePrimaryText; }
	bool mergeWith(const QUndoCommand *command) Q_DECL_OVERRIDE;

protected:
	void redo() Q_DECL_OVERRIDE;

private:
	SString m_primaryText;
};

class SetLineSecondaryTextAction : public SubtitleLineAction
{
	friend class SetLineTextsAction;

public:
	SetLineSecondaryTextAction(SubtitleLine &line, const SString &secondaryText);
	virtual ~SetLineSecondaryTextAction();

	inline int id() const Q_DECL_OVERRIDE { return UndoAction::SetLineSecondaryText; }
	bool mergeWith(const QUndoCommand *command) Q_DECL_OVERRIDE;

protected:
	void redo() Q_DECL_OVERRIDE;

private:
	SString m_secondaryText;
};

class SetLineTextsAction : public SubtitleLineAction
{
public:

	SetLineTextsAction(SubtitleLine &line, const SString &primaryText, const SString &secondaryText);
	virtual ~SetLineTextsAction();

	inline int id() const Q_DECL_OVERRIDE { return UndoAction::SetLineTexts; }
	bool mergeWith(const QUndoCommand *command) Q_DECL_OVERRIDE;

protected:
	void redo() Q_DECL_OVERRIDE;

private:
	SString m_primaryText;
	SString m_secondaryText;
};

class SetLineShowTimeAction : public SubtitleLineAction
{
	friend class SetLineTimesAction;

public:
	SetLineShowTimeAction(SubtitleLine &line, const Time &showTime);
	virtual ~SetLineShowTimeAction();

	inline int id() const Q_DECL_OVERRIDE { return UndoAction::SetLineShowTime; }
	bool mergeWith(const QUndoCommand *command) Q_DECL_OVERRIDE;

protected:
	void redo() Q_DECL_OVERRIDE;

private:
	Time m_showTime;
};

class SetLineHideTimeAction : public SubtitleLineAction
{
	friend class SetLineTimesAction;

public:
	SetLineHideTimeAction(SubtitleLine &line, const Time &hideTime);
	virtual ~SetLineHideTimeAction();

	inline int id() const Q_DECL_OVERRIDE { return UndoAction::SetLineHideTime; }
	bool mergeWith(const QUndoCommand *command) Q_DECL_OVERRIDE;

protected:
	void redo() Q_DECL_OVERRIDE;

private:
	Time m_hideTime;
};

class SetLineTimesAction : public SubtitleLineAction
{
public:
	SetLineTimesAction(SubtitleLine &line, const Time &showTime, const Time &hideTime, QString description = QString());
	virtual ~SetLineTimesAction();

	inline int id() const Q_DECL_OVERRIDE { return UndoAction::SetLineTimes; }
	bool mergeWith(const QUndoCommand *command) Q_DECL_OVERRIDE;

protected:
	void redo() Q_DECL_OVERRIDE;

private:
	typedef enum { ShowTime = 0x1, HideTime = 0x2 } SignalFlags;

	Time m_showTime;
	Time m_hideTime;
};

class SetLineErrorsAction : public SubtitleLineAction
{
public:
	SetLineErrorsAction(SubtitleLine &line, int errorFlags);
	virtual ~SetLineErrorsAction();

	inline int id() const Q_DECL_OVERRIDE { return UndoAction::SetLineErrors; }
	bool mergeWith(const QUndoCommand *command) Q_DECL_OVERRIDE;

protected:
	void redo() Q_DECL_OVERRIDE;

private:
	int m_errorFlags;
};
}

#endif
