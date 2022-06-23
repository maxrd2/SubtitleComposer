/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SUBVIEWER2INPUTFORMAT_H
#define SUBVIEWER2INPUTFORMAT_H

#include "core/richdocument.h"
#include "formats/inputformat.h"

#include <QRegExp>

namespace SubtitleComposer {
class SubViewer2InputFormat : public InputFormat
{
	friend class FormatManager;

protected:
	bool parseSubtitles(Subtitle &subtitle, const QString &data) const override
	{
		unsigned readLines = 0;

		for(int offset = 0; m_lineRegExp.indexIn(data, offset) != -1; offset = m_lineRegExp.pos() + m_lineRegExp.matchedLength()) {
			Time showTime(m_lineRegExp.cap(1).toInt(), m_lineRegExp.cap(2).toInt(), m_lineRegExp.cap(3).toInt(), m_lineRegExp.cap(4).toInt() * 10);

			Time hideTime(m_lineRegExp.cap(5).toInt(), m_lineRegExp.cap(6).toInt(), m_lineRegExp.cap(7).toInt(), m_lineRegExp.cap(8).toInt() * 10);

			int styleFlags = 0;
			QString text = m_lineRegExp.cap(9).replace(QLatin1String("[br]"), QLatin1String("\n")).trimmed();
			if(m_styleRegExp.indexIn(text) != -1) {
				QString styleText(m_styleRegExp.cap(1));
				if(styleText.contains('b', Qt::CaseInsensitive))
					styleFlags |= RichString::Bold;
				if(styleText.contains('i', Qt::CaseInsensitive))
					styleFlags |= RichString::Italic;
				if(styleText.contains('u', Qt::CaseInsensitive))
					styleFlags |= RichString::Underline;

				text.remove(m_styleRegExp);
			}

			SubtitleLine *l = new SubtitleLine(showTime, hideTime);
			l->primaryDoc()->setRichText(RichString(text, styleFlags), true);
			subtitle.insertLine(l);

			readLines++;
		}
		return readLines > 0;
	}

	SubViewer2InputFormat() :
		InputFormat(QStringLiteral("SubViewer 2.0"), QStringList(QStringLiteral("sub"))),
		m_lineRegExp(QStringLiteral("([0-2][0-9]):([0-5][0-9]):([0-5][0-9])\\.([0-9][0-9])," "([0-2][0-9]):([0-5][0-9]):([0-5][0-9])\\.([0-9][0-9])\n" "([^\n]*)\n\n"), Qt::CaseInsensitive),
		m_styleRegExp(QStringLiteral("(\\{y:[ubi]+\\})"), Qt::CaseInsensitive)
	{}

	mutable QRegExp m_lineRegExp;
	mutable QRegExp m_styleRegExp;
};
}

#endif
