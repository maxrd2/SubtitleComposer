/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MICRODVDOUTPUTFORMAT_H
#define MICRODVDOUTPUTFORMAT_H

#include "formats/outputformat.h"
#include "core/richdocument.h"
#include "core/subtitleiterator.h"

namespace SubtitleComposer {
class MicroDVDOutputFormat : public OutputFormat
{
	friend class FormatManager;

public:
	virtual ~MicroDVDOutputFormat() {}

protected:

	QString dumpSubtitles(const Subtitle &subtitle, bool primary) const override
	{
		QString ret;

		double framesPerSecond = subtitle.framesPerSecond();
		ret += m_lineBuilder
				.arg(1)
				.arg(1)
				.arg(QString::number(framesPerSecond, 'f', 3));

		for(SubtitleIterator it(subtitle); it.current(); ++it) {
			const SubtitleLine *line = it.current();

			const RichString &text = (primary ? line->primaryDoc() : line->secondaryDoc())->toRichText();
			QString subtitle;

			int prevStyle = 0;
			QRgb prevColor = 0;
			for(int i = 0, sz = text.length(); i < sz; i++) {
				int curStyle = text.styleFlagsAt(i);
				QRgb curColor = (curStyle &RichString::Color) != 0 ? text.styleColorAt(i) : 0;
				curStyle &= RichString::Bold | RichString::Italic | RichString::Underline;
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
					.arg(static_cast<long>((line->showTime().toMillis() / 1000.0) * framesPerSecond + 0.5))
					.arg(static_cast<long>((line->hideTime().toMillis() / 1000.0) * framesPerSecond + 0.5))
					.arg(subtitle);
		}
		return ret;
	}

	MicroDVDOutputFormat() :
		OutputFormat(QStringLiteral("MicroDVD"), QStringList() << QStringLiteral("sub") << QStringLiteral("txt")),
		m_lineBuilder(QStringLiteral("{%1}{%2}%3\n"))
	{
		m_stylesMap[0] = QStringLiteral("{y:}");
		m_stylesMap[RichString::Bold] = QStringLiteral("{y:b}");
		m_stylesMap[RichString::Italic] = QStringLiteral("{y:i}");
		m_stylesMap[RichString::Underline] = QStringLiteral("{y:u}");
		m_stylesMap[RichString::Bold | RichString::Italic] = QStringLiteral("{y:b,i}");
		m_stylesMap[RichString::Bold | RichString::Underline] = QStringLiteral("{y:u,b}");
		m_stylesMap[RichString::Italic | RichString::Underline] = QStringLiteral("{y:u,i}");
		m_stylesMap[RichString::Bold | RichString::Italic | RichString::Underline] = QStringLiteral("{y:u,b,i}");
	}

	const QString m_lineBuilder;
	mutable QMap<int, QString> m_stylesMap;
};
}

#endif
