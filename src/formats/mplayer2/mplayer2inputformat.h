/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MPLAYER2INPUTFORMAT_H
#define MPLAYER2INPUTFORMAT_H

#include "core/richtext/richdocument.h"
#include "helpers/common.h"
#include "formats/inputformat.h"

#include <QRegularExpression>

namespace SubtitleComposer {
class MPlayer2InputFormat : public InputFormat
{
	friend class FormatManager;

protected:
	MPlayer2InputFormat()
		: InputFormat($("MPlayer2"), QStringList($("mpl")))
	{}

	bool parseSubtitles(Subtitle &subtitle, const QString &data) const override
	{
		staticRE$(lineRE, "\\[(\\d+)\\]\\[(\\d+)\\]([^\n]+)\n", REu | REi);

		QRegularExpressionMatchIterator itLine = lineRE.globalMatch(data);
		if(!itLine.hasNext())
			return false;

		do {
			const QRegularExpressionMatch mLine = itLine.next();
			const Time showTime(mLine.captured(1).toInt() * 100);
			const Time hideTime(mLine.captured(2).toInt() * 100);
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
