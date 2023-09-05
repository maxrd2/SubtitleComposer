/*
	SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
	SPDX-FileCopyrightText: 2021-2023 Mladen Milinkovic <maxrd2@smoothware.net>

	SPDX-License-Identifier: GPL-2.0-or-later

	@name Remove Hearing Impaired Text
	@version 1.0
	@summary Remove hearing impaired text from subtitle lines.
	@author SubtitleComposer Team
*/

const s = subtitle.instance();
for(let i = s.linesCount() - 1; i >= 0; i--) {
	let line = s.line(i),
		text = line.richPrimaryText()
			.replace(/\s*\([^)]+\)\s*/g, ' ')
			.replace(/\s*\[[^\]]+\]\s*/g, ' ')
			.replace(/^(<.*>)?.*:\s+/gm, '$1')
			.replace(/(^\s*|\s*$)/gm, '');

	if(text.replace(/(<[^>]+>|\s)/g, '') === '')
		s.removeLine(i);
	else
		line.setRichPrimaryText(text);
}
