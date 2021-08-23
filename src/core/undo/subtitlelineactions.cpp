/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "subtitlelineactions.h"
#include "core/richdocument.h"
#include "core/subtitle.h"

#include <KLocalizedString>

using namespace SubtitleComposer;

// *** SubtitleLineAction
SubtitleLineAction::SubtitleLineAction(SubtitleLine *line, UndoStack::DirtyMode dirtyMode, const QString &description)
	: UndoAction(dirtyMode, line->subtitle(), description),
	  m_line(line)
{}

SubtitleLineAction::~SubtitleLineAction()
{}


// *** SetLinePrimaryTextAction
SetLinePrimaryTextAction::SetLinePrimaryTextAction(SubtitleLine *line, RichDocument *primaryDoc)
	: SubtitleLineAction(line, UndoStack::Primary, i18n("Set Line Text")),
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
	const bool prev = m_line->ignoreDocChanges(true);
	while(m_primaryDoc->isUndoAvailable() && m_primaryDoc->availableUndoSteps() >= m_primaryDocState)
		m_primaryDoc->undo();
	m_line->ignoreDocChanges(prev);
	emit m_line->primaryTextChanged();
}

void
SetLinePrimaryTextAction::redo()
{
	const bool prev = m_line->ignoreDocChanges(true);
	while(m_primaryDoc->isRedoAvailable() && m_primaryDoc->availableUndoSteps() < m_primaryDocState)
		m_primaryDoc->redo();
	m_line->ignoreDocChanges(prev);
	emit m_line->primaryTextChanged();
}


// *** SetLineSecondaryTextAction
SetLineSecondaryTextAction::SetLineSecondaryTextAction(SubtitleLine *line, RichDocument *secondaryDoc)
	: SubtitleLineAction(line, UndoStack::Secondary, i18n("Set Line Secondary Text")),
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
	const bool prev = m_line->ignoreDocChanges(true);
	while(m_secondaryDoc->isUndoAvailable() && m_secondaryDoc->availableUndoSteps() >= m_secondaryDocState)
		m_secondaryDoc->undo();
	m_line->ignoreDocChanges(prev);
	emit m_line->secondaryTextChanged();
}

void
SetLineSecondaryTextAction::redo()
{
	const bool prev = m_line->ignoreDocChanges(true);
	while(m_secondaryDoc->isRedoAvailable() && m_secondaryDoc->availableUndoSteps() < m_secondaryDocState)
		m_secondaryDoc->redo();
	m_line->ignoreDocChanges(prev);
	emit m_line->secondaryTextChanged();
}



// *** SetLineShowTimeAction
SetLineShowTimeAction::SetLineShowTimeAction(SubtitleLine *line, const Time &showTime)
	: SubtitleLineAction(line, UndoStack::Both, i18n("Set Line Show Time")),
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
	Time tmp = m_line->m_showTime;
	m_line->m_showTime = m_showTime;
	m_showTime = tmp;

	emit m_line->showTimeChanged(m_line->m_showTime);
}


// *** SetLineHideTimeAction
SetLineHideTimeAction::SetLineHideTimeAction(SubtitleLine *line, const Time &hideTime)
	: SubtitleLineAction(line, UndoStack::Both, i18n("Set Line Hide Time")),
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
	Time tmp = m_line->m_hideTime;
	m_line->m_hideTime = m_hideTime;
	m_hideTime = tmp;

	emit m_line->hideTimeChanged(m_line->m_hideTime);
}


// *** SetLineTimesAction
SetLineTimesAction::SetLineTimesAction(SubtitleLine *line, const Time &showTime, const Time &hideTime, QString description)
	: SubtitleLineAction(line, UndoStack::Both, description),
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
	if(m_line->m_showTime != m_showTime) {
		Time tmp = m_line->m_showTime;
		m_line->m_showTime = m_showTime;
		m_showTime = tmp;

		emit m_line->showTimeChanged(m_line->m_showTime);
	}

	if(m_line->m_hideTime != m_hideTime) {
		Time tmp = m_line->m_hideTime;
		m_line->m_hideTime = m_hideTime;
		m_hideTime = tmp;

		emit m_line->hideTimeChanged(m_line->m_hideTime);
	}
}

// *** SetLineErrorsAction
SetLineErrorsAction::SetLineErrorsAction(SubtitleLine *line, int errorFlags)
	: SubtitleLineAction(line, UndoStack::None, i18n("Set Line Errors")),
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
	int tmp = m_line->m_errorFlags;
	m_line->m_errorFlags = m_errorFlags;
	m_errorFlags = tmp;

	emit m_line->errorFlagsChanged(m_line->m_errorFlags);
}
