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

#include "scripting_range.h"

using namespace SubtitleComposer;

/// RANGE IMPLEMENTATION
/// ====================

Scripting::Range::Range( const SubtitleComposer::Range& range, QObject* parent ):
	QObject( parent ),
	m_backend( range )
{
}

int Scripting::Range::start() const
{
	return m_backend.start();
}

int Scripting::Range::end() const
{
	return m_backend.end();
}

int Scripting::Range::length() const
{
	return m_backend.length();
}

bool Scripting::Range::contains( int index ) const
{
	return m_backend.contains( index );
}

bool Scripting::Range::contains( const QObject* object ) const
{
	if ( const Scripting::Range* range = qobject_cast<const Scripting::Range*>( object ) )
		return m_backend.contains( range->m_backend );
	else
		return false;
}

#include "scripting_range.moc"
