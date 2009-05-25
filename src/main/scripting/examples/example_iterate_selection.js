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

function RangesIterator( rangeList, forward )
{
	this.forward = typeof( forward ) == "undefined" ? true : forward;
	this.rangeList = rangeList;
	this.rangeIndex = this.forward ? -1 : this.rangeList.rangesCount()

	this.current = function() {
		return this.rangeIndex >= 0 && this.rangeIndex < this.rangeList.rangesCount() ?
 			this.rangeList.range( this.rangeIndex ) :
			null;
	};

	this.hasNext = function() {
		return this.forward ?
			this.rangeIndex < this.rangeList.rangesCount() - 1 :
			this.rangeIndex > 0;
	};

	this.next = function() {
		this.rangeIndex += this.forward ? 1 : -1;
		return this.current();
	};
}

function LinesIterator( rangeList, forward )
{
	this.forward = typeof( forward ) == "undefined" ? true : forward;
	this.rangesIt = new RangesIterator( rangeList, this.forward );
	this.lineIndex = -1;
	this.subtitle = subtitle.instance();

	this.current = function() {
		var range = this.rangesIt.current();
		return range != null && this.lineIndex >= range.start() && this.lineIndex < range.end()?
 			this.subtitle.line( this.lineIndex ) :
			null;
	};

	this.hasNext = function() {
		return this.rangesIt.hasNext() || (this.rangesIt.current() && (this.forward ?
			this.lineIndex < this.rangesIt.current().end() :
			this.lineIndex > this.rangesIt.current().start()));
	};

	this.next = function() {
		var currentRange = this.rangesIt.current();
		if ( currentRange == null
			 || (forward && this.lineIndex == currentRange.end())
			 || (! forward && this.lineIndex == currentRange.start()) )
		{
			if ( ! this.rangesIt.hasNext() )
				return null;
			currentRange = this.rangesIt.next();
			this.lineIndex = this.forward ? currentRange.start() : currentRange.end();
		}
		else
			this.lineIndex += this.forward ? 1 : -1;
		return this.subtitle.line( this.lineIndex )
	};
}

var rangesIt = new RangesIterator( ranges.newSelectionRangeList(), true );
while ( rangesIt.hasNext() )
{
	var range = rangesIt.next();
	debug.information( range.start().toString() + ":" + range.end().toString() );
}

var linesIt = new LinesIterator( ranges.newSelectionRangeList(), false );
while ( linesIt.hasNext() )
{
	var line = linesIt.next();
	debug.information( "Plain text: " + line.plainPrimaryText() + "\n\nRich Text: " + line.richPrimaryText() );
}
