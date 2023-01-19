/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SUBVIEWER1INPUTFORMAT_H
#define SUBVIEWER1INPUTFORMAT_H

#include "core/richtext/richdocument.h"
#include "helpers/common.h"
#include "formats/inputformat.h"

#include <QRegularExpression>

namespace SubtitleComposer {
class SubViewer1InputFormat : public InputFormat
{
	friend class FormatManager;

protected:
	SubViewer1InputFormat()
		: InputFormat($("SubViewer 1.0"), QStringList($("sub")))
	{}

	bool parseSubtitles(Subtitle &subtitle, const QString &data) const override
	{
		staticRE$(reTime, "\\[([0-2][0-9]):([0-5][0-9]):([0-5][0-9])\\]\\n([^\n]*)\\n", REu);
		QRegularExpressionMatchIterator itTime = reTime.globalMatch(data);
		if(!itTime.hasNext())
			return false;

		for(;;) {
			QRegularExpressionMatch mTime = itTime.next();

			const Time showTime(mTime.captured(1).toInt(), mTime.captured(2).toInt(), mTime.captured(3).toInt(), 0);
			const QString text(mTime.captured(4).replace('|', '\n').trimmed());

			if(!itTime.hasNext())
				break;
			mTime = itTime.peekNext();

			const Time hideTime(mTime.captured(1).toInt(), mTime.captured(2).toInt(), mTime.captured(3).toInt(), 0);

			SubtitleLine *l = new SubtitleLine(showTime, hideTime);
			l->primaryDoc()->setPlainText(text);
			subtitle.insertLine(l);

		}

		return subtitle.count() > 0;
	}
};
}

#endif
