/*
	SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
	SPDX-FileCopyrightText: 2021 Mladen Milinkovic <maxrd2@smoothware.net>

	SPDX-License-Identifier: GPL-2.0-or-later

	@name Remove Hearing Impaired Text
	@version 1.0
	@summary Remove hearing impaired text from subtitle lines.
	@author SubtitleComposer Team
*/

var s = subtitle.instance();
for(var i = s.linesCount() - 1; i >= 0; i--) {
	var line = s.line(i),
		text = line.richPrimaryText()
			.replace(/\s*\([^)]+\)\s*/, ' ')
			.replace(/\s*\[[^\]]+\]\s*/, ' ')
			.replace(/^(<.*>)?.*:\s*/, '$1')
			.replace(/(^\s*|\s*$)/, '');

	if(text.replace(/(<[^>]+>|\s)/, '') === '')
		s.removeLine(i);
	else
		line.setRichPrimaryText(text);
}
