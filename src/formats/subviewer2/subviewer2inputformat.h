/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SUBVIEWER2INPUTFORMAT_H
#define SUBVIEWER2INPUTFORMAT_H

#include "core/richtext/richdocument.h"
#include "helpers/common.h"
#include "formats/inputformat.h"

#include <QRegularExpression>

namespace SubtitleComposer {
class SubViewer2InputFormat : public InputFormat
{
	friend class FormatManager;

protected:
	SubViewer2InputFormat()
		: InputFormat($("SubViewer 2.0"), QStringList($("sub")))
	{}

	bool parseSubtitles(Subtitle &subtitle, const QString &data) const override
	{
		staticRE$(reLine,
				"([0-2][0-9]):([0-5][0-9]):([0-5][0-9])\\.([0-9][0-9]),"
				"([0-2][0-9]):([0-5][0-9]):([0-5][0-9])\\.([0-9][0-9])\\n"
				"([^\\n]*)\\n\\n", REu | REi);
		QRegularExpressionMatchIterator itLine = reLine.globalMatch(data);
		if(!itLine.hasNext())
			return false;

		do {
			QRegularExpressionMatch mLine = itLine.next();

			const Time showTime(mLine.captured(1).toInt(), mLine.captured(2).toInt(), mLine.captured(3).toInt(), mLine.captured(4).toInt() * 10);
			const Time hideTime(mLine.captured(5).toInt(), mLine.captured(6).toInt(), mLine.captured(7).toInt(), mLine.captured(8).toInt() * 10);

			const QString text = mLine.captured(9).replace(QLatin1String("[br]"), QLatin1String("\n")).trimmed();
			int styleFlags = 0;

			staticRE$(reStyle, "(\\{y:[ubi]+\\})", REu | REi);
			QRegularExpressionMatchIterator itStyle = reStyle.globalMatch(text);
			if(itStyle.hasNext()) {
				// TODO: seems styles are affecting the whole line... is that true?
				const QString styleText = itStyle.next().captured(1);
				if(styleText.contains('b', Qt::CaseInsensitive))
					styleFlags |= RichString::Bold;
				if(styleText.contains('i', Qt::CaseInsensitive))
					styleFlags |= RichString::Italic;
				if(styleText.contains('u', Qt::CaseInsensitive))
					styleFlags |= RichString::Underline;
			}

			SubtitleLine *l = new SubtitleLine(showTime, hideTime);
			l->primaryDoc()->setRichText(RichString(text, styleFlags), true);
			subtitle.insertLine(l);

		} while(itLine.hasNext());

		return subtitle.count() > 0;
	}
};
}

#endif
