/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef YOUTUBECAPTIONSOUTPUTFORMAT_H
#define YOUTUBECAPTIONSOUTPUTFORMAT_H

#include "core/richtext/richdocument.h"
#include "core/subtitleiterator.h"
#include "formats/outputformat.h"
#include "helpers/common.h"

namespace SubtitleComposer {
class YouTubeCaptionsOutputFormat : public OutputFormat
{
	friend class FormatManager;

protected:
	QString dumpSubtitles(const Subtitle &subtitle, bool primary) const override
	{
		QString ret;

		for(SubtitleIterator it(subtitle); it.current(); ++it) {
			const SubtitleLine *ln = it.current();
			const Time ts = ln->showTime();
			const Time th = ln->hideTime();
			ret += QString::asprintf("%d:%02d:%02d.%03d,%d:%02d:%02d.%03d\n",
				ts.hours(), ts.minutes(), ts.seconds(), ts.millis(),
				th.hours(), th.minutes(), th.seconds(), th.millis());

			const RichString text = (primary ? ln->primaryDoc() : ln->secondaryDoc())->toRichText();

			// TODO does the format actually supports styled text?
			// if so, does it use standard HTML style tags?
			ret += text.richString();

			ret += $("\n\n");
		}
		return ret;
	}

	YouTubeCaptionsOutputFormat()
		: OutputFormat($("YouTube Captions"), QStringList($("sbv")))
	{}
};
}

#endif
