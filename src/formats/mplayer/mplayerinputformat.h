/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MPLAYERINPUTFORMAT_H
#define MPLAYERINPUTFORMAT_H

#include "core/richdocument.h"
#include "formats/inputformat.h"

#include <QRegExp>

namespace SubtitleComposer {
class MPlayerInputFormat : public InputFormat
{
	friend class FormatManager;

protected:
	bool parseSubtitles(Subtitle &subtitle, const QString &data) const override
	{
		double framesPerSecond = subtitle.framesPerSecond();

		unsigned readLines = 0;

		for(int offset = 0; m_lineRegExp.indexIn(data, offset) != -1; offset += m_lineRegExp.matchedLength()) {
			Time showTime(static_cast<long>((m_lineRegExp.cap(1).toLong() / framesPerSecond) * 1000));
			Time hideTime(static_cast<long>((m_lineRegExp.cap(2).toLong() / framesPerSecond) * 1000));
			QString text(m_lineRegExp.cap(3).replace("|", "\n"));

			SubtitleLine *line = new SubtitleLine(showTime, hideTime);
			line->primaryDoc()->setPlainText(text);
			subtitle.insertLine(line);

			readLines++;
		}
		return readLines > 0;
	}

	MPlayerInputFormat() :
		InputFormat(QStringLiteral("MPlayer"), QStringList(QStringLiteral("mpl"))),
		m_lineRegExp(QStringLiteral("(^|\n)(\\d+),(\\d+),0,([^\n]+)[^\n]"))
	{}

	mutable QRegExp m_lineRegExp;
};
}

#endif
