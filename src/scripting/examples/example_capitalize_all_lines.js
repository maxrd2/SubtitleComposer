/*
	SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>

	SPDX-License-Identifier: GPL-2.0-or-later

	@category Examples
	@name Capitalize All Lines
	@version 1.0
	@summary Example script to capitalize first letter of each subtitle line.
	@author SubtitleComposer Team
*/

let s = subtitle.instance();
for(let i = 0, n = s.linesCount(); i < n; i++) {
	let line = s.line(i);
	let text = line.primaryText();
	line.setPrimaryText(text.left(1).toUpper().append(text.mid(1)));
}
