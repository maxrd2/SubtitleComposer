#ifndef SCRIPTING_RANGELIST_H
#define SCRIPTING_RANGELIST_H

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

#include "../../core/rangelist.h"

#include <QtCore/QObject>

namespace SubtitleComposer
{
	namespace Scripting
	{
		class RangeList : public QObject
		{
			Q_OBJECT

			public slots:

				QObject* complement() const;

				bool isEmpty() const;

				int rangesCount() const;
				int indexesCount() const;

				int firstIndex() const;
				int lastIndex() const;

				QObject* range( int rangeIndex ) const;

				bool contains( int index ) const;

				void clear();

				void trimToIndex( int index );
				void trimToRange( const QObject* range );

				QObject* addIndex( int index );
				QObject* addRange( const QObject* range );

				void shiftIndexesForwards( int fromIndex, int delta, bool fillSplitGap );
				void shiftIndexesBackwards( int fromIndex, int delta );

			private:

				friend class RangesModule;
				friend class Subtitle;

				RangeList( const SubtitleComposer::RangeList& backend, QObject* parent );

				SubtitleComposer::RangeList m_backend;
		};
	}
}

#endif
