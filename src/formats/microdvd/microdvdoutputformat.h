#ifndef MICRODVDOUTPUTFORMAT_H
#define MICRODVDOUTPUTFORMAT_H

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

#include "../outputformat.h"
#include "../../core/subtitleiterator.h"

namespace SubtitleComposer {
class MicroDVDOutputFormat : public OutputFormat
{
	friend class FormatManager;

public:
	virtual ~MicroDVDOutputFormat() {}

protected:

	virtual QString dumpSubtitles(const Subtitle &subtitle, bool primary) const
	{
		QString ret;

		double framesPerSecond = subtitle.framesPerSecond();
		ret += m_lineBuilder
				.arg(1)
				.arg(1)
				.arg(QString::number(framesPerSecond, 'f', 3));

		for(SubtitleIterator it(subtitle); it.current(); ++it) {
			const SubtitleLine *line = it.current();

			const SString &text = primary ? line->primaryText() : line->secondaryText();
			QString subtitle = "";

			int prevStyle = 0;
			QRgb prevColor = 0;
			for(int i = 0, sz = text.length(); i < sz; i++) {
				int curStyle = text.styleFlagsAt(i);
				QRgb curColor = (curStyle &SString::Color) != 0 ? text.styleColorAt(i) : 0;
				curStyle &= SString::Bold | SString::Italic | SString::Underline;
				if(prevStyle != curStyle)
					subtitle += m_stylesMap[curStyle];
				if(prevColor != curColor)
					subtitle += "{c:" + (curColor != 0 ? "$" + QColor(qBlue(curColor), qGreen(curColor), qRed(curColor)).name().mid(1).toLower() : "") + "}";

				const QChar ch = text.at(i);
				if(ch == '\n' || ch == '\r')
					subtitle += '|';
				else
					subtitle += ch;

				prevStyle = curStyle;
				prevColor = curColor;
			}

			ret += m_lineBuilder
					.arg((long)((line->showTime().toMillis() / 1000.0) * framesPerSecond + 0.5))
					.arg((long)((line->hideTime().toMillis() / 1000.0) * framesPerSecond + 0.5))
					.arg(subtitle);
		}
		return ret;
	}

	MicroDVDOutputFormat() :
		OutputFormat("MicroDVD", QString("sub:txt").split(":")),
		m_lineBuilder("{%1}{%2}%3\n")
	{
		m_stylesMap[0] = "{y:}";
		m_stylesMap[SString::Bold] = "{y:b}";
		m_stylesMap[SString::Italic] = "{y:i}";
		m_stylesMap[SString::Underline] = "{y:u}";
		m_stylesMap[SString::Bold | SString::Italic] = "{y:b,i}";
		m_stylesMap[SString::Bold | SString::Underline] = "{y:u,b}";
		m_stylesMap[SString::Italic | SString::Underline] = "{y:u,i}";
		m_stylesMap[SString::Bold | SString::Italic | SString::Underline] = "{y:u,b,i}";
	}

	const QString m_lineBuilder;
	mutable QMap<int, QString> m_stylesMap;
};
}

#endif
