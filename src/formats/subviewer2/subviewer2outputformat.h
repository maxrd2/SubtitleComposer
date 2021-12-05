/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SUBVIEWER2OUTPUTFORMAT_H
#define SUBVIEWER2OUTPUTFORMAT_H

#include "formats/outputformat.h"
#include "core/richtext/richdocument.h"
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
			ret += QString::asprintf("%02d:%02d:%02d.%02d,%02d:%02d:%02d.%02d\n", showTime.hours(), showTime.minutes(), showTime.seconds(), (showTime.millis() + 5) / 10, hideTime.hours(), hideTime.minutes(), hideTime.seconds(), (hideTime.millis() + 5) / 10);

			const RichString text = (primary ? line->primaryDoc() : line->secondaryDoc())->toRichText();
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
		m_stylesMap[RichString::Bold] = QStringLiteral("{Y:b}");
		m_stylesMap[RichString::Italic] = QStringLiteral("{Y:i}");
		m_stylesMap[RichString::Underline] = QStringLiteral("{Y:u}");
		m_stylesMap[RichString::Bold | RichString::Italic] = QStringLiteral("{Y:bi}");
		m_stylesMap[RichString::Bold | RichString::Underline] = QStringLiteral("{Y:ub}");
		m_stylesMap[RichString::Italic | RichString::Underline] = QStringLiteral("{Y:ui}");
		m_stylesMap[RichString::Bold | RichString::Italic | RichString::Underline] = QStringLiteral("{Y:ubi}");
	}

	mutable QMap<int, QString> m_stylesMap;
};
}

#endif
