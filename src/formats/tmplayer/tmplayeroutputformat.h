/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TMPLAYEROUTPUTFORMAT_H
#define TMPLAYEROUTPUTFORMAT_H

#include "formats/outputformat.h"
#include "core/richtext/richdocument.h"
#include "core/subtitleiterator.h"

namespace SubtitleComposer {
class TMPlayerOutputFormat : public OutputFormat
{
	friend class FormatManager;

protected:
	QString dumpSubtitles(const Subtitle &subtitle, bool primary) const override
	{
		QString ret;

		for(SubtitleIterator it(subtitle); it.current(); ++it) {
			const SubtitleLine *line = it.current();

			Time showTime = line->showTime();
			ret += QString::asprintf(m_timeFormat, showTime.hours(), showTime.minutes(), showTime.seconds());

			QString text = (primary ? line->primaryDoc() : line->secondaryDoc())->toPlainText();
			ret += text.replace('\n', '|');
			ret += '\n';

			// We behave like Subtitle Workshop here: to compensate for the lack of hide time
			// indication provisions in the format we add an empty line with the hide time.
			Time hideTime = line->hideTime();
			ret += QString::asprintf(m_timeFormat, hideTime.hours(), hideTime.minutes(), hideTime.seconds());

			ret += '\n';
		}
		return ret;
	}

	TMPlayerOutputFormat() :
		OutputFormat(QStringLiteral("TMPlayer"), QStringList() << QStringLiteral("sub") << QStringLiteral("txt")),
		m_timeFormat("%02d:%02d:%02d:")
	{}

	TMPlayerOutputFormat(const QString &name, const QStringList &extensions, const char *timeFormat) :
		OutputFormat(name, extensions),
		m_timeFormat(timeFormat)
	{}

	const char *m_timeFormat;
};

class TMPlayerPlusOutputFormat : public TMPlayerOutputFormat
{
	friend class FormatManager;

protected:
	TMPlayerPlusOutputFormat() :
		TMPlayerOutputFormat(QStringLiteral("TMPlayer+"), QStringList() << QStringLiteral("sub") << QStringLiteral("txt"), "%02d:%02d:%02d=")
	{}
};
}

#endif
