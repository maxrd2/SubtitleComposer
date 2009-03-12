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

#include "subtitleiterator.h"
#include "subtitleline.h"

using namespace SubtitleComposer;

SubtitleIterator::SubtitleIterator( const Subtitle& subtitle, const RangeList& ranges, bool gotoLast ):
	QObject( 0 ),
	m_subtitle( &subtitle ),
	m_autoSync( false ),
	m_autoCircle( false ),
	m_ranges( ranges ),
	m_linesIterator( m_subtitle->m_lines.begin() ),
	m_linesIteratorStart( m_subtitle->m_lines.begin() )
{
	if ( m_subtitle->isEmpty() )
		m_ranges.clear();
	else
		m_ranges.trimToIndex( m_subtitle->lastIndex() );

	m_isFullIterator = m_ranges.isFullRange( m_subtitle->lastIndex() );

	if ( m_ranges.isEmpty() )
	{
		m_index = Invalid; // no operations allowed
		if ( m_subtitle->linesCount() )
			kDebug() << "SubtitleIterator requested with empty ranges list";
	}
	else
	{
		m_index = Invalid - 1; // a non INVALID index (needed only for initialization)
		if ( gotoLast )
			toLast();
		else
			toFirst();
	}
}

SubtitleIterator::SubtitleIterator( const SubtitleIterator& it ):
	QObject( 0 ),
	m_subtitle( it.m_subtitle ),
	m_autoSync( false ),
	m_autoCircle( it.m_autoCircle ),
	m_ranges( it.m_ranges ),
	m_isFullIterator( it.m_isFullIterator ),
	m_index( it.m_index ),
	m_rangesIterator( it.m_rangesIterator ),
	m_linesIterator( it.m_linesIterator ),
	m_linesIteratorStart( it.m_linesIteratorStart )
{
	setAutoSync( it.m_autoSync );
}

SubtitleIterator& SubtitleIterator::operator=( const SubtitleIterator& it )
{
	if ( &it != this )
	{
		setAutoSync( false );

		m_subtitle = it.m_subtitle;
		m_autoCircle = it.m_autoCircle;
		m_ranges = it.m_ranges;
		m_isFullIterator = it.m_isFullIterator;
		m_index = it.m_index;
		m_rangesIterator = it.m_rangesIterator;
		m_linesIterator = it.m_linesIterator;
		m_linesIteratorStart = it.m_linesIteratorStart;

		setAutoSync( it.m_autoSync );
	}

	return *this;
}

SubtitleIterator::~SubtitleIterator()
{
	setAutoSync( false );
}

bool SubtitleIterator::isAutoSync() const
{
	return m_autoSync;
}

void SubtitleIterator::setAutoSync( bool value )
{
	if ( m_autoSync != value )
	{
		if ( m_autoSync )
		{
			disconnect( m_subtitle, SIGNAL( linesInserted(int,int) ), this, SLOT( onSubtitleLinesInserted(int,int) ) );
			disconnect( m_subtitle, SIGNAL( linesRemoved(int,int) ), this, SLOT( onSubtitleLinesRemoved(int,int) ) );
		}

		m_autoSync = value;

		if ( m_autoSync )
		{
			connect( m_subtitle, SIGNAL( linesInserted(int,int) ), this, SLOT( onSubtitleLinesInserted(int,int) ) );
			connect( m_subtitle, SIGNAL( linesRemoved(int,int) ), this, SLOT( onSubtitleLinesRemoved(int,int) ) );
		}
	}
}

bool SubtitleIterator::isAutoCircle() const
{
	return m_autoCircle;
}

void SubtitleIterator::setAutoCircle( bool value )
{
	if ( m_autoCircle != value )
	{
		m_autoCircle = value;

		if ( m_index == AfterLast )
			toFirst();
		else if ( m_index == BehindFirst )
			toLast();
	}
}

bool SubtitleIterator::isFullIterator() const
{
	return m_isFullIterator;
}

RangeList SubtitleIterator::ranges() const
{
	return m_ranges;
}

void SubtitleIterator::toFirst()
{
	if ( m_index == Invalid )
		return;

	m_rangesIterator = m_ranges.begin();

	m_linesIteratorStart = m_subtitle->m_lines.begin();

	m_linesIterator = m_linesIteratorStart;

	m_index = m_ranges.firstIndex();
	if ( m_index )
		m_linesIterator += m_index; // safe because indexes within m_ranges are all valid lines
}

void SubtitleIterator::toLast()
{
	if ( m_index == Invalid )
		return;

	m_rangesIterator = m_ranges.end();
	m_rangesIterator--; // safe because m_ranges is not empty (otherwise m_index would be INVALID).

	m_linesIteratorStart = m_subtitle->m_lines.begin();

	m_linesIterator = m_linesIteratorStart;

	m_index = m_ranges.lastIndex();
	if ( m_index )
		m_linesIterator += m_index; // safe because indexes within m_ranges are all valid lines
}

bool SubtitleIterator::toIndex( const int index )
{
	Q_ASSERT( index >= 0 );
	Q_ASSERT( index <= Range::MaxIndex );

	if ( m_index == Invalid )
		return false;

	bool savedAutoCircle = m_autoCircle; // auto circling interferes with what we have to do

	m_autoCircle = false;

	if ( m_index < index )
	{
		while ( m_index < index )
		{
			operator++();
			if ( m_index == AfterLast )
			{
				toLast(); // equivalent to operator--() in this case
				break;
			}
		}
	}
	else if ( m_index > index )
	{
		while ( m_index > index )
		{
			operator--();
			if ( m_index == BehindFirst )
			{
				toFirst(); // equivalent to operator++() in this case
				break;
			}
		}
	}

	m_autoCircle = savedAutoCircle;

	return m_index == index;
}

SubtitleIterator& SubtitleIterator::operator++()
{
	if ( m_index == Invalid || m_index == AfterLast )
		return *this;

	if ( m_index == BehindFirst )
		toFirst();
	else
	{
		m_index++;
		++m_linesIterator;

		int currentRangeEnd = (*m_rangesIterator).end();
		if ( m_index > currentRangeEnd )
		{
			m_rangesIterator++;
			if ( m_rangesIterator == m_ranges.end() )
				m_index = AfterLast;
			else
			{
				m_index = (*m_rangesIterator).start();
				m_linesIterator += (m_index - (currentRangeEnd + 1));
			}
		}
	}

	if ( m_autoCircle && m_index == AfterLast )
		toFirst();

	return *this;
}


SubtitleIterator& SubtitleIterator::operator--()
{
	if (  m_index == Invalid || m_index == BehindFirst )
		return *this;

	if ( m_index == AfterLast )
		toLast();
	else
	{
		m_index--;
		--m_linesIterator;

		int currentRangeStart = (*m_rangesIterator).start();
		if ( m_index < currentRangeStart )
		{
			if ( m_rangesIterator == m_ranges.begin() )
				m_index = BehindFirst;
			else
			{
				m_rangesIterator--;
				m_index = (*m_rangesIterator).end();
				m_linesIterator -= (currentRangeStart - (m_index + 1));
			}
		}
	}

	if ( m_autoCircle && m_index == BehindFirst )
		toLast();

	return *this;
}

SubtitleIterator& SubtitleIterator::operator+=( int steps )
{
	if ( steps > 0 )
	{
		for ( int i = 0; i < steps; ++i )
			operator++();
	}
	else if ( steps < 0 )
	{
		for ( int i = 0; i > steps; --i )
			operator--();
	}

	return *this;
}

SubtitleIterator& SubtitleIterator::operator-=( int steps )
{
	if ( steps > 0 )
	{
		for ( int i = 0; i < steps; ++i )
			operator--();
	}
	else if ( steps < 0 )
	{
		for ( int i = 0; i > steps; --i )
			operator++();
	}

	return *this;
}

void SubtitleIterator::onSubtitleLinesInserted( int firstIndex, int lastIndex )
{
	if ( m_index == Invalid )
		return;

	int prevIndex = m_index;
	Range insertedRange( firstIndex, lastIndex );

	m_ranges.shiftIndexesForwards( firstIndex, insertedRange.length(), true );
	if ( m_isFullIterator )
		m_ranges << insertedRange;

	m_index = Invalid - 1; // a non Invalid index (needed only for initialization)

	if ( prevIndex == AfterLast )
	{
		if ( lastIndex == m_ranges.lastIndex() ) // lines were inserted at the end
		{
			toFirst(); // restore internal variables to a valid state
			toIndex( firstIndex ); // point to the first newly inserted item
		}
		else
		{
			toLast(); // restore internal variables to a valid state
			operator++(); // point again to AfterLast
		}
	}
	else if ( prevIndex == BehindFirst )
	{
		toFirst(); // restore internal variables to a valid state
		operator--(); // point again to BehindFirst
	}
	else
	{
		toFirst(); // restore internal variables to a valid state
		if ( prevIndex >= firstIndex )
			toIndex( prevIndex + insertedRange.length() ); // point to the previously pointed line
		else
			toIndex( prevIndex ); // point to the previously pointed line
	}

	emit syncronized( firstIndex, lastIndex, true );
}

void SubtitleIterator::onSubtitleLinesRemoved( int firstIndex, int lastIndex )
{
	if ( m_index == Invalid )
		return;

	int prevIndex = m_index;
	Range removedRange( firstIndex, lastIndex );

	kDebug() << "PREV RANGES" << m_ranges.inspect();
	m_ranges.shiftIndexesBackwards( firstIndex, removedRange.length() );
	kDebug() << "NEW RANGES" << m_ranges.inspect();

	if ( m_ranges.isEmpty() )
		m_index = Invalid;
	else
	{
		m_index = Invalid - 1; // a non Invalid index (needed only for initialization)

		if ( prevIndex == AfterLast )
		{
			toLast(); // restore internal variables to a valid state
			operator++(); // point again to AfterLast
		}
		else if ( prevIndex == BehindFirst )
		{
			toFirst(); // restore internal variables to a valid state
			operator--(); // point again to BehindFirst
		}
		else
		{
			toFirst(); // restore internal variables to a valid state
			if ( prevIndex < firstIndex )
				toIndex( prevIndex );
			else if ( m_index > lastIndex )
				toIndex( prevIndex - removedRange.length() );
			else // prevIndex was one of the removed lines
				toIndex( firstIndex );
		}
	}

	emit syncronized( firstIndex, lastIndex, false );
}

#include "subtitleiterator.moc"
