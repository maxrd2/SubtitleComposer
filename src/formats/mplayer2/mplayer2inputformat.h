/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef MPLAYER2INPUTFORMAT_H
#define MPLAYER2INPUTFORMAT_H

#include "core/richdocument.h"
#include "formats/inputformat.h"

#include <QRegExp>

namespace SubtitleComposer {
class MPlayer2InputFormat : public InputFormat
{
	friend class FormatManager;

protected:
	bool parseSubtitles(Subtitle &subtitle, const QString &data) const override
	{
		unsigned readLines = 0;

		for(int offset = 0; m_lineRegExp.indexIn(data, offset) != -1; offset += m_lineRegExp.matchedLength()) {
			Time showTime(m_lineRegExp.cap(1).toInt() * 100);
			Time hideTime(m_lineRegExp.cap(2).toInt() * 100);
			QString text(m_lineRegExp.cap(3).replace('|', '\n'));

			SubtitleLine *line = new SubtitleLine(showTime, hideTime);
			line->primaryDoc()->setPlainText(text);
			subtitle.insertLine(line);

			readLines++;
		}
		return readLines > 0;
	}

	MPlayer2InputFormat() :
		InputFormat(QStringLiteral("MPlayer2"), QStringList(QStringLiteral("mpl"))),
		m_lineRegExp(QStringLiteral("\\[(\\d+)\\]\\[(\\d+)\\]([^\n]+)\n"))
	{}

	mutable QRegExp m_lineRegExp;
};
}

#endif
