/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MPLAYERINPUTFORMAT_H
#define MPLAYERINPUTFORMAT_H

#include "core/richtext/richdocument.h"
#include "helpers/common.h"
#include "formats/inputformat.h"

#include <QRegularExpression>

namespace SubtitleComposer {
class MPlayerInputFormat : public InputFormat
{
	friend class FormatManager;

protected:
	MPlayerInputFormat()
		: InputFormat($("MPlayer"), QStringList($("mpl")))
	{}

	bool parseSubtitles(Subtitle &subtitle, const QString &data) const override
	{
		staticRE$(lineRE, "(^|\n)(\\d+),(\\d+),0,([^\n]+)[^\n]", REu | REi);

		const double fps = subtitle.framesPerSecond();

		QRegularExpressionMatchIterator itLine = lineRE.globalMatch(data);
		if(!itLine.hasNext())
			return false;

		do {
			const QRegularExpressionMatch mLine = itLine.next();
			const Time showTime(static_cast<long>((mLine.captured(1).toLong() / fps) * 1000));
			const Time hideTime(static_cast<long>((mLine.captured(2).toLong() / fps) * 1000));
			const QString text = mLine.captured(3).replace(QChar('|'), QChar('\n'));

			SubtitleLine *line = new SubtitleLine(showTime, hideTime);
			line->primaryDoc()->setPlainText(text);
			subtitle.insertLine(line);
		} while(itLine.hasNext());

		return true;
	}
};
}

#endif
