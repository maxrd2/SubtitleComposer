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

#include "scripting_styles.h"

using namespace SubtitleComposer;

Scripting::StylesModule::StylesModule( QObject* parent ):
	QObject( parent )
{
}

int Scripting::StylesModule::cummulativeFlags( const QString& text ) const
{
	SString sText;
	sText.setRichString( text );
	return sText.cummulativeStyleFlags();
}

QString Scripting::StylesModule::setFlags( const QString& text, int styleFlags ) const
{
	SString sText;
	sText.setRichString( text );
	sText.setStyleFlags( 0, sText.length(), styleFlags );
	return sText.richString();
}

QString Scripting::StylesModule::setFlags( const QString& text, int styleFlags, bool on ) const
{
	SString sText;
	sText.setRichString( text );
	sText.setStyleFlags( 0, sText.length(), styleFlags, on );
	return sText.richString();
}

QString Scripting::StylesModule::setFlags( const QString& text, int index, int len, int styleFlags ) const
{
	if ( index < 0 || ! len )
		return text;

	SString sText;
	sText.setRichString( text );

	if ( index >= sText.length() )
		return text;

	sText.setStyleFlags( index, len, styleFlags );
	return sText.richString();
}

QString Scripting::StylesModule::setFlags( const QString& text, int index, int len, int styleFlags, bool on ) const
{
	if ( index < 0 || ! len )
		return text;

	SString sText;
	sText.setRichString( text );

	if ( index >= sText.length() )
		return text;

	sText.setStyleFlags( index, len, styleFlags, on );
	return sText.richString();
}

#include "scripting_styles.moc"
