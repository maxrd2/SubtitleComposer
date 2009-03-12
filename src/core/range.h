#ifndef RANGE_H
#define RANGE_H

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

#include <climits>

#include <QtCore/QList>

#include <KDebug>

namespace SubtitleComposer
{
	class Range
	{
		public:

			static const int MaxIndex = INT_MAX;

			Range( int index ):
				m_start( index ),
				m_end( index )
			{
				Q_ASSERT( index >= 0 );
				Q_ASSERT( index <= MaxIndex );
			}

			Range( int startIndex, int endIndex ):
				m_start( startIndex ),
				m_end( endIndex )
			{
				Q_ASSERT( m_start >= 0 );
				Q_ASSERT( m_end >= 0 );
				Q_ASSERT( m_start <= m_end );
				Q_ASSERT( m_start <= MaxIndex );
			}

			Range( const Range& range ):
				m_start( range.m_start ),
				m_end( range.m_end )
			{
			}

			Range& operator=( const Range& range )
			{
				if ( this == &range )
					return *this;

				m_start = range.m_start;
				m_end = range.m_end;

				return *this;
			}

			inline static Range full()
			{
				return Range( 0, MaxIndex );
			}

			inline static Range lower( int index )
			{
				Q_ASSERT( index >= 0 );
				Q_ASSERT( index <= MaxIndex );
				return Range( 0, index );
			}

			inline static Range upper( int index )
			{
				Q_ASSERT( index >= 0 );
				Q_ASSERT( index <= MaxIndex );
				return Range( index, MaxIndex );
			}

			inline bool isFullRange( int maxIndex=MaxIndex ) const
			{
				return m_start == 0 && m_end == maxIndex;
			}

			inline int start() const { return m_start; }
			inline int end() const { return m_end; }
			inline int length() const { return m_end - m_start + 1; }

			inline bool contains( int index ) const
			{
				return index >= m_start && index <= m_end;
			}

			inline bool contains( const Range& range ) const
			{
				return m_end <= range.m_end && m_start >= range.m_start;
			}

			inline bool operator==( const Range& range ) const
			{
				return m_start == range.m_start && m_end == range.m_end;
			}

			inline bool operator!=( const Range& range ) const
			{
				return m_start != range.m_start || m_end != range.m_end;
			}

			inline bool operator<( const Range& range ) const
			{
				return m_end < range.m_start;
			}

			inline bool operator<=( const Range& range ) const
			{
				return m_end <= range.m_end && m_start <= range.m_start;
			}

			inline bool operator>( const Range& range ) const
			{
				return m_start > range.m_end;
			}

			inline bool operator>=( const Range& range ) const
			{
				return m_start >= range.m_start && m_end >= range.m_end;
			}

		private:

			friend class RangeList;

			Range() {}

			int m_start;
			int m_end;
	};
}

#endif
