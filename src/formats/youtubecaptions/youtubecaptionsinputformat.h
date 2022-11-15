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
		staticRE$(reTime, "[\\d]+\n([0-2][0-9]):([0-5][0-9]):([0-5][0-9])[,\\.]([0-9][0-9][0-9]),([0-2][0-9]):([0-5][0-9]):([0-5][0-9])[,\\.]([0-9][0-9][0-9])\n");
		QRegularExpressionMatchIterator itTime = reTime.globalMatch(data);
		if(!itTime.hasNext())
			return false;

		do {
			QRegularExpressionMatch mTime = itTime.next();
			const Time showTime(mTime.captured(1).toInt(), mTime.captured(2).toInt(), mTime.captured(3).toInt(), mTime.captured(4).toInt());
			const Time hideTime(mTime.captured(5).toInt(), mTime.captured(6).toInt(), mTime.captured(7).toInt(), mTime.captured(8).toInt());

			const int off = mTime.capturedEnd();
			const QString text = data.mid(off, itTime.hasNext() ? itTime.peekNext().capturedStart() - off : -1).trimmed();

			// TODO does the format actually support styled text?
			// if so, does it use standard HTML style tags?

			SubtitleLine *l = new SubtitleLine(showTime, hideTime);
			l->primaryDoc()->setHtml(text, true);
			subtitle.insertLine(l);
		} while(itTime.hasNext());

		return subtitle.count() > 0;
	}
};
}

#endif
