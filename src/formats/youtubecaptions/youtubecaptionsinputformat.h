/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef YOUTUBECAPTIONSINPUTFORMAT_H
#define YOUTUBECAPTIONSINPUTFORMAT_H

#include "core/richtext/richdocument.h"
#include "helpers/common.h"
#include "formats/inputformat.h"

#include <QRegularExpression>

namespace SubtitleComposer {
class YouTubeCaptionsInputFormat : public InputFormat
{
	friend class FormatManager;

protected:
	YouTubeCaptionsInputFormat()
		: InputFormat($("YouTube Captions"), QStringList($("sbv")))
	{}

	bool parseSubtitles(Subtitle &subtitle, const QString &data) const override
	{
		staticRE$(reTime,
			"([\\d]+\\n)?"
			"(\\d+):([0-5][0-9]):([0-5][0-9])[,\\.]([0-9][0-9][0-9]),"
			"(\\d+):([0-5][0-9]):([0-5][0-9])[,\\.]([0-9][0-9][0-9])\\n");
		QRegularExpressionMatchIterator it = reTime.globalMatch(data);
		if(!it.hasNext())
			return false;

		do {
			QRegularExpressionMatch tm = it.next();
			const Time showTime(tm.captured(2).toInt(), tm.captured(3).toInt(), tm.captured(4).toInt(), tm.captured(5).toInt());
			const Time hideTime(tm.captured(6).toInt(), tm.captured(7).toInt(), tm.captured(8).toInt(), tm.captured(9).toInt());

			const int off = tm.capturedEnd();
			const QString text = data.mid(off, it.hasNext() ? it.peekNext().capturedStart() - off : -1).trimmed();

			// TODO does the format actually support styled text?
			// if so, does it use standard HTML style tags?

			SubtitleLine *l = new SubtitleLine(showTime, hideTime);
			l->primaryDoc()->setHtml(text, true);
			subtitle.insertLine(l);
		} while(it.hasNext());

		return subtitle.count() > 0;
	}
};
}

#endif
