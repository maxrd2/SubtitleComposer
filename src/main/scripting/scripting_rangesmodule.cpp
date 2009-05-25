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

#include "scripting_rangesmodule.h"
#include "scripting_rangelist.h"
#include "scripting_range.h"
#include "../application.h"
#include "../lineswidget.h"

using namespace SubtitleComposer;

Scripting::RangesModule::RangesModule( QObject* parent ):
	QObject( parent )
{
}

QObject* Scripting::RangesModule::newRange( int firstIndex, int lastIndex )
{
	if ( firstIndex > lastIndex || firstIndex < 0 || lastIndex < 0 )
		return 0;
	return new Scripting::Range( SubtitleComposer::Range( firstIndex, lastIndex ), this );
}

QObject* Scripting::RangesModule::newLowerRange( int index )
{
	return new Scripting::Range( SubtitleComposer::Range::lower( index ), this );
}

QObject* Scripting::RangesModule::newUpperRange( int index )
{
	return new Scripting::Range( SubtitleComposer::Range::upper( index ), this );
}

QObject* Scripting::RangesModule::newUptoLastSelectedRange()
{
	int index = app()->linesWidget()->lastSelectedIndex();
	return index < 0 ? 0 : new Scripting::Range( SubtitleComposer::Range::lower( index ), this );
}

QObject* Scripting::RangesModule::newFromFirstSelectedRange()
{
	int index = app()->linesWidget()->firstSelectedIndex();
	return index < 0 ? 0 : new Scripting::Range( SubtitleComposer::Range::upper( index ), this );
}


QObject* Scripting::RangesModule::newEmptyRangeList()
{
	return new Scripting::RangeList( SubtitleComposer::RangeList(), this );
}

QObject* Scripting::RangesModule::newSelectionRangeList()
{
	return new Scripting::RangeList( app()->linesWidget()->selectionRanges(), this );
}

#include "scripting_rangesmodule.moc"
