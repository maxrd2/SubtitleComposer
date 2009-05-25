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

#include "scripting_subtitle.h"
#include "scripting_subtitleline.h"
#include "scripting_range.h"
#include "scripting_rangelist.h"
#include "../application.h"

using namespace SubtitleComposer;

Scripting::Subtitle::Subtitle( SubtitleComposer::Subtitle* backend, QObject* parent ):
	QObject( parent ),
	m_backend( backend )
{
}

double Scripting::Subtitle::framesPerSecond() const
{
	return m_backend->framesPerSecond();
}

void Scripting::Subtitle::setFramesPerSecond( double framesPerSecond )
{
	return m_backend->setFramesPerSecond( framesPerSecond );
}

void Scripting::Subtitle::changeFramesPerSecond( double toFramesPerSecond, double fromFramesPerSecond )
{
	return m_backend->changeFramesPerSecond( toFramesPerSecond, fromFramesPerSecond );
}

bool Scripting::Subtitle::isEmpty() const
{
	return m_backend->isEmpty();
}

int Scripting::Subtitle::linesCount() const
{
	return m_backend->linesCount();
}

int Scripting::Subtitle::lastIndex() const
{
	return m_backend->lastIndex();
}

QObject* Scripting::Subtitle::line( int index )
{
	if ( index < 0 || index > (m_backend->lastIndex()) )
		return 0;
	return new Scripting::SubtitleLine( m_backend->line( index ), this );
}

QObject* Scripting::Subtitle::firstLine()
{
	if ( m_backend->isEmpty() )
		return 0;
	return new Scripting::SubtitleLine( m_backend->firstLine(), this );
}

QObject* Scripting::Subtitle::lastLine()
{
	if ( m_backend->isEmpty() )
		return 0;
	return new Scripting::SubtitleLine( m_backend->lastLine(), this );
}

SubtitleComposer::RangeList Scripting::Subtitle::toRangesList( const QObject* object )
{
	const Scripting::Range* range = qobject_cast<const Scripting::Range*>( object );
	if ( range )
		return range->m_backend;
	const Scripting::RangeList* rangeList = qobject_cast<const Scripting::RangeList*>( object );
	if ( rangeList )
		return rangeList->m_backend;
	return SubtitleComposer::RangeList();
}

static SubtitleComposer::Subtitle::TextTarget toTextTarget( int value, int opDefault )
{
	if ( value < SubtitleComposer::Subtitle::Primary || value >= SubtitleComposer::Subtitle::TextTargetSIZE )
		return (SubtitleComposer::Subtitle::TextTarget)opDefault;
	return (SubtitleComposer::Subtitle::TextTarget)value;
}

Scripting::SubtitleLine* Scripting::Subtitle::insertNewLine( int index, bool timeAfter, int target )
{
	if ( index > m_backend->linesCount() )
		return 0;
	static const int opDefault = SubtitleComposer::Subtitle::Both;
	return new Scripting::SubtitleLine( m_backend->insertNewLine( index, timeAfter, toTextTarget( target, opDefault ) ), this );
}

void Scripting::Subtitle::removeLine( int index, int target )
{
	if ( index < 0 || index >= m_backend->linesCount() )
		return;
	static const int opDefault = SubtitleComposer::Subtitle::Both;
	m_backend->removeLines( SubtitleComposer::Range( index ), toTextTarget( target, opDefault ) );
}

void Scripting::Subtitle::removeLines( const QObject* ranges, int target )
{
	static const int opDefault = SubtitleComposer::Subtitle::Both;
	m_backend->removeLines( toRangesList( ranges ), toTextTarget( target, opDefault ) );
}

void Scripting::Subtitle::swapTexts( const QObject* ranges )
{
	m_backend->swapTexts( toRangesList( ranges ) );
}

void Scripting::Subtitle::splitLines( const QObject* ranges )
{
	m_backend->splitLines( toRangesList( ranges ) );
}

void Scripting::Subtitle::joinLines( const QObject* ranges )
{
	m_backend->joinLines( toRangesList( ranges ) );
}

void Scripting::Subtitle::shiftLines( const QObject* ranges, int msecs )
{
	m_backend->shiftLines( toRangesList( ranges ), msecs );
}

void Scripting::Subtitle::adjustLines( const QObject* object, int firstTime, int lastTime )
{
	if ( const Scripting::Range* range = qobject_cast<const Scripting::Range*>( object ) )
		m_backend->adjustLines( range->m_backend, firstTime, lastTime );
}

void Scripting::Subtitle::sortLines( const QObject* object )
{
	if ( const Scripting::Range* range = qobject_cast<const Scripting::Range*>( object ) )
		m_backend->sortLines( range->m_backend );
}

void Scripting::Subtitle::applyDurationLimits( const QObject* ranges, int minDuration, int maxDuration, bool canOverlap )
{
	m_backend->applyDurationLimits( toRangesList( ranges ), minDuration, maxDuration, canOverlap );
}

void Scripting::Subtitle::setMaximumDurations( const QObject* ranges )
{
	m_backend->setMaximumDurations( toRangesList( ranges ) );
}

void Scripting::Subtitle::setAutoDurations( const QObject* ranges, int msecsPerChar, int msecsPerWord, int msecsPerLine, bool canOverlap, int calculationTarget )
{
	const int opDefault = app()->translationMode() ? SubtitleComposer::Subtitle::Both : SubtitleComposer::Subtitle::Primary;

	m_backend->setAutoDurations(
		toRangesList( ranges ),
		msecsPerChar, msecsPerWord, msecsPerLine,
		canOverlap, toTextTarget( calculationTarget, opDefault )
	);
}

void Scripting::Subtitle::fixOverlappingLines( const QObject* ranges, int minInterval )
{
	m_backend->fixOverlappingLines( toRangesList( ranges ), minInterval );
}

void Scripting::Subtitle::fixPunctuation( const QObject* ranges, bool spaces, bool quotes, bool englishI, bool ellipsis, int target )
{
	const int opDefault = app()->translationMode() ? SubtitleComposer::Subtitle::Both : SubtitleComposer::Subtitle::Primary;
	m_backend->fixPunctuation( toRangesList( ranges ), spaces, quotes, englishI, ellipsis, toTextTarget( target, opDefault ) );
}

void Scripting::Subtitle::lowerCase( const QObject* ranges, int target )
{
	const int opDefault = app()->translationMode() ? SubtitleComposer::Subtitle::Both : SubtitleComposer::Subtitle::Primary;
	m_backend->lowerCase( toRangesList( ranges ), toTextTarget( target, opDefault ) );
}

void Scripting::Subtitle::upperCase( const QObject* ranges, int target )
{
	const int opDefault = app()->translationMode() ? SubtitleComposer::Subtitle::Both : SubtitleComposer::Subtitle::Primary;
	m_backend->upperCase( toRangesList( ranges ), toTextTarget( target, opDefault ) );
}

void Scripting::Subtitle::titleCase( const QObject* ranges, bool lowerFirst, int target )
{
	const int opDefault = app()->translationMode() ? SubtitleComposer::Subtitle::Both : SubtitleComposer::Subtitle::Primary;
	m_backend->titleCase( toRangesList( ranges ), lowerFirst, toTextTarget( target, opDefault ) );
}

void Scripting::Subtitle::sentenceCase( const QObject* ranges, bool lowerFirst, int target )
{
	const int opDefault = app()->translationMode() ? SubtitleComposer::Subtitle::Both : SubtitleComposer::Subtitle::Primary;
	m_backend->sentenceCase( toRangesList( ranges ), lowerFirst, toTextTarget( target, opDefault ) );
}

void Scripting::Subtitle::breakLines( const QObject* ranges, int minLengthForLineBreak, int target )
{
	const int opDefault = app()->translationMode() ? SubtitleComposer::Subtitle::Both : SubtitleComposer::Subtitle::Primary;
	m_backend->breakLines( toRangesList( ranges ), minLengthForLineBreak, toTextTarget( target, opDefault ) );
}

void Scripting::Subtitle::unbreakTexts( const QObject* ranges, int target )
{
	const int opDefault = app()->translationMode() ? SubtitleComposer::Subtitle::Both : SubtitleComposer::Subtitle::Primary;
	m_backend->unbreakTexts( toRangesList( ranges ), toTextTarget( target, opDefault ) );
}

void Scripting::Subtitle::simplifyTextWhiteSpace( const QObject* ranges, int target )
{
	const int opDefault = app()->translationMode() ? SubtitleComposer::Subtitle::Both : SubtitleComposer::Subtitle::Primary;
	m_backend->simplifyTextWhiteSpace( toRangesList( ranges ), toTextTarget( target, opDefault ) );
}

void Scripting::Subtitle::setMarked( const QObject* ranges, bool value )
{
	m_backend->setMarked( toRangesList( ranges ), value );
}

void Scripting::Subtitle::clearErrors( const QObject* ranges, int errorFlags )
{
	m_backend->clearErrors( toRangesList( ranges ), errorFlags );
}

void Scripting::Subtitle::checkErrors( const QObject* ranges, int errorFlags, int minDuration, int maxDuration,
									   int minDurationPerChar, int maxDurationPerChar, int maxChars, int maxLines )
{
	m_backend->checkErrors(
		toRangesList( ranges ), errorFlags,
		minDuration, maxDuration,
		minDurationPerChar, maxDurationPerChar,
		maxChars, maxLines
	);
}

void Scripting::Subtitle::recheckErrors( const QObject* ranges, int minDuration, int maxDuration,
										 int minDurationPerChar, int maxDurationPerChar, int maxChars, int maxLines )
{
	m_backend->recheckErrors(
		toRangesList( ranges ),
		minDuration, maxDuration,
		minDurationPerChar, maxDurationPerChar,
		maxChars, maxLines
	);
}

#include "scripting_subtitle.moc"
