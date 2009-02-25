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

#include "subtitlelineactions.h"
#include "subtitle.h"

#include <KLocale>

#define PROPAGATE_LINE_SIGNALS

using namespace SubtitleComposer;

/// SUBTITLE LINE ACTION
/// ====================

SubtitleLineAction::SubtitleLineAction( SubtitleLine& line, SubtitleAction::DirtyMode dirtyMode, const QString& description ):
	Action( description ),
	m_line( line ),
	m_dirtyMode( dirtyMode ) {}
SubtitleLineAction::~SubtitleLineAction() {}

void SubtitleLineAction::_undo()
{
	_redo();
}

void SubtitleLineAction::_preRedo()
{
	if ( m_line.m_subtitle )
		m_line.m_subtitle->incrementState( m_dirtyMode );
}

void SubtitleLineAction::_preUndo()
{
	if ( m_line.m_subtitle )
		m_line.m_subtitle->decrementState( m_dirtyMode );
}

void SubtitleLineAction::_emitUndoSignals()
{
	_emitRedoSignals();
}


/// SET LINE PRIMARY TEXT ACTION
/// ============================

SetLinePrimaryTextAction::SetLinePrimaryTextAction( SubtitleLine& line, const SString& primaryText ):
	SubtitleLineAction( line, SubtitleAction::Primary, i18n( "Set Line Text" ) ),
	m_primaryText( primaryText ) {}
SetLinePrimaryTextAction::~SetLinePrimaryTextAction() {}

bool SetLinePrimaryTextAction::mergeWithPrevious( Action* pa )
{
	SetLinePrimaryTextAction* prevAction = tryCastToThisLineAction<SetLinePrimaryTextAction>( pa );
	if ( ! prevAction )
		return false;

	prevAction->_preUndo();
	m_primaryText = prevAction->m_primaryText;
	return true;
}

void SetLinePrimaryTextAction::_redo()
{
	SString aux = m_line.m_primaryText;
	m_line.m_primaryText = m_primaryText;
	m_primaryText = aux;
}

void SetLinePrimaryTextAction::_emitRedoSignals()
{
	m_line.emit primaryTextChanged( m_line.m_primaryText );
#ifdef PROPAGATE_LINE_SIGNALS
	if ( m_line.m_subtitle )
		m_line.m_subtitle->emit linePrimaryTextChanged( &m_line, m_line.m_primaryText );
#endif
}


/// SET LINE SECONDARY TEXT ACTION
/// ==============================

SetLineSecondaryTextAction::SetLineSecondaryTextAction( SubtitleLine& line, const SString& secondaryText ):
	SubtitleLineAction( line, SubtitleAction::Secondary, i18n( "Set Line Secondary Text" ) ),
	m_secondaryText( secondaryText ) {}
SetLineSecondaryTextAction::~SetLineSecondaryTextAction() {}

bool SetLineSecondaryTextAction::mergeWithPrevious( Action* pa )
{
	SetLineSecondaryTextAction* prevAction = tryCastToThisLineAction<SetLineSecondaryTextAction>( pa );
	if ( ! prevAction )
		return false;

	prevAction->_preUndo();
	m_secondaryText = prevAction->m_secondaryText;
	return true;
}

void SetLineSecondaryTextAction::_redo()
{
	SString aux = m_line.m_secondaryText;
	m_line.m_secondaryText = m_secondaryText;
	m_secondaryText = aux;
}

void SetLineSecondaryTextAction::_emitRedoSignals()
{
	m_line.emit secondaryTextChanged( m_line.m_secondaryText );
#ifdef PROPAGATE_LINE_SIGNALS
	if ( m_line.m_subtitle )
		m_line.m_subtitle->emit lineSecondaryTextChanged( &m_line, m_line.m_secondaryText );
#endif
}


/// SET LINE TEXTS ACTION
/// =====================

SetLineTextsAction::SetLineTextsAction( SubtitleLine& line, const SString& primaryText, const SString& secondaryText ):
	SubtitleLineAction( line, SubtitleAction::Both, i18n( "Set Line Texts" ) ),
	m_primaryText( primaryText ),
	m_secondaryText( secondaryText ) {}
SetLineTextsAction::~SetLineTextsAction() {}

bool SetLineTextsAction::mergeWithPrevious( Action* pa )
{
	SetLineTextsAction* prevAction = tryCastToThisLineAction<SetLineTextsAction>( pa );
	if ( prevAction )
	{
		prevAction->_preUndo();
		m_primaryText = prevAction->m_primaryText;
		m_secondaryText = prevAction->m_secondaryText;
	}
	else
	{
		SetLinePrimaryTextAction* prevAction2 = tryCastToThisLineAction<SetLinePrimaryTextAction>( pa );
		if ( prevAction2 )
		{
			prevAction2->_preUndo();
			m_primaryText = prevAction2->m_primaryText;
		}
		else
		{
			SetLineSecondaryTextAction* prevAction3 = tryCastToThisLineAction<SetLineSecondaryTextAction>( pa );
		 	if ( prevAction3 )
			{
				prevAction3->_preUndo();
				m_secondaryText = prevAction3->m_secondaryText;
			}
			else
				return false;
		}
	}

	return true;
}

void SetLineTextsAction::_redo()
{
	if ( m_line.m_primaryText != m_primaryText )
	{
		SString aux = m_line.m_primaryText;
		m_line.m_primaryText = m_primaryText;
		m_primaryText = aux;
	}

	if ( m_line.m_secondaryText != m_secondaryText )
	{
		SString aux = m_line.m_secondaryText;
		m_line.m_secondaryText = m_secondaryText;
		m_secondaryText = aux;
	}
}

void SetLineTextsAction::_emitRedoSignals()
{
	if ( m_line.m_primaryText != m_primaryText )
	{
		m_line.emit primaryTextChanged( m_line.m_primaryText );
#ifdef PROPAGATE_LINE_SIGNALS
		if ( m_line.m_subtitle )
			m_line.m_subtitle->emit linePrimaryTextChanged( &m_line, m_line.m_primaryText );
#endif
	}

	if ( m_line.m_secondaryText != m_secondaryText )
	{
		m_line.emit secondaryTextChanged( m_line.m_secondaryText );
#ifdef PROPAGATE_LINE_SIGNALS
		if ( m_line.m_subtitle )
			m_line.m_subtitle->emit lineSecondaryTextChanged( &m_line, m_line.m_secondaryText );
#endif
	}
}


/// SET LINE SHOW TIME ACTION
/// =========================

SetLineShowTimeAction::SetLineShowTimeAction( SubtitleLine& line, const Time& showTime ):
	SubtitleLineAction( line, SubtitleAction::Both, i18n( "Set Line Show Time" ) ),
	m_showTime( showTime ) {}
SetLineShowTimeAction::~SetLineShowTimeAction() {}

bool SetLineShowTimeAction::mergeWithPrevious( Action* pa )
{
	SetLineShowTimeAction* prevAction = tryCastToThisLineAction<SetLineShowTimeAction>( pa );
	if ( ! prevAction )
		return false;

	prevAction->_preUndo();
	m_showTime = prevAction->m_showTime;
	return true;
}

void SetLineShowTimeAction::_redo()
{
	Time aux = m_line.m_showTime;
	m_line.m_showTime = m_showTime;
	m_showTime = aux;
}

void SetLineShowTimeAction::_emitRedoSignals()
{
	m_line.emit showTimeChanged( m_line.m_showTime );
#ifdef PROPAGATE_LINE_SIGNALS
	if ( m_line.m_subtitle )
		m_line.m_subtitle->emit lineShowTimeChanged( &m_line, m_line.m_showTime );
#endif
}


/// SET LINE HIDE TIME ACTION
/// =========================

SetLineHideTimeAction::SetLineHideTimeAction( SubtitleLine& line, const Time& hideTime ):
	SubtitleLineAction( line, SubtitleAction::Both, i18n( "Set Line Hide Time" ) ),
	m_hideTime( hideTime ) {}
SetLineHideTimeAction::~SetLineHideTimeAction() {}

bool SetLineHideTimeAction::mergeWithPrevious( Action* pa )
{
	SetLineHideTimeAction* prevAction = tryCastToThisLineAction<SetLineHideTimeAction>( pa );
	if ( ! prevAction )
		return false;

	prevAction->_preUndo();
	m_hideTime = prevAction->m_hideTime;
	return true;
}

void SetLineHideTimeAction::_redo()
{
	Time aux = m_line.m_hideTime;
	m_line.m_hideTime = m_hideTime;
	m_hideTime = aux;
}

void SetLineHideTimeAction::_emitRedoSignals()
{
	m_line.emit hideTimeChanged( m_line.m_hideTime );
#ifdef PROPAGATE_LINE_SIGNALS
	if ( m_line.m_subtitle )
		m_line.m_subtitle->emit lineHideTimeChanged( &m_line, m_line.m_hideTime );
#endif
}


/// SET LINE TIMES ACTION
/// =====================

SetLineTimesAction::SetLineTimesAction( SubtitleLine& line, const Time& showTime, const Time& hideTime, QString description ):
	SubtitleLineAction( line, SubtitleAction::Both, description ),
	m_showTime( showTime ),
	m_hideTime( hideTime ) {}
SetLineTimesAction::~SetLineTimesAction() {}

bool SetLineTimesAction::mergeWithPrevious( Action* pa )
{
	SetLineTimesAction* prevAction = tryCastToThisLineAction<SetLineTimesAction>( pa );
	if ( prevAction )
	{
		prevAction->_preUndo();
		m_showTime = prevAction->m_showTime;
		m_hideTime = prevAction->m_hideTime;
	}
	else
	{
		SetLineHideTimeAction* prevAction2 = tryCastToThisLineAction<SetLineHideTimeAction>( pa );
		if ( prevAction2 )
		{
			prevAction2->_preUndo();
			m_hideTime = prevAction2->m_hideTime;
		}
		else
		{
			SetLineShowTimeAction* prevAction3 = tryCastToThisLineAction<SetLineShowTimeAction>( pa );
		 	if ( prevAction3 )
			{
				prevAction3->_preUndo();
				m_showTime = prevAction3->m_showTime;
			}
			else
				return false;
		}
	}

	return true;
}

void SetLineTimesAction::_redo()
{
	if ( m_line.m_showTime != m_showTime )
	{
		Time aux = m_line.m_showTime;
		m_line.m_showTime = m_showTime;
		m_showTime = aux;
	}

	if ( m_line.m_hideTime != m_hideTime )
	{
		Time aux = m_line.m_hideTime;
		m_line.m_hideTime = m_hideTime;
		m_hideTime = aux;
	}
}

void SetLineTimesAction::_emitRedoSignals()
{
	if ( m_line.m_showTime != m_showTime )
	{
		m_line.emit showTimeChanged( m_line.m_showTime );
#ifdef PROPAGATE_LINE_SIGNALS
		if ( m_line.m_subtitle )
			m_line.m_subtitle->emit lineShowTimeChanged( &m_line, m_line.m_showTime );
#endif
	}

	if ( m_line.m_hideTime != m_hideTime )
	{
		m_line.emit hideTimeChanged( m_line.m_hideTime );
#ifdef PROPAGATE_LINE_SIGNALS
		if ( m_line.m_subtitle )
			m_line.m_subtitle->emit lineHideTimeChanged( &m_line, m_line.m_hideTime );
#endif
	}
}


/// SET LINE ERRORS ACTION
/// ======================

SetLineErrorsAction::SetLineErrorsAction( SubtitleLine& line, int errorFlags ):
	SubtitleLineAction( line, SubtitleAction::None, i18n( "Set Line Errors" ) ),
	m_errorFlags( errorFlags ) {}
SetLineErrorsAction::~SetLineErrorsAction() {}

bool SetLineErrorsAction::mergeWithPrevious( Action* pa )
{
	SetLineErrorsAction* prevAction = tryCastToThisLineAction<SetLineErrorsAction>( pa );
	if ( ! prevAction )
		return false;

	prevAction->_preUndo();
	m_errorFlags = prevAction->m_errorFlags;
	return true;
}

void SetLineErrorsAction::_redo()
{
	int aux = m_line.m_errorFlags;
	m_line.m_errorFlags = m_errorFlags;
	m_errorFlags = aux;
}

void SetLineErrorsAction::_emitRedoSignals()
{
	m_line.emit errorFlagsChanged( m_line.m_errorFlags );
#ifdef PROPAGATE_LINE_SIGNALS
	if ( m_line.m_subtitle )
		m_line.m_subtitle->emit lineErrorFlagsChanged( &m_line, m_line.m_errorFlags );
#endif
}
