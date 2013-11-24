#ifndef YOUTUBECAPTIONSINPUTFORMAT_H
#define YOUTUBECAPTIONSINPUTFORMAT_H

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
class YouTubeCaptionsInputFormat : public InputFormat
{
	friend class FormatManager;

public:
	virtual ~YouTubeCaptionsInputFormat() {}

protected:
	virtual bool parseSubtitles(Subtitle &subtitle, const QString &data) const
	{
		if(m_regExp.indexIn(data, 0) == -1)
			return false; // couldn't find first line

		unsigned readLines = 0;

		int offset = 0;
		do {
			Time showTime(m_regExp.cap(1).toInt(), m_regExp.cap(2).toInt(), m_regExp.cap(3).toInt(), m_regExp.cap(4).toInt());

			Time hideTime(m_regExp.cap(5).toInt(), m_regExp.cap(6).toInt(), m_regExp.cap(7).toInt(), m_regExp.cap(8).toInt());

			offset += m_regExp.matchedLength();

			QString text(data.mid(offset, (unsigned)m_regExp.indexIn(data, offset) - offset));

			offset += text.length();

			// TODO does the format actually supports styled text?
			// if so, does it use standard HTML style tags?
			SString stext;
			stext.setRichString(text.trimmed());

			subtitle.insertLine(new SubtitleLine(stext, showTime, hideTime));

			readLines++;
		} while(m_regExp.matchedLength() != -1);

		return readLines > 0;
	}

	YouTubeCaptionsInputFormat() :
		InputFormat("YouTube Captions", QStringList("sbv")),
		m_regExp("[\\d]+\n([0-2][0-9]):([0-5][0-9]):([0-5][0-9])[,\\.]([0-9][0-9][0-9]),([0-2][0-9]):([0-5][0-9]):([0-5][0-9])[,\\.]([0-9][0-9][0-9])\n")
	{}

	mutable QRegExp m_regExp;
};
}

#endif
