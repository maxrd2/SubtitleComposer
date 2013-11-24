#ifndef SUBVIEWER2INPUTFORMAT_H
#define SUBVIEWER2INPUTFORMAT_H

/***************************************************************************
 *   Copyright (C) 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../inputformat.h"

#include <QtCore/QRegExp>

namespace SubtitleComposer {
class SubViewer2InputFormat : public InputFormat
{
	friend class FormatManager;

public:
	virtual ~SubViewer2InputFormat() {}

protected:
	virtual bool parseSubtitles(Subtitle &subtitle, const QString &data) const
	{
		unsigned readLines = 0;

		for(int offset = 0; m_lineRegExp.indexIn(data, offset) != -1; offset = m_lineRegExp.pos() + m_lineRegExp.matchedLength()) {
			Time showTime(m_lineRegExp.cap(1).toInt(), m_lineRegExp.cap(2).toInt(), m_lineRegExp.cap(3).toInt(), m_lineRegExp.cap(4).toInt() * 10);

			Time hideTime(m_lineRegExp.cap(5).toInt(), m_lineRegExp.cap(6).toInt(), m_lineRegExp.cap(7).toInt(), m_lineRegExp.cap(8).toInt() * 10);

			int styleFlags = 0;
			QString text = m_lineRegExp.cap(9).replace("[br]", "\n").trimmed();
			if(m_styleRegExp.indexIn(text) != -1) {
				QString styleText(m_styleRegExp.cap(1));
				if(styleText.contains('b', Qt::CaseInsensitive))
					styleFlags |= SString::Bold;
				if(styleText.contains('i', Qt::CaseInsensitive))
					styleFlags |= SString::Italic;
				if(styleText.contains('u', Qt::CaseInsensitive))
					styleFlags |= SString::Underline;

				text.remove(m_styleRegExp);
			}

			subtitle.insertLine(new SubtitleLine(SString(text, styleFlags), showTime, hideTime));

			readLines++;
		}
		return readLines > 0;
	}

	SubViewer2InputFormat() :
		InputFormat("SubViewer 2.0", QStringList("sub")),
		m_lineRegExp("([0-2][0-9]):([0-5][0-9]):([0-5][0-9])\\.([0-9][0-9])," "([0-2][0-9]):([0-5][0-9]):([0-5][0-9])\\.([0-9][0-9])\n" "([^\n]*)\n\n", Qt::CaseInsensitive),
		m_styleRegExp("(\\{y:[ubi]+\\})", Qt::CaseInsensitive)
	{}

	mutable QRegExp m_lineRegExp;
	mutable QRegExp m_styleRegExp;
};
}

#endif
