/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SUBSTATIONALPHAINPUTFORMAT_H
#define SUBSTATIONALPHAINPUTFORMAT_H

#include "core/richtext/richdocument.h"
#include "helpers/common.h"
#include "formats/inputformat.h"

#include <QRegularExpression>
#include <QStringBuilder>
#include <QDebug>

namespace SubtitleComposer {
class SubStationAlphaInputFormat : public InputFormat
{
	friend class FormatManager;
	friend class AdvancedSubStationAlphaInputFormat;

protected:
	RichString toRichString(const QString &string) const
	{
		staticRE$(reCommands, "\\{([^\\}]+)\\}", REu);

		RichString ret;

		int currentStyle = 0;
		QRgb currentColor = 0;

		QRegularExpressionMatchIterator itCommands = reCommands.globalMatch(string);
		int offset = 0;
		while(itCommands.hasNext()) {
			QRegularExpressionMatch mCommands = itCommands.next();
			int newStyleFlags = currentStyle;
			QRgb newColor = currentColor;

			QString commands(mCommands.captured(1));
			QStringList commandsList(commands.split('\\'));
			for(QStringList::ConstIterator it = commandsList.constBegin(), end = commandsList.constEnd(); it != end; ++it) {
				if(it->isEmpty()) {
					continue;
				} else if(*it == QLatin1String("i0")) {
					newStyleFlags &= ~RichString::Italic;
				} else if(*it == QLatin1String("b0")) {
					newStyleFlags &= ~RichString::Bold;
				} else if(*it == QLatin1String("u0")) {
					newStyleFlags &= ~RichString::Underline;
				} else if(*it == QLatin1String("i1")) {
					newStyleFlags |= RichString::Italic;
				} else if(it->at(0) == 'b') {
					// it's usually followed 1, but can be weight of the font: 400, 700, ...
					newStyleFlags |= RichString::Bold;
				} else if(*it == QLatin1String("u1")) {
					newStyleFlags |= RichString::Underline;
				} else if(it->at(0) == 'c') {
					QString val = ($("000000") + it->mid(3).chopped(1)).right(6);
					if(val == QLatin1String("000000")) {
						newStyleFlags &= ~RichString::Color;
						newColor = 0;
					} else {
						newStyleFlags |= RichString::Color;
						newColor = QColor(QChar('#') % val.mid(4, 2) % val.mid(2, 2) % val.mid(0, 2)).rgb();
					}
				}
			}

			const QString text = string.mid(offset, mCommands.capturedStart() - offset)
					.replace(QLatin1String("\\N"), QLatin1String("\n"), Qt::CaseInsensitive);
			ret.append(RichString(text, currentStyle, currentColor));

			currentStyle = newStyleFlags;
			currentColor = newColor;
			offset = mCommands.capturedEnd();
		}
		const QString text = string.mid(offset)
				.replace(QLatin1String("\\N"), QLatin1String("\n"), Qt::CaseInsensitive);
		ret.append(RichString(text, currentStyle, currentColor));

		return ret;
	}

	bool parseSubtitles(Subtitle &subtitle, const QString &data) const override
	{
		staticRE$(reScriptInfo, "^ *\\[Script Info\\] *[\r\n]+", REu);
		if(!reScriptInfo.globalMatch(data).hasNext())
			return false;

		staticRE$(reStyles, "[\r\n]+ *\\[[vV]4\\+? Styles\\] *[\r\n]+", REu);
		QRegularExpressionMatchIterator itStyles = reStyles.globalMatch(data);
		if(!itStyles.hasNext())
			return false;
		const int stylesStart = itStyles.next().capturedStart();

		FormatData formatData = createFormatData();

		formatData.setValue($("ScriptInfo"), data.mid(0, stylesStart));

		staticRE$(reEvents, "[\r\n]+ *\\[Events\\] *[\r\n]+", REu);
		QRegularExpressionMatchIterator itEvents = reEvents.globalMatch(data, stylesStart);
		if(!itEvents.hasNext())
			return false;
		int eventsStart = itEvents.next().capturedStart();

		formatData.setValue($("Styles"), data.mid(stylesStart, eventsStart - stylesStart));

		staticRE$(reFormat, " *Format: *(\\w+,? *)+[\r\n]+", REu);
		QRegularExpressionMatchIterator itFormat = reFormat.globalMatch(data, eventsStart);
		if(!itFormat.hasNext())
			return false;

		setFormatData(subtitle, &formatData);
		formatData.clear();

		staticRE$(reDialogue, " *Dialogue: *[^,]+, *([^,]+), *([^,]+), *[^,]*, *[^,]*, *[^,]*, *[^,]*, *[^,]*, *[^,]*, *([^\r\n]*)[\r\n]+", REu);
		staticRE$(reDialogueData, " *(Dialogue: *[^,]+, *)[^,]+(, *)[^,]+(, *[^,]+, *[^,]*, *[^,]*, *[^,]*, *[^,]*, *[^,]*, *).*", REu);
		staticRE$(reTime, "(\\d+):(\\d+):(\\d+).(\\d+)", REu);

		do {
			QRegularExpressionMatch mFormat = itFormat.next();
			QRegularExpressionMatchIterator itDialogue = reDialogue.globalMatch(data, mFormat.capturedEnd());
			while(itDialogue.hasNext()) {
				QRegularExpressionMatch mDialogue = itDialogue.next();

				QRegularExpressionMatchIterator itTime = reTime.globalMatch(mDialogue.captured(1));
				if(!itTime.hasNext()) {
					qWarning() << "SubStationAlpha failed to match showTime";
					break;
				}
				QRegularExpressionMatch mTime = itTime.next();
				Time showTime(mTime.captured(1).toInt(), mTime.captured(2).toInt(), mTime.captured(3).toInt(), mTime.captured(4).toInt() * 10);

				itTime = reTime.globalMatch(mDialogue.captured(2));
				if(!itTime.hasNext()) {
					qWarning() << "SubStationAlpha failed to match hideTime";
					break;
				}
				mTime = itTime.next();
				Time hideTime(mTime.captured(1).toInt(), mTime.captured(2).toInt(), mTime.captured(3).toInt(), mTime.captured(4).toInt() * 10);

				SubtitleLine *line = new SubtitleLine(showTime, hideTime);
				line->primaryDoc()->setRichText(toRichString(mDialogue.captured(3)), true);

				formatData.setValue($("Dialogue"), mDialogue.captured(0).replace(reDialogueData, $("\\1%1\\2%2\\3%3\n")));
				setFormatData(line, &formatData);

				subtitle.insertLine(line);
			}
		} while(itFormat.hasNext());

		return true;
	}

	SubStationAlphaInputFormat(
			const QString &name = $("SubStation Alpha"),
			const QStringList &extensions = QStringList($("ssa")))
		: InputFormat(name, extensions)
	{}
};

class AdvancedSubStationAlphaInputFormat : public SubStationAlphaInputFormat
{
	friend class FormatManager;

protected:
	AdvancedSubStationAlphaInputFormat()
		: SubStationAlphaInputFormat($("Advanced SubStation Alpha"), QStringList($("ass")))
	{}
};

}

#endif
