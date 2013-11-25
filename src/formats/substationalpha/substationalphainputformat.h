#ifndef SUBSTATIONALPHAINPUTFORMAT_H
#define SUBSTATIONALPHAINPUTFORMAT_H

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

#include <QRegExp>

namespace SubtitleComposer {
class SubStationAlphaInputFormat : public InputFormat
{
	friend class FormatManager;
	friend class AdvancedSubStationAlphaInputFormat;

public:
	virtual ~SubStationAlphaInputFormat() {}

protected:
	SString toSString(QString string) const
	{
		static QRegExp cmdRegExp("\\{([^\\}]+)\\}");

		SString ret;

		string.replace("\\N", "\n");
		string.replace("\\n", "\n");

		int currentStyle = 0;
		QRgb currentColor = 0;

		int offsetPos = 0, matchedPos;
		while((matchedPos = cmdRegExp.indexIn(string, offsetPos)) != -1) {
			int newStyleFlags = currentStyle;
			QRgb newColor = currentColor;

			QString commands(cmdRegExp.cap(1));
			QStringList commandsList(commands.split('\\'));
			for(QStringList::ConstIterator it = commandsList.begin(), end = commandsList.end(); it != end; ++it) {
				if(it->isEmpty()) {
					continue;
				} else if(*it == "i0") {
					newStyleFlags &= ~SString::Italic;
				} else if(*it == "b0") {
					newStyleFlags &= ~SString::Bold;
				} else if(*it == "u0") {
					newStyleFlags &= ~SString::Underline;
				} else if(*it == "i1") {
					newStyleFlags |= SString::Italic;
				} else if(it->at(0) == 'b') {
					// it's usually followed 1, but can be weight of the font: 400, 700, ...
					newStyleFlags |= SString::Bold;
				} else if(*it == "u1") {
					newStyleFlags |= SString::Underline;
				} else if(it->at(0) == 'c') {
					QString val = ("000000" + it->mid(3, -2)).right(6);
					if(val == "000000") {
						newStyleFlags &= ~SString::Color;
						newColor = 0;
					} else {
						newStyleFlags |= SString::Color;
						newColor = QColor("#" + val.mid(4, 2) + val.mid(2, 2) + val.mid(0, 2)).rgb();
					}
				}
			}

			ret.append(SString(string.mid(offsetPos, matchedPos - offsetPos), currentStyle, currentColor));

			currentStyle = newStyleFlags;
			currentColor = newColor;
			offsetPos = matchedPos + cmdRegExp.matchedLength();
		}
		ret.append(SString(string.mid(offsetPos, matchedPos - offsetPos), currentStyle, currentColor));

		return ret;
	}

	virtual bool parseSubtitles(Subtitle &subtitle, const QString &data) const
	{
		if(m_scriptInfoRegExp.indexIn(data) == -1)
			return false;

		int stylesStart = m_stylesRegExp.indexIn(data);
		if(stylesStart == -1)
			return false;

		FormatData formatData = createFormatData();

		formatData.setValue("ScriptInfo", data.mid(0, stylesStart));

		int eventsStart = m_eventsRegExp.indexIn(data, stylesStart);
		if(eventsStart == -1)
			return false;

		formatData.setValue("Styles", data.mid(stylesStart, eventsStart - stylesStart));

		if(m_formatRegExp.indexIn(data, eventsStart) == -1)
			return false;

		setFormatData(subtitle, formatData);
		formatData.clear();

		unsigned readLines = 0;

		int offset = m_formatRegExp.pos() + m_formatRegExp.matchedLength();
		for(; m_dialogueRegExp.indexIn(data, offset) != -1; offset += m_dialogueRegExp.matchedLength()) {
			if(m_timeRegExp.indexIn(m_dialogueRegExp.cap(1)) == -1)
				continue;
			Time showTime(m_timeRegExp.cap(1).toInt(), m_timeRegExp.cap(2).toInt(), m_timeRegExp.cap(3).toInt(), m_timeRegExp.cap(4).toInt() * 10);

			if(m_timeRegExp.indexIn(m_dialogueRegExp.cap(2)) == -1)
				continue;
			Time hideTime(m_timeRegExp.cap(1).toInt(), m_timeRegExp.cap(2).toInt(), m_timeRegExp.cap(3).toInt(), m_timeRegExp.cap(4).toInt() * 10);

			SubtitleLine *line = new SubtitleLine(toSString(m_dialogueRegExp.cap(3)), showTime, hideTime);

			formatData.setValue("Dialogue", m_dialogueRegExp.cap(0).replace(m_dialogueDataRegExp, "\\1%1\\2%2\\3%3\n"));
			setFormatData(line, formatData);

			subtitle.insertLine(line);

			readLines++;
		}
		return readLines > 0;
	}

	SubStationAlphaInputFormat(
			const QString &name = "SubStation Alpha",
			const QStringList &extensions = QStringList("ssa"),
			const QString &stylesRegExp = s_stylesRegExp) :
		InputFormat(name, extensions),
		m_scriptInfoRegExp(s_scriptInfoRegExp),
		m_stylesRegExp(stylesRegExp),
		m_eventsRegExp(s_eventsRegExp),
		m_formatRegExp(s_formatRegExp),
		m_dialogueRegExp(s_dialogueRegExp),
		m_dialogueDataRegExp(s_dialogueDataRegExp),
		m_timeRegExp(s_timeRegExp)
	{}

	mutable QRegExp m_scriptInfoRegExp;
	mutable QRegExp m_stylesRegExp;
	mutable QRegExp m_eventsRegExp;
	mutable QRegExp m_formatRegExp;
	mutable QRegExp m_dialogueRegExp;
	mutable QRegExp m_dialogueDataRegExp;
	mutable QRegExp m_timeRegExp;

	static const char *s_scriptInfoRegExp;
	static const char *s_stylesRegExp;
	static const char *s_formatRegExp;
	static const char *s_eventsRegExp;
	static const char *s_dialogueRegExp;
	static const char *s_dialogueDataRegExp;
	static const char *s_timeRegExp;
};

const char *SubStationAlphaInputFormat::s_scriptInfoRegExp = "^ *\\[Script Info\\] *[\r\n]+";
const char *SubStationAlphaInputFormat::s_stylesRegExp = "[\r\n]+ *\\[[vV]4 Styles\\] *[\r\n]+";
const char *SubStationAlphaInputFormat::s_formatRegExp = " *Format: *(\\w+,? *)+[\r\n]+";
const char *SubStationAlphaInputFormat::s_eventsRegExp = "[\r\n]+ *\\[Events\\] *[\r\n]+";
const char *SubStationAlphaInputFormat::s_dialogueRegExp = " *Dialogue: *[^,]+, *([^,]+), *([^,]+), *[^,]+, *[^,]*, *\\d{4}, *\\d{4}, *\\d{4}, *[^,]*, *([^\r\n]*)[\r\n]+";
const char *SubStationAlphaInputFormat::s_dialogueDataRegExp = " *(Dialogue: *[^,]+, *)[^,]+(, *)[^,]+(, *[^,]+, *[^,]*, *\\d{4}, *\\d{4}, *\\d{4}, *[^,]*, *).*";
const char *SubStationAlphaInputFormat::s_timeRegExp = "(\\d+):(\\d+):(\\d+).(\\d+)";

class AdvancedSubStationAlphaInputFormat : public SubStationAlphaInputFormat
{
	friend class FormatManager;

protected:
	AdvancedSubStationAlphaInputFormat() :
		SubStationAlphaInputFormat("Advanced SubStation Alpha", QStringList("ass"), "[\r\n]+ *\\[[vV]4\\+ Styles\\] *[\r\n]+")
	{}
};

}

#endif
