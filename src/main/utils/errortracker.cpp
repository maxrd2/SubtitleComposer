
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

#include "errortracker.h"
#include "../application.h"
#include "../../core/subtitleline.h"

using namespace SubtitleComposer;

ErrorTracker::ErrorTracker( QObject* parent ):
	QObject( parent ),
	m_subtitle( 0 ),
	m_autoClearFixed( app()->errorsConfig()->autoClearFixed() ),
	m_minDuration( app()->errorsConfig()->minDuration() ),
	m_maxDuration( app()->errorsConfig()->maxDuration() ),
	m_minDurationPerChar( app()->errorsConfig()->minDurationPerChar() ),
	m_maxDurationPerChar( app()->errorsConfig()->maxDurationPerChar() ),
	m_maxCharacters( app()->errorsConfig()->maxCharacters() ),
	m_maxLines( app()->errorsConfig()->maxLines() )
{
	connect( app()->errorsConfig(), SIGNAL( optionChanged( const QString&,const QString& ) ),
			 this, SLOT( onErrorsOptionChanged( const QString&,const QString& ) ) );
}

ErrorTracker::~ErrorTracker()
{
}

bool ErrorTracker::isTracking() const
{
	return m_autoClearFixed && m_subtitle;
}

void ErrorTracker::setSubtitle( Subtitle* subtitle )
{
	if ( isTracking() )
		disconnectSlots();
	m_subtitle = subtitle;
	if ( isTracking() )
		connectSlots();
}

void ErrorTracker::connectSlots()
{
	connect( m_subtitle, SIGNAL( linePrimaryTextChanged( SubtitleLine*, const SString& ) ),
				this, SLOT( onLinePrimaryTextChanged( SubtitleLine* ) ) );
	connect( m_subtitle, SIGNAL( lineSecondaryTextChanged( SubtitleLine*, const SString& ) ),
				this, SLOT( onLineSecondaryTextChanged( SubtitleLine* ) ) );
	connect( m_subtitle, SIGNAL( lineShowTimeChanged( SubtitleLine*, const Time& ) ),
				this, SLOT( onLineTimesChanged( SubtitleLine* ) ) );
	connect( m_subtitle, SIGNAL( lineHideTimeChanged( SubtitleLine*, const Time& ) ),
				this, SLOT( onLineTimesChanged( SubtitleLine* ) ) );
}

void ErrorTracker::disconnectSlots()
{
	disconnect( m_subtitle, SIGNAL( linePrimaryTextChanged( SubtitleLine*, const SString& ) ),
				this, SLOT( onLinePrimaryTextChanged( SubtitleLine* ) ) );
	disconnect( m_subtitle, SIGNAL( lineSecondaryTextChanged( SubtitleLine*, const SString& ) ),
				this, SLOT( onLineSecondaryTextChanged( SubtitleLine* ) ) );
	disconnect( m_subtitle, SIGNAL( lineShowTimeChanged( SubtitleLine*, const Time& ) ),
				this, SLOT( onLineTimesChanged( SubtitleLine* ) ) );
	disconnect( m_subtitle, SIGNAL( lineHideTimeChanged( SubtitleLine*, const Time& ) ),
				this, SLOT( onLineTimesChanged( SubtitleLine* ) ) );
}

void ErrorTracker::updateLineErrors( SubtitleLine* line, int errorFlags ) const
{
	line->check(
		errorFlags,
		m_minDuration,
		m_maxDuration,
		m_minDurationPerChar,
		m_maxDurationPerChar,
		m_maxCharacters,
		m_maxLines
	);
}

void ErrorTracker::onLinePrimaryTextChanged( SubtitleLine* line )
{
	updateLineErrors( line, line->errorFlags() & SubtitleLine::PrimaryOnlyErrors );
}

void ErrorTracker::onLineSecondaryTextChanged( SubtitleLine* line )
{
	updateLineErrors( line, line->errorFlags() & SubtitleLine::SecondaryOnlyErrors );
}

void ErrorTracker::onLineTimesChanged( SubtitleLine* line )
{
	updateLineErrors( line, line->errorFlags() & SubtitleLine::TimesErrors );

	SubtitleLine* prevLine = line->prevLine();
	if ( prevLine )
		updateLineErrors( prevLine, prevLine->errorFlags() & SubtitleLine::OverlapsWithNext );
}

void ErrorTracker::onErrorsOptionChanged( const QString& /*optionName*/, const QString& /*value*/ )
{
	ErrorsConfig* errorsConfig = app()->errorsConfig();

	if ( m_autoClearFixed != errorsConfig->autoClearFixed() ) // is this option that has been toggled
	{
		if ( isTracking() )
			disconnectSlots();
		m_autoClearFixed = ! m_autoClearFixed;
		if ( isTracking() )
			connectSlots();
	}
	else
	{
		m_minDuration = errorsConfig->minDuration();
		m_maxDuration = errorsConfig->maxDuration();
		m_minDurationPerChar = errorsConfig->minDurationPerChar();
		m_maxDurationPerChar = errorsConfig->maxDurationPerChar();
		m_maxCharacters = errorsConfig->maxCharacters();
		m_maxLines = errorsConfig->maxLines();
	}
}

#include "errortracker.moc"
