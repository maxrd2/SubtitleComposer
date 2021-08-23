/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SUBVIEWER1INPUTFORMAT_H
#define SUBVIEWER1INPUTFORMAT_H

#include "core/richdocument.h"
#include "formats/inputformat.h"

#include <QRegExp>

namespace SubtitleComposer {
class SubViewer1InputFormat : public InputFormat
{
	friend class FormatManager;

protected:
	bool parseSubtitles(Subtitle &subtitle, const QString &data) const override
	{
		if(m_regExp.indexIn(data, 0) == -1)
			return false; // couldn't find first line

		unsigned readLines = 0;

		int offset = m_regExp.pos();
		do {
			offset += m_regExp.matchedLength();

			Time showTime(m_regExp.cap(1).toInt(), m_regExp.cap(2).toInt(), m_regExp.cap(3).toInt(), 0);

			QString text(m_regExp.cap(4).replace('|', '\n').trimmed());

			// search hideTime
			if(m_regExp.indexIn(data, offset) == -1)
				break;

			Time hideTime(m_regExp.cap(1).toInt(), m_regExp.cap(2).toInt(), m_regExp.cap(3).toInt(), 0);

			SubtitleLine *l = new SubtitleLine(showTime, hideTime);
			l->primaryDoc()->setPlainText(text);
			subtitle.insertLine(l);

			offset += m_regExp.matchedLength();

			readLines++;
		} while(m_regExp.indexIn(data, offset) != -1); // search next line's showTime

		return readLines > 0;
	}

	SubViewer1InputFormat() :
		InputFormat(QStringLiteral("SubViewer 1.0"), QStringList(QStringLiteral("sub"))),
		m_regExp(QStringLiteral("\\[([0-2][0-9]):([0-5][0-9]):([0-5][0-9])\\]\n([^\n]*)\n"))
	{}

	mutable QRegExp m_regExp;
};
}

#endif
