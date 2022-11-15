/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SUBRIPINPUTFORMAT_H
#define SUBRIPINPUTFORMAT_H

#include "core/richtext/richdocument.h"
#include "helpers/common.h"
#include "formats/inputformat.h"

#include <QRegularExpression>

namespace SubtitleComposer {
class SubRipInputFormat : public InputFormat
{
	friend class FormatManager;

protected:
	SubRipInputFormat()
		: InputFormat($("SubRip"), QStringList($("srt")))

	{}

	bool parseSubtitles(Subtitle &subtitle, const QString &data) const override
	{
		staticRE$(reTime, "[\\d]+\n([0-2][0-9]):([0-5][0-9]):([0-5][0-9])[,\\.]([0-9]+) --> ([0-2][0-9]):([0-5][0-9]):([0-5][0-9])[,\\.]([0-9]+)\n", REu);

		QRegularExpressionMatchIterator itTime = reTime.globalMatch(data);
		if(!itTime.hasNext())
			return false;

		do {
			QRegularExpressionMatch mTime = itTime.next();

			Time showTime(mTime.captured(1).toInt(), mTime.captured(2).toInt(), mTime.captured(3).toInt(), mTime.captured(4).toInt());
			Time hideTime(mTime.captured(5).toInt(), mTime.captured(6).toInt(), mTime.captured(7).toInt(), mTime.captured(8).toInt());

			const int off = mTime.capturedEnd();
			const QString text = data.mid(off, itTime.hasNext() ? itTime.peekNext().capturedStart() - off : -1).trimmed();

			RichString stext;
			stext.setRichString(text);

			SubtitleLine *line = new SubtitleLine(showTime, hideTime);
			line->primaryDoc()->setRichText(stext, true);
			subtitle.insertLine(line);
		} while(itTime.hasNext());

		return true;
	}
};
}

#endif
