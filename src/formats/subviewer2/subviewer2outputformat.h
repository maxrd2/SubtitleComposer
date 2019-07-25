#ifndef SUBVIEWER2OUTPUTFORMAT_H
#define SUBVIEWER2OUTPUTFORMAT_H

/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2019 Mladen Milinkovic <max@smoothware.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "formats/outputformat.h"
#include "core/subtitleiterator.h"

namespace SubtitleComposer {
class SubViewer2OutputFormat : public OutputFormat
{
	friend class FormatManager;

protected:
	QString dumpSubtitles(const Subtitle &subtitle, bool primary) const override
	{
		QString ret(QStringLiteral("[INFORMATION]\n[TITLE]\n[AUTHOR]\n[SOURCE]\n[PRG]\n[FILEPATH]\n[DELAY]0\n[CD TRACK]0\n" "[COMMENT]\n[END INFORMATION]\n[SUBTITLE]\n[COLF]&HFFFFFF,[STYLE]bd,[SIZE]24,[FONT]Tahoma\n"));

		for(SubtitleIterator it(subtitle); it.current(); ++it) {
			const SubtitleLine *line = it.current();

			Time showTime = line->showTime();
			Time hideTime = line->hideTime();
			ret += m_builder.sprintf("%02d:%02d:%02d.%02d,%02d:%02d:%02d.%02d\n", showTime.hours(), showTime.minutes(), showTime.seconds(), (showTime.mseconds() + 5) / 10, hideTime.hours(), hideTime.minutes(), hideTime.seconds(), (hideTime.mseconds() + 5) / 10);

			const SString &text = primary ? line->primaryText() : line->secondaryText();
			ret += m_stylesMap[text.cummulativeStyleFlags()];
			ret += text.string().replace("\n", "[br]");

			ret += QStringLiteral("\n\n");
		}
		return ret;
	}

	SubViewer2OutputFormat() :
		OutputFormat(QStringLiteral("SubViewer 2.0"), QStringList(QStringLiteral("sub"))),
		m_stylesMap()
	{
		m_stylesMap[SString::Bold] = QStringLiteral("{Y:b}");
		m_stylesMap[SString::Italic] = QStringLiteral("{Y:i}");
		m_stylesMap[SString::Underline] = QStringLiteral("{Y:u}");
		m_stylesMap[SString::Bold | SString::Italic] = QStringLiteral("{Y:bi}");
		m_stylesMap[SString::Bold | SString::Underline] = QStringLiteral("{Y:ub}");
		m_stylesMap[SString::Italic | SString::Underline] = QStringLiteral("{Y:ui}");
		m_stylesMap[SString::Bold | SString::Italic | SString::Underline] = QStringLiteral("{Y:ubi}");
	}

	mutable QString m_builder;
	mutable QMap<int, QString> m_stylesMap;
};
}

#endif
