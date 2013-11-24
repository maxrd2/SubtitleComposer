#ifndef TMPLAYERINPUTFORMAT_H
#define TMPLAYERINPUTFORMAT_H

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
// FIXME TMPlayer Multiline variant

class TMPlayerInputFormat : public InputFormat
{
	friend class FormatManager;

public:
	virtual ~TMPlayerInputFormat() {}

protected:
	virtual bool parseSubtitles(Subtitle &subtitle, const QString &data) const
	{
		unsigned readLines = 0;

		if(m_regExp.indexIn(data, 0) == -1)
			return false;

		Time previousShowTime(m_regExp.cap(1).toInt(), m_regExp.cap(2).toInt(), m_regExp.cap(3).toInt(), 0);
		QString previousText(m_regExp.cap(4).replace("|", "\n").trimmed());

		int offset = m_regExp.matchedLength();
		for(; m_regExp.indexIn(data, offset) != -1; offset += m_regExp.matchedLength()) {
			Time showTime(m_regExp.cap(1).toInt(), m_regExp.cap(2).toInt(), m_regExp.cap(3).toInt(), 0);
			QString text(m_regExp.cap(4).replace("|", "\n").trimmed());

			// To compensate for the format deficiencies, Subtitle Composer writes empty lines
			// indicating that way the line hide time. We do the same.
			if(!previousText.isEmpty()) {
				subtitle.insertLine(new SubtitleLine(previousText, previousShowTime, showTime));

				readLines++;
			}

			previousText = text;
			previousShowTime = showTime;
		}
		if(!previousText.isEmpty()) {
			subtitle.insertLine(new SubtitleLine(previousText, previousShowTime, previousShowTime + 2000));

			readLines++;
		}

		return true;
	}

	TMPlayerInputFormat() :
		InputFormat("TMPlayer", QString("sub:txt").split(":")),
		m_regExp("([0-2]?[0-9]):([0-5][0-9]):([0-5][0-9]):([^\n]*)\n?") {}

	TMPlayerInputFormat(const QString &name, const QStringList &extensions, const QString &regExp) :
		InputFormat(name, extensions),
		m_regExp(regExp) {}

	mutable QRegExp m_regExp;
};

class TMPlayerPlusInputFormat : public TMPlayerInputFormat
{
	friend class FormatManager;

protected:
	TMPlayerPlusInputFormat() :
		TMPlayerInputFormat("TMPlayer+", QString("sub:txt").split(":"), "([0-2]?[0-9]):([0-5][0-9]):([0-5][0-9])=([^\n]*)\n?")
	{}
};
}

#endif
