#ifndef MICRODVDINPUTFORMAT_H
#define MICRODVDINPUTFORMAT_H

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
class MicroDVDInputFormat : public InputFormat
{
	friend class FormatManager;

public:
	virtual ~MicroDVDInputFormat() {}

protected:
	virtual bool parseSubtitles(Subtitle &subtitle, const QString &data) const
	{
		if(m_lineRegExp.indexIn(data, 0) == -1)
			return false; // couldn't find first line (content or FPS)

		int offset = 0;

		// if present, the FPS must by indicated by the first entry with both initial and final frames at 1
		bool ok;
		double framesPerSecond = m_lineRegExp.cap(3).toDouble(&ok);
		if(ok && m_lineRegExp.cap(1) == "1" && m_lineRegExp.cap(2) == "1") {
			// first line contained the frames per second
			subtitle.setFramesPerSecond(framesPerSecond);

			offset += m_lineRegExp.matchedLength();
			if(m_lineRegExp.indexIn(data, offset) == -1)
				return false; // couldn't find first line with content
		} else {
			// first line doesn't contain the FPS, use the value loaded by default
			framesPerSecond = subtitle.framesPerSecond();
		}

		unsigned readLines = 0;

		do {
			offset += m_lineRegExp.matchedLength();

			Time showTime((long)((m_lineRegExp.cap(1).toLong() / framesPerSecond) * 1000));
			Time hideTime((long)((m_lineRegExp.cap(2).toLong() / framesPerSecond) * 1000));

			SString richText;

			QString text = m_lineRegExp.cap(3);

			int globalStyle = 0, currentStyle = 0;
			QRgb globalColor = 0, currentColor = 0;
			int offsetPos = 0, matchedPos;
			while((matchedPos = m_styleRegExp.indexIn(text, offsetPos)) != -1) {
				QString tag(m_styleRegExp.cap(1)), val(m_styleRegExp.cap(2).toLower());

				int newStyle = currentStyle;
				QRgb newColor = currentColor;

				if(tag == "Y") {
					globalStyle = 0;
					if(val.contains('b'))
						globalStyle |= SString::Bold;
					if(val.contains('i'))
						globalStyle |= SString::Italic;
					if(val.contains('u'))
						globalStyle |= SString::Underline;
				} else if(tag == "C") {
					globalColor = val.length() != 7 ? 0 : QColor("#" + val.mid(5, 2) + val.mid(3, 2) + val.mid(1, 2)).rgb();
				} else if(tag == "y") {
					newStyle = 0;
					if(val.contains('b'))
						newStyle |= SString::Bold;
					if(val.contains('i'))
						newStyle |= SString::Italic;
					if(val.contains('u'))
						newStyle |= SString::Underline;
				} else if(tag == "c") {
					newColor = val.length() != 7 ? 0 : QColor("#" + val.mid(5, 2) + val.mid(3, 2) + val.mid(1, 2)).rgb();
				}

				if(newStyle != currentStyle || currentColor != newColor) {
					QString token(text.mid(offsetPos, matchedPos - offsetPos));
					richText += SString(token, currentStyle | (currentColor == 0 ? 0 : SString::Color), currentColor);
					currentStyle = newStyle;
					currentColor = newColor;
				}

				offsetPos = matchedPos + m_styleRegExp.cap(0).length();
			}

			QString token(text.mid(offsetPos, matchedPos - offsetPos));
			richText += SString(token, currentStyle | (currentColor == 0 ? 0 : SString::Color), currentColor);

			if(globalColor != 0)
				globalStyle |= SString::Color;
			if(globalStyle != 0) {
				for(int i = 0, sz = richText.length(); i < sz; i++) {
					if(richText.styleFlagsAt(i) == 0) {
						richText.setStyleFlagsAt(i, globalStyle);
						richText.setStyleColorAt(i, globalColor);
					}
				}
			}

			subtitle.insertLine(new SubtitleLine(richText.replace('|', '\n'), showTime, hideTime));

			readLines++;
		} while(m_lineRegExp.indexIn(data, offset) != -1);

		return readLines > 0;
	}

	MicroDVDInputFormat() :
		InputFormat("MicroDVD", QString("sub:txt").split(":")),
		m_lineRegExp("\\{(\\d+)\\}\\{(\\d+)\\}([^\n]+)\n", Qt::CaseInsensitive),
		m_styleRegExp("\\{([yc]):([^}]*)\\}", Qt::CaseInsensitive)
	{}

	mutable QRegExp m_lineRegExp;
	mutable QRegExp m_styleRegExp;
};
}

#endif
