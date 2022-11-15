/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MICRODVDINPUTFORMAT_H
#define MICRODVDINPUTFORMAT_H

#include "core/richtext/richdocument.h"
#include "helpers/common.h"
#include "formats/inputformat.h"

#include <QRegularExpression>
#include <QStringBuilder>

namespace SubtitleComposer {
class MicroDVDInputFormat : public InputFormat
{
	friend class FormatManager;

protected:
	MicroDVDInputFormat()
		: InputFormat($("MicroDVD"), QStringList() << $("sub") << $("txt"))
	{}

	bool parseSubtitles(Subtitle &subtitle, const QString &data) const override
	{
		staticRE$(lineRE, "\\{(\\d+)\\}\\{(\\d+)\\}([^\n]+)\n", REu | REi);
		staticRE$(styleRE, "\\{([yc]):([^}]*)\\}", REu | REi);

		QRegularExpressionMatchIterator itLine = lineRE.globalMatch(data);
		if(!itLine.hasNext())
			return false; // couldn't find first line (content or FPS)

		QRegularExpressionMatch mLine = itLine.next();

		// if present, the FPS must by indicated by the first entry with both initial and final frames at 1
		bool ok;
		double fps = mLine.captured(3).toDouble(&ok);
		if(ok && mLine.captured(1) == QLatin1String("1") && mLine.captured(2) == QLatin1String("1")) {
			// first line contained the frames per second
			subtitle.setFramesPerSecond(fps);
		} else {
			// first line doesn't contain the FPS, use the value loaded by default
			fps = subtitle.framesPerSecond();
		}

		if(!itLine.hasNext())
			return false;

		do {
			mLine = itLine.next();

			Time showTime(static_cast<long>((mLine.captured(1).toLong() / fps) * 1000));
			Time hideTime(static_cast<long>((mLine.captured(2).toLong() / fps) * 1000));

			RichString richText;

			const QString text = mLine.captured(3);
			QRegularExpressionMatchIterator itText = styleRE.globalMatch(text);

			int globalStyle = 0, currentStyle = 0;
			QRgb globalColor = 0, currentColor = 0;
			int offsetPos = 0;
			while(itText.hasNext()) {
				QRegularExpressionMatch mText = itText.next();
				QString tag(mText.captured(1)), val(mText.captured(2).toLower());

				int newStyle = currentStyle;
				QRgb newColor = currentColor;

				if(tag == QChar('Y')) {
					globalStyle = 0;
					if(val.contains('b'))
						globalStyle |= RichString::Bold;
					if(val.contains('i'))
						globalStyle |= RichString::Italic;
					if(val.contains('u'))
						globalStyle |= RichString::Underline;
				} else if(tag == QLatin1String("C")) {
					globalColor = val.length() != 7 ? 0 : QColor(QChar('#') % val.mid(5, 2) % val.mid(3, 2) % val.mid(1, 2)).rgb();
				} else if(tag == QLatin1String("y")) {
					newStyle = 0;
					if(val.contains('b'))
						newStyle |= RichString::Bold;
					if(val.contains('i'))
						newStyle |= RichString::Italic;
					if(val.contains('u'))
						newStyle |= RichString::Underline;
				} else if(tag == QLatin1String("c")) {
					newColor = val.length() != 7 ? 0 : QColor(QChar('#') % val.mid(5, 2) % val.mid(3, 2) % val.mid(1, 2)).rgb();
				}

				if(newStyle != currentStyle || currentColor != newColor) {
					QString token(text.mid(offsetPos, mText.capturedStart() - offsetPos));
					richText += RichString(token, currentStyle | (currentColor == 0 ? 0 : RichString::Color), currentColor);
					currentStyle = newStyle;
					currentColor = newColor;
				}

				offsetPos = mText.capturedEnd();
			}

			QString token(text.mid(offsetPos));
			richText += RichString(token, currentStyle | (currentColor == 0 ? 0 : RichString::Color), currentColor);

			if(globalColor != 0)
				globalStyle |= RichString::Color;
			if(globalStyle != 0) {
				for(int i = 0, sz = richText.length(); i < sz; i++) {
					if(richText.styleFlagsAt(i) == 0) {
						richText.setStyleFlagsAt(i, globalStyle);
						richText.setStyleColorAt(i, globalColor);
					}
				}
			}

			SubtitleLine *l = new SubtitleLine(showTime, hideTime);
			l->primaryDoc()->setRichText(richText.replace('|', '\n'), true);
			subtitle.insertLine(l);
		} while(itLine.hasNext());

		return true;
	}
};
}

#endif
