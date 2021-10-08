/*
	SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>

	SPDX-License-Identifier: GPL-2.0-or-later

	@category Examples
	@name Remove Odd Lines
	@version 1.0
	@summary Example script to remove odd lines.
	@author SubtitleComposer Team
*/

let s = subtitle.instance();
for(let i = s.linesCount() - 1; i >= 0; i--) {
	if(i % 2 == 0)
		s.removeLine(i);
}
