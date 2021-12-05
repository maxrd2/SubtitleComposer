/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MPLAYEROUTPUTFORMAT_H
#define MPLAYEROUTPUTFORMAT_H

#include "formats/outputformat.h"
#include "core/richtext/richdocument.h"
#include "core/subtitleiterator.h"

namespace SubtitleComposer {
class MPlayerOutputFormat : public OutputFormat
{
	friend class FormatManager;

protected:
	QString dumpSubtitles(const Subtitle &subtitle, bool primary) const override
	{
		QString ret;

		double framesPerSecond = subtitle.framesPerSecond();

		for(SubtitleIterator it(subtitle); it.current(); ++it) {
			const SubtitleLine *line = it.current();

			QString text = (primary ? line->primaryDoc() : line->secondaryDoc())->toPlainText();

			ret += m_lineBuilder.arg(static_cast<long>((line->showTime().toMillis() / 1000.0) * framesPerSecond + 0.5))
					.arg(static_cast<long>((line->hideTime().toMillis() / 1000.0) * framesPerSecond + 0.5))
					.arg(text.replace('\n', '|'));
		}
		return ret;
	}

	MPlayerOutputFormat() :
		OutputFormat(QStringLiteral("MPlayer"), QStringList(QStringLiteral("mpl"))),
		m_lineBuilder(QStringLiteral("%1,%2,0,%3\n"))
	{}

	const QString m_lineBuilder;
};
}

#endif
