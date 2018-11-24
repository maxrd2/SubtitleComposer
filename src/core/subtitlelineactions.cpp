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
#include "core/subtitle.h"

#define PROPAGATE_LINE_SIGNALS

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
SetLinePrimaryTextAction::SetLinePrimaryTextAction(SubtitleLine &line, const SString &primaryText)
	: SubtitleLineAction(line, UndoAction::Primary, i18n("Set Line Text")),
	  m_primaryText(primaryText)
{}

SetLinePrimaryTextAction::~SetLinePrimaryTextAction()
{}

bool
SetLinePrimaryTextAction::mergeWith(const QUndoCommand *command)
{
	const SetLinePrimaryTextAction *currentAction = static_cast<const SetLinePrimaryTextAction *>(command);
	return &currentAction->m_line == &m_line;
}

void
SetLinePrimaryTextAction::redo()
{
	SString tmp = m_line.m_primaryText;
	m_line.m_primaryText = m_primaryText;
	m_primaryText = tmp;

	emit m_line.primaryTextChanged(m_line.m_primaryText);
#ifdef PROPAGATE_LINE_SIGNALS
	if(m_line.m_subtitle)
		emit m_line.m_subtitle->linePrimaryTextChanged(&m_line, m_line.m_primaryText);
#endif
}


// *** SetLineSecondaryTextAction
SetLineSecondaryTextAction::SetLineSecondaryTextAction(SubtitleLine &line, const SString &secondaryText)
	: SubtitleLineAction(line, UndoAction::Secondary, i18n("Set Line Secondary Text")),
	  m_secondaryText(secondaryText)
{}

SetLineSecondaryTextAction::~SetLineSecondaryTextAction()
{}

bool
SetLineSecondaryTextAction::mergeWith(const QUndoCommand *command)
{
	const SetLineSecondaryTextAction *currentAction = static_cast<const SetLineSecondaryTextAction *>(command);
	return &currentAction->m_line == &m_line;
}

void
SetLineSecondaryTextAction::redo()
{
	SString tmp = m_line.m_secondaryText;
	m_line.m_secondaryText = m_secondaryText;
	m_secondaryText = tmp;

	emit m_line.secondaryTextChanged(m_line.m_secondaryText);
#ifdef PROPAGATE_LINE_SIGNALS
	if(m_line.m_subtitle)
		emit m_line.m_subtitle->lineSecondaryTextChanged(&m_line, m_line.m_secondaryText);
#endif
}


// *** SetLineTextsAction
SetLineTextsAction::SetLineTextsAction(SubtitleLine &line, const SString &primaryText, const SString &secondaryText)
	: SubtitleLineAction(line, UndoAction::Both, i18n("Set Line Texts")),
	  m_primaryText(primaryText),
	  m_secondaryText(secondaryText)
{}

SetLineTextsAction::~SetLineTextsAction()
{}

bool
SetLineTextsAction::mergeWith(const QUndoCommand *command)
{
	const SetLineTextsAction *currentAction = static_cast<const SetLineTextsAction *>(command);
	return &currentAction->m_line == &m_line;
}

void
SetLineTextsAction::redo()
{
	if(m_line.m_primaryText != m_primaryText) {
		SString tmp = m_line.m_primaryText;
		m_line.m_primaryText = m_primaryText;
		m_primaryText = tmp;

		emit m_line.primaryTextChanged(m_line.m_primaryText);
#ifdef PROPAGATE_LINE_SIGNALS
		if(m_line.m_subtitle)
			emit m_line.m_subtitle->linePrimaryTextChanged(&m_line, m_line.m_primaryText);
#endif
	}

	if(m_line.m_secondaryText != m_secondaryText) {
		SString tmp = m_line.m_secondaryText;
		m_line.m_secondaryText = m_secondaryText;
		m_secondaryText = tmp;

		emit m_line.secondaryTextChanged(m_line.m_secondaryText);
#ifdef PROPAGATE_LINE_SIGNALS
		if(m_line.m_subtitle)
			emit m_line.m_subtitle->lineSecondaryTextChanged(&m_line, m_line.m_secondaryText);
#endif
	}
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
#ifdef PROPAGATE_LINE_SIGNALS
	if(m_line.m_subtitle)
		emit m_line.m_subtitle->lineShowTimeChanged(&m_line, m_line.m_showTime);
#endif
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
#ifdef PROPAGATE_LINE_SIGNALS
	if(m_line.m_subtitle)
		emit m_line.m_subtitle->lineHideTimeChanged(&m_line, m_line.m_hideTime);
#endif
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
#ifdef PROPAGATE_LINE_SIGNALS
		if(m_line.m_subtitle)
			emit m_line.m_subtitle->lineShowTimeChanged(&m_line, m_line.m_showTime);
#endif
	}

	if(m_line.m_hideTime != m_hideTime) {
		Time tmp = m_line.m_hideTime;
		m_line.m_hideTime = m_hideTime;
		m_hideTime = tmp;

		emit m_line.hideTimeChanged(m_line.m_hideTime);
#ifdef PROPAGATE_LINE_SIGNALS
		if(m_line.m_subtitle)
			emit m_line.m_subtitle->lineHideTimeChanged(&m_line, m_line.m_hideTime);
#endif
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
#ifdef PROPAGATE_LINE_SIGNALS
	if(m_line.m_subtitle)
		emit m_line.m_subtitle->lineErrorFlagsChanged(&m_line, m_line.m_errorFlags);
#endif
}
