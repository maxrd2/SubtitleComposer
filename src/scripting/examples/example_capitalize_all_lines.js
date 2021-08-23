/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

s = subtitle.instance();
for ( var lineIndex = 0, linesCount = s.linesCount(); lineIndex < linesCount; ++lineIndex )
{
	var line = s.line( lineIndex );
	var text = line.primaryText();
	line.setPrimaryText( text.left( 1 ).toUpper().append( text.mid( 1 ) ) );
}
