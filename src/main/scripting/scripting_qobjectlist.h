#ifndef SCRIPTING_QOBJECTLIST_H
#define SCRIPTING_QOBJECTLIST_H

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

#include <QtCore/QObject>
#include <QtCore/QList>

namespace SubtitleComposer
{
	namespace Scripting
	{
		class QObjectList : public QObject
		{
			Q_OBJECT

			Q_PROPERTY( bool isEmpty READ isEmpty )

			Q_PROPERTY( int length READ length )
			Q_PROPERTY( int size READ length )
			Q_PROPERTY( int count READ length )

			public:

				QObjectList( const char* contentClass, QObject* parent );
				QObjectList( const QList<QObject*>& backend, const char* contentClass, QObject* parent );

			public slots:

				bool isEmpty() const;
				int length() const;

				QObject* at( int index ) const;

				void insert( int index, QObject* object );
				void append( QObject* object );
				void prepend( QObject* object );

				void removeFirst();
				void removeLast();
				void removeAt( int index );

				void clear();

			private:

				const char* m_contentClass;
				QList<QObject*> m_backend;
		};
	}
}

#endif
