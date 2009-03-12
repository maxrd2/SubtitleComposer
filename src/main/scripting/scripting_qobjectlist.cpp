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

#include "scripting_qobjectlist.h"

using namespace SubtitleComposer;

Scripting::QObjectList::QObjectList( const char* contentClass, QObject* parent ):
	QObject( parent ),
	m_contentClass( contentClass )
{
}

Scripting::QObjectList::QObjectList( const QList<QObject*>& backend, const char* contentClass, QObject* parent ):
	QObject( parent ),
	m_contentClass( contentClass ),
	m_backend( backend )
{
}

bool Scripting::QObjectList::isEmpty() const
{
	return m_backend.isEmpty();
}

int Scripting::QObjectList::length() const
{
	return m_backend.count();
}

QObject* Scripting::QObjectList::at( int index ) const
{
	if ( index < 0 || index >= m_backend.count() )
		return 0;
	return m_backend.at( index );
}

void Scripting::QObjectList::insert( int index, QObject* object )
{
	if ( ! object || index < 0 || index > m_backend.count() || strcmp( object->metaObject()->className(), m_contentClass ) )
		return;
	m_backend.insert( index, object );
}

void Scripting::QObjectList::append( QObject* object )
{
	if ( ! object || strcmp( object->metaObject()->className(), m_contentClass ) )
		return;
	m_backend.append( object );
}

void Scripting::QObjectList::prepend( QObject* object )
{
	if ( object && ! strcmp( object->metaObject()->className(), m_contentClass ) )
		m_backend.prepend( object );
}

void Scripting::QObjectList::removeFirst()
{
	if ( ! m_backend.isEmpty() )
		return m_backend.removeFirst();
}

void Scripting::QObjectList::removeLast()
{
	if ( ! m_backend.isEmpty() )
		m_backend.removeLast();
}

void Scripting::QObjectList::removeAt( int index )
{
	if ( index >= 0 && index < m_backend.count() )
		m_backend.removeAt( index );
}

void Scripting::QObjectList::clear()
{
	m_backend.clear();
}

#include "scripting_qobjectlist.moc"
