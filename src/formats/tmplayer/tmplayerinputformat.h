/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TMPLAYERINPUTFORMAT_H
#define TMPLAYERINPUTFORMAT_H

#include "core/richtext/richdocument.h"
#include "helpers/common.h"
#include "formats/inputformat.h"

#include <QRegularExpression>

namespace SubtitleComposer {
// FIXME TMPlayer Multiline variant

class TMPlayerInputFormat : public InputFormat
{
	friend class FormatManager;

protected:
	TMPlayerInputFormat(const QString &name, const QStringList &extensions, const QString &re)
		: InputFormat(name, extensions),
		  m_reTime(re)
	{}

	TMPlayerInputFormat()
		: TMPlayerInputFormat($("TMPlayer"), QStringList() << $("sub") << $("txt"),
							  $("([0-2]?[0-9]):([0-5][0-9]):([0-5][0-9]):([^\n]*)\n?"))
	{}

	bool parseSubtitles(Subtitle &subtitle, const QString &data) const override
	{
		QRegularExpressionMatchIterator itTime = m_reTime.globalMatch(data);
		if(!itTime.hasNext())
			return false;

		do {
			QRegularExpressionMatch mTime = itTime.next();

			const QString text = mTime.captured(4).replace('|', '\n').trimmed();

			// To compensate for the format deficiencies, Subtitle Workshop writes empty lines
			// indicating that way the line hide time. We do the same.
			if(text.isEmpty())
				continue;

			const Time showTime(mTime.captured(1).toInt(), mTime.captured(2).toInt(), mTime.captured(3).toInt(), 0);
			Time hideTime;
			if(itTime.hasNext()) {
				mTime = itTime.peekNext();
				hideTime = Time(mTime.captured(1).toInt(), mTime.captured(2).toInt(), mTime.captured(3).toInt(), 0);
			} else {
				hideTime = showTime + 2000;
			}

			SubtitleLine *l = new SubtitleLine(showTime, hideTime);
			l->primaryDoc()->setPlainText(text);
			subtitle.insertLine(l);
		} while(itTime.hasNext());

		return subtitle.count() > 0;
	}

	QRegularExpression m_reTime;
};

class TMPlayerPlusInputFormat : public TMPlayerInputFormat
{
	friend class FormatManager;

protected:
	TMPlayerPlusInputFormat()
		: TMPlayerInputFormat($("TMPlayer+"), QStringList() << $("sub") << $("txt"),
							  $("([0-2]?[0-9]):([0-5][0-9]):([0-5][0-9])=([^\n]*)\n?"))
	{}
};
}

#endif
