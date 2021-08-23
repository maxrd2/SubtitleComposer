/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)

    SPDX-License-Identifier: GPL-2.0-or-later
*/

var s = subtitle.instance();
for(var i = s.linesCount() - 1; i >= 0; i--) {
	var line = s.line(i),
		text = line.richPrimaryText()
			.replace(/\s*\([^)]+\)\s*/, ' ')
			.replace(/\s*\[[^\]]+\]\s*/, ' ')
			.replace(/^(<.*>)?.*:\s*/, '$1')
			.replace(/(^\s*|\s*$)/, '');
	if(text.replace(/(<[^>]+>|\s)/, '') == '')
		s.removeLine(i);
	else
		line.setRichPrimaryText(text);
}
