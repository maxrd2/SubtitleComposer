/*
    SPDX-FileCopyrightText: 2021-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "webvttoutputformat.h"

#include "core/richtext/richdocument.h"
#include "core/subtitleiterator.h"
#include "helpers/common.h"

#include <QRegularExpression>

using namespace SubtitleComposer;


WebVTTOutputFormat::WebVTTOutputFormat()
	: OutputFormat($("WebVTT"), QStringList($("vtt")))
{}

inline static QString
fixEmptyLines(QString str)
{
	staticRE$(reEmptyLine, "\\n(?=\\n)", REu);
	return str.replace(reEmptyLine, $("\n "));
}

QString
WebVTTOutputFormat::dumpSubtitles(const Subtitle &subtitle, bool primary) const
{
	QString ret;

	ret += $("WEBVTT");
	ret.append(QChar::LineFeed);
	const QString &intro = fixEmptyLines(subtitle.meta("comment.intro.0"));
	if(!intro.isEmpty()) {
		ret.append(intro);
		ret.append(QChar::LineFeed);
	}
	ret.append(QChar::LineFeed);

	for(int noteId = 0;;) {
		const QByteArray key(QByteArray("comment.top.") + QByteArray::number(noteId++));
		if(!subtitle.metaExists(key))
			break;
		ret.append($("NOTE"));
		ret.append(QChar::LineFeed);
		ret.append(fixEmptyLines(subtitle.meta(key)));
		ret.append(QChar::LineFeed);
		ret.append(QChar::LineFeed);
	}

	if(!subtitle.stylesheet()->unformattedCSS().isEmpty()) {
		ret.append($("STYLE"));
		ret.append(QChar::LineFeed);
		ret.append(fixEmptyLines(subtitle.stylesheet()->unformattedCSS()));
		ret.append(QChar::LineFeed);
		ret.append(QChar::LineFeed);
	}

	for(SubtitleIterator it(subtitle); it.current(); ++it) {
		const SubtitleLine *line = it.current();

		const QString &comment = line->meta("comment");
		if(!comment.isEmpty()) {
			ret.append($("NOTE"));
			ret.append(comment.contains(QChar::LineFeed) ? QChar::LineFeed : QChar::Space);
			ret.append(fixEmptyLines(comment));
			ret.append(QChar::LineFeed);
			ret.append(QChar::LineFeed);
		}

		const QString &cueId = line->meta("id");
		if(!cueId.isEmpty()) {
			ret.append(cueId);
			ret.append(QChar::LineFeed);
		}

		const Time showTime = line->showTime();
		const Time hideTime = line->hideTime();
		ret += QString::asprintf("%02d:%02d:%02d.%03d --> %02d:%02d:%02d.%03d",
					showTime.hours(), showTime.minutes(), showTime.seconds(), showTime.millis(),
					hideTime.hours(), hideTime.minutes(), hideTime.seconds(), hideTime.millis());
		const SubtitleRect &p = line->pos();
		// FIXME: consider hAlign/vAlign in rect calculations
		// FIXME: position/line can have extra alignment/anchor parameter
		if(p.vertical) {
			ret.append($(" vertical:lr")); // FIXME: RTL support (vertical:rl)
			const int top = p.top;
			const int left = p.left;
			const int height = int(p.bottom) - top;
			if(left) // FIXME: with vertical:rl should be right
				ret.append(QString::asprintf(" line:%02d%%", left));
			if(top)
				ret.append(QString::asprintf(" position:%02d%%", top));
			if(height != 100)
				ret.append(QString::asprintf(" size:%02d%%", height));
		} else {
			const int top = p.top;
			const int left = p.left;
			const int width = int(p.right) - left;
			if(top)
				ret.append(QString::asprintf(" line:%02d%%", top));
			if(left)
				ret.append(QString::asprintf(" position:%02d%%", left));
			if(width != 100)
				ret.append(QString::asprintf(" size:%02d%%", width));
		}
		if(p.hAlign == SubtitleRect::START)
			ret.append(QLatin1String(" align:start"));
		else if(p.hAlign == SubtitleRect::END)
			ret.append(QLatin1String(" align:end"));
		ret.append(QChar::LineFeed);

		const RichString text = (primary ? line->primaryDoc() : line->secondaryDoc())->toRichText();
		ret += text.richString()
				.replace(QLatin1String("&amp;"), QLatin1String("&"))
				.replace(QLatin1String("&lt;"), QLatin1String("<"))
				.replace(QLatin1String("&gt;"), QLatin1String(">"));

		ret += $("\n\n");
	}
	return ret;
}
