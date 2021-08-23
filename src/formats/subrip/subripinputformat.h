/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SUBRIPINPUTFORMAT_H
#define SUBRIPINPUTFORMAT_H

#include "core/richdocument.h"
#include "formats/inputformat.h"

#include <QRegExp>

namespace SubtitleComposer {
class SubRipInputFormat : public InputFormat
{
	friend class FormatManager;

protected:
	bool parseSubtitles(Subtitle &subtitle, const QString &data) const override
	{
		if(m_regExp.indexIn(data, 0) == -1)
			return false; // couldn't find first line

		unsigned readLines = 0;

		int offset = 0;
		do {
			Time showTime(m_regExp.cap(1).toInt(), m_regExp.cap(2).toInt(), m_regExp.cap(3).toInt(), m_regExp.cap(4).toInt());
			Time hideTime(m_regExp.cap(5).toInt(), m_regExp.cap(6).toInt(), m_regExp.cap(7).toInt(), m_regExp.cap(8).toInt());

			offset += m_regExp.matchedLength();

			QString text(data.mid(offset, m_regExp.indexIn(data, offset) - offset));

			offset += text.length();

			SString stext;
			stext.setRichString(text.trimmed());

			SubtitleLine *line = new SubtitleLine(showTime, hideTime);
			line->primaryDoc()->setRichText(stext, true);
			subtitle.insertLine(line);

			readLines++;
		} while(m_regExp.matchedLength() != -1);

		return readLines > 0;
	}

	SubRipInputFormat() :
		InputFormat(QStringLiteral("SubRip"), QStringList(QStringLiteral("srt"))),
		m_regExp(QStringLiteral("[\\d]+\n([0-2][0-9]):([0-5][0-9]):([0-5][0-9])[,\\.]([0-9]+) --> ([0-2][0-9]):([0-5][0-9]):([0-5][0-9])[,\\.]([0-9]+)\n"))
	{}

	mutable QRegExp m_regExp;
};
}

#endif
