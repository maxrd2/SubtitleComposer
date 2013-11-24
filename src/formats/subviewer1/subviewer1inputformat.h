#ifndef SUBVIEWER1INPUTFORMAT_H
#define SUBVIEWER1INPUTFORMAT_H

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
class SubViewer1InputFormat : public InputFormat
{
	friend class FormatManager;

public:
	virtual ~SubViewer1InputFormat() {}

protected:
	virtual bool parseSubtitles(Subtitle &subtitle, const QString &data) const
	{
		if(m_regExp.indexIn(data, 0) == -1)
			return false; // couldn't find first line

		unsigned readLines = 0;

		int offset = m_regExp.pos();
		do {
			offset += m_regExp.matchedLength();

			Time showTime(m_regExp.cap(1).toInt(), m_regExp.cap(2).toInt(), m_regExp.cap(3).toInt(), 0);

			QString text(m_regExp.cap(4).replace("|", "\n").trimmed());

			// search hideTime
			if(m_regExp.indexIn(data, offset) == -1)
				break;

			Time hideTime(m_regExp.cap(1).toInt(), m_regExp.cap(2).toInt(), m_regExp.cap(3).toInt(), 0);

			subtitle.insertLine(new SubtitleLine(text, showTime, hideTime));

			offset += m_regExp.matchedLength();

			readLines++;
		} while(m_regExp.indexIn(data, offset) != -1);          // search next line's showTime

		return readLines > 0;
	}

	SubViewer1InputFormat() :
		InputFormat("SubViewer 1.0", QStringList("sub")),
		m_regExp("\\[([0-2][0-9]):([0-5][0-9]):([0-5][0-9])\\]\n([^\n]*)\n")
	{}

	mutable QRegExp m_regExp;
};
}

#endif
