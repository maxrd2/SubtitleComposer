#ifndef SUBTITLELINEACTIONS_H
#define SUBTITLELINEACTIONS_H

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

#include "action.h"
#include "time.h"
#include "sstring.h"
#include "subtitleline.h"
#include "subtitleactions.h"

#include <QtCore/QString>

namespace SubtitleComposer {
class SubtitleLineAction : public Action
{
public:
	SubtitleLineAction(SubtitleLine &line, SubtitleAction::DirtyMode dirtyMode, const QString &desc = QString());
	virtual ~SubtitleLineAction();

protected:
	virtual void _undo();

	virtual void _preRedo();
	virtual void _preUndo();

	virtual void _emitUndoSignals();

	template<class T>
	T * tryCastToThisLineAction(Action *prevAction)
	{
		T *castedPrevAction = tryCastTo<T>(prevAction);
		return castedPrevAction ? (&castedPrevAction->m_line == &m_line ? castedPrevAction : 0) : 0;
	}

protected:
	SubtitleLine &m_line;
	SubtitleAction::DirtyMode m_dirtyMode;
};

class SetLinePrimaryTextAction : public SubtitleLineAction
{
	friend class SetLineTextsAction;

public:
	SetLinePrimaryTextAction(SubtitleLine &line, const SString &primaryText);
	virtual ~SetLinePrimaryTextAction();

protected:
	virtual bool mergeWithPrevious(Action *prevAction);

	virtual void _redo();
	virtual void _emitRedoSignals();

private:
	SString m_primaryText;
};

class SetLineSecondaryTextAction : public SubtitleLineAction
{
	friend class SetLineTextsAction;

public:
	SetLineSecondaryTextAction(SubtitleLine &line, const SString &secondaryText);
	virtual ~SetLineSecondaryTextAction();

protected:
	virtual bool mergeWithPrevious(Action *prevAction);

	virtual void _redo();
	virtual void _emitRedoSignals();

private:
	SString m_secondaryText;
};

class SetLineTextsAction : public SubtitleLineAction
{
public:

	SetLineTextsAction(SubtitleLine &line, const SString &primaryText, const SString &secondaryText);
	virtual ~SetLineTextsAction();

protected:
	virtual bool mergeWithPrevious(Action *prevAction);

	virtual void _redo();
	virtual void _emitRedoSignals();

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

protected:
	virtual bool mergeWithPrevious(Action *prevAction);

	virtual void _redo();
	virtual void _emitRedoSignals();

private:
	Time m_showTime;
};

class SetLineHideTimeAction : public SubtitleLineAction
{
	friend class SetLineTimesAction;

public:
	SetLineHideTimeAction(SubtitleLine &line, const Time &hideTime);
	virtual ~SetLineHideTimeAction();

protected:
	virtual bool mergeWithPrevious(Action *prevAction);

	virtual void _redo();
	virtual void _emitRedoSignals();

private:
	Time m_hideTime;
};

class SetLineTimesAction : public SubtitleLineAction
{
public:
	SetLineTimesAction(SubtitleLine &line, const Time &showTime, const Time &hideTime, QString description = QString());
	virtual ~SetLineTimesAction();

protected:
	virtual bool mergeWithPrevious(Action *prevAction);

	virtual void _redo();
	virtual void _emitRedoSignals();

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

protected:
	virtual bool mergeWithPrevious(Action *prevAction);

	virtual void _redo();
	virtual void _emitRedoSignals();

private:
	int m_errorFlags;
};
}

#endif
