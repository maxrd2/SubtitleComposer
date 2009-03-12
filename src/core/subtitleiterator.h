#ifndef SUBTITLEITERATOR_H
#define SUBTITLEITERATOR_H

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

#include "subtitleline.h"
#include "subtitle.h"
#include "range.h"
#include "rangelist.h"

#include <QtCore/QObject>
#include <QtCore/QList>

namespace SubtitleComposer
{
	class SubtitleIterator : public QObject
	{
		Q_OBJECT

		Q_PROPERTY( int index READ index WRITE toIndex );

		public:

			static const int AfterLast = -1;
			static const int BehindFirst = -2;
			static const int Invalid = -3;

			explicit SubtitleIterator( const Subtitle& subtitle, const RangeList& ranges=Range::full(), bool toLast=false );
			SubtitleIterator( const SubtitleIterator& it );
			SubtitleIterator& operator=( const SubtitleIterator& it );
			virtual ~SubtitleIterator();

			bool isAutoSync() const;
			void setAutoSync( bool value );

			bool isAutoCircle() const;
			void setAutoCircle( bool value );

			bool isFullIterator() const;
			RangeList ranges() const;

			void toFirst();
			void toLast();
			bool toIndex( int index );

			inline int index() { return m_index; }
			inline int firstIndex() { return m_index == Invalid ? -1 : m_ranges.firstIndex(); }
			inline int lastIndex() { return m_index == Invalid ? -1 : m_ranges.lastIndex(); }

			inline SubtitleLine* current() const { return m_index < 0 ? 0 : *m_linesIterator; }
			inline operator SubtitleLine*() const { return m_index < 0 ? 0 : *m_linesIterator; }

			SubtitleIterator& operator++();
			SubtitleIterator& operator+=( int steps );
			SubtitleIterator& operator--();
			SubtitleIterator& operator-=( int steps );

		signals:

			void syncronized( int firstIndex, int lastIndex, bool inserted );

		private slots:

			void onSubtitleLinesInserted( int firstIndex, int lastIndex );
			void onSubtitleLinesRemoved( int firstIndex, int lastIndex );

		private:

			const Subtitle* m_subtitle;
			bool m_autoSync;
			bool m_autoCircle;
			RangeList m_ranges;
			bool m_isFullIterator;
			int m_index;
			RangeList::ConstIterator m_rangesIterator;
			QList<SubtitleLine*>::Iterator m_linesIterator;
			QList<SubtitleLine*>::Iterator m_linesIteratorStart;
	};
}

#endif
