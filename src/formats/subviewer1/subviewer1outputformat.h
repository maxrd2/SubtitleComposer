/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SUBVIEWER1OUTPUTFORMAT_H
#define SUBVIEWER1OUTPUTFORMAT_H

#include "formats/outputformat.h"
#include "core/richdocument.h"
#include "core/subtitleiterator.h"

namespace SubtitleComposer {
class SubViewer1OutputFormat : public OutputFormat
{
	friend class FormatManager;

protected:
	QString dumpSubtitles(const Subtitle &subtitle, bool primary) const override
	{
		QString ret(QStringLiteral("[TITLE]\n\n[AUTHOR]\n\n[SOURCE]\n\n[PRG]\n\n[FILEPATH]\n\n[DELAY]\n0\n[CD TRACK]\n0\n[BEGIN]\n" "******** START SCRIPT ********\n"));

		for(SubtitleIterator it(subtitle); it.current(); ++it) {
			const SubtitleLine *line = it.current();

			Time showTime = line->showTime();
			ret += QString::asprintf("[%02d:%02d:%02d]\n", showTime.hours(), showTime.minutes(), showTime.seconds());

			QString text = (primary ? line->primaryDoc() : line->secondaryDoc())->toPlainText();
			ret += text.replace('\n', '|');

			Time hideTime = line->hideTime();
			ret += QString::asprintf("\n[%02d:%02d:%02d]\n\n", hideTime.hours(), hideTime.minutes(), hideTime.seconds());
		}
		ret += "[END]\n" "******** END SCRIPT ********\n";

		return ret;
	}

	SubViewer1OutputFormat() :
		OutputFormat(QStringLiteral("SubViewer 1.0"), QStringList(QStringLiteral("sub")))
	{}
};
}

#endif
