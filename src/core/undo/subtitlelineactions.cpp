/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2018 Mladen Milinkovic <max@smoothware.net>
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

#include "subtitlelineactions.h"
#include "core/richdocument.h"
#include "core/subtitle.h"

#include <KLocalizedString>

using namespace SubtitleComposer;

// *** SubtitleLineAction
SubtitleLineAction::SubtitleLineAction(SubtitleLine &line, UndoAction::DirtyMode dirtyMode, const QString &description)
	: UndoAction(dirtyMode, line.subtitle(), description),
	  m_line(line)
{}

SubtitleLineAction::~SubtitleLineAction()
{}


// *** SetLinePrimaryTextAction
SetLinePrimaryTextAction::SetLinePrimaryTextAction(SubtitleLine &line, RichDocument *primaryDoc)
	: SubtitleLineAction(line, UndoAction::Primary, i18n("Set Line Text")),
	  m_primaryDoc(primaryDoc),
	  m_primaryDocState(primaryDoc->availableUndoSteps())
{}

SetLinePrimaryTextAction::~SetLinePrimaryTextAction()
{}

bool
SetLinePrimaryTextAction::mergeWith(const QUndoCommand *command)
{
	const SetLinePrimaryTextAction *cur = static_cast<const SetLinePrimaryTextAction *>(command);
	return cur->m_primaryDoc == m_primaryDoc && cur->m_primaryDocState == m_primaryDocState;
}

void
SetLinePrimaryTextAction::undo()
{
	QSignalBlocker s(m_primaryDoc);
	while(m_primaryDoc->isUndoAvailable() && m_primaryDoc->availableUndoSteps() >= m_primaryDocState)
		m_primaryDoc->undo();
	emit m_line.primaryTextChanged();
}

void
SetLinePrimaryTextAction::redo()
{
	QSignalBlocker s(m_primaryDoc);
	while(m_primaryDoc->isRedoAvailable() && m_primaryDoc->availableUndoSteps() < m_primaryDocState)
		m_primaryDoc->redo();
	emit m_line.primaryTextChanged();
}


// *** SetLineSecondaryTextAction
SetLineSecondaryTextAction::SetLineSecondaryTextAction(SubtitleLine &line, RichDocument *secondaryDoc)
	: SubtitleLineAction(line, UndoAction::Secondary, i18n("Set Line Secondary Text")),
	  m_secondaryDoc(secondaryDoc),
	  m_secondaryDocState(secondaryDoc->availableUndoSteps())
{}

SetLineSecondaryTextAction::~SetLineSecondaryTextAction()
{}

bool
SetLineSecondaryTextAction::mergeWith(const QUndoCommand *command)
{
	const SetLineSecondaryTextAction *cur = static_cast<const SetLineSecondaryTextAction *>(command);
	return cur->m_secondaryDoc == m_secondaryDoc && cur->m_secondaryDocState == m_secondaryDocState;
}

void
SetLineSecondaryTextAction::undo()
{
	QSignalBlocker s(m_secondaryDoc);
	while(m_secondaryDoc->isUndoAvailable() && m_secondaryDoc->availableUndoSteps() >= m_secondaryDocState)
		m_secondaryDoc->undo();
	emit m_line.secondaryTextChanged();
}

void
SetLineSecondaryTextAction::redo()
{
	QSignalBlocker s(m_secondaryDoc);
	while(m_secondaryDoc->isRedoAvailable() && m_secondaryDoc->availableUndoSteps() < m_secondaryDocState)
		m_secondaryDoc->redo();
	emit m_line.secondaryTextChanged();
}



// *** SetLineShowTimeAction
SetLineShowTimeAction::SetLineShowTimeAction(SubtitleLine &line, const Time &showTime)
	: SubtitleLineAction(line, UndoAction::Both, i18n("Set Line Show Time")),
	  m_showTime(showTime)
{}

SetLineShowTimeAction::~SetLineShowTimeAction()
{}

bool
SetLineShowTimeAction::mergeWith(const QUndoCommand *command)
{
	const SetLineShowTimeAction *currentAction = static_cast<const SetLineShowTimeAction *>(command);
	return &currentAction->m_line == &m_line;
}

void
SetLineShowTimeAction::redo()
{
	Time tmp = m_line.m_showTime;
	m_line.m_showTime = m_showTime;
	m_showTime = tmp;

	emit m_line.showTimeChanged(m_line.m_showTime);
}


// *** SetLineHideTimeAction
SetLineHideTimeAction::SetLineHideTimeAction(SubtitleLine &line, const Time &hideTime)
	: SubtitleLineAction(line, UndoAction::Both, i18n("Set Line Hide Time")),
	  m_hideTime(hideTime)
{}

SetLineHideTimeAction::~SetLineHideTimeAction()
{}

bool
SetLineHideTimeAction::mergeWith(const QUndoCommand *command)
{
	const SetLineHideTimeAction *currentAction = static_cast<const SetLineHideTimeAction *>(command);
	return &currentAction->m_line == &m_line;
}

void
SetLineHideTimeAction::redo()
{
	Time tmp = m_line.m_hideTime;
	m_line.m_hideTime = m_hideTime;
	m_hideTime = tmp;

	emit m_line.hideTimeChanged(m_line.m_hideTime);
}


// *** SetLineTimesAction
SetLineTimesAction::SetLineTimesAction(SubtitleLine &line, const Time &showTime, const Time &hideTime, QString description)
	: SubtitleLineAction(line, UndoAction::Both, description),
	  m_showTime(showTime),
	  m_hideTime(hideTime)
{}

SetLineTimesAction::~SetLineTimesAction()
{}

bool
SetLineTimesAction::mergeWith(const QUndoCommand *command)
{
	const SetLineTimesAction *currentAction = static_cast<const SetLineTimesAction *>(command);
	return &currentAction->m_line == &m_line;
}

void
SetLineTimesAction::redo()
{
	if(m_line.m_showTime != m_showTime) {
		Time tmp = m_line.m_showTime;
		m_line.m_showTime = m_showTime;
		m_showTime = tmp;

		emit m_line.showTimeChanged(m_line.m_showTime);
	}

	if(m_line.m_hideTime != m_hideTime) {
		Time tmp = m_line.m_hideTime;
		m_line.m_hideTime = m_hideTime;
		m_hideTime = tmp;

		emit m_line.hideTimeChanged(m_line.m_hideTime);
	}
}

// *** SetLineErrorsAction
SetLineErrorsAction::SetLineErrorsAction(SubtitleLine &line, int errorFlags)
	: SubtitleLineAction(line, UndoAction::None, i18n("Set Line Errors")),
	  m_errorFlags(errorFlags)
{}

SetLineErrorsAction::~SetLineErrorsAction()
{}

bool
SetLineErrorsAction::mergeWith(const QUndoCommand *command)
{
	const SetLineErrorsAction *currentAction = static_cast<const SetLineErrorsAction *>(command);
	return &currentAction->m_line == &m_line;
}

void
SetLineErrorsAction::redo()
{
	int tmp = m_line.m_errorFlags;
	m_line.m_errorFlags = m_errorFlags;
	m_errorFlags = tmp;

	emit m_line.errorFlagsChanged(m_line.m_errorFlags);
}
