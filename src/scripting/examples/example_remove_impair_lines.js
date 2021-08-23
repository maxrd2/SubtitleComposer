/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)

    SPDX-License-Identifier: GPL-2.0-or-later
*/

s = subtitle.instance();
for ( var lineIndex = s.linesCount() - 1; lineIndex >= 0; --lineIndex )
{
	if ( lineIndex % 2 == 0 )
		s.removeLine( lineIndex );
}
