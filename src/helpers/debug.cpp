/*
    SPDX-FileCopyrightText: 2020-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "debug.h"

#include "helpers/common.h"

#include <QMetaEnum>
#include <QStringBuilder>

using namespace SubtitleComposer;


QString
SubtitleComposer::propertyName(QTextFormat::Property key)
{
	if(key >= QTextFormat::UserProperty)
		return $("UserProperty+") % QString::number(key - QTextFormat::UserProperty);
	const char *keyName = QMetaEnum::fromType<QTextFormat::Property>().valueToKey(static_cast<int>(key));
	return keyName ? keyName : QString::number(key);
}

QString
SubtitleComposer::textFormatString(const QTextFormat &format)
{
	QString res;
	const QMap<int, QVariant> &props = format.properties();
	for(auto it = props.cbegin(); it != props.cend(); ++it)
		res.append(propertyName(QTextFormat::Property(it.key())) % QChar(':') % it.value().toString() % $("; "));
	return res;
}

QString
SubtitleComposer::dumpFormatRanges(const QString &text, const QVector<QTextLayout::FormatRange> &ranges)
{
	QString res;
	for(const QTextLayout::FormatRange &r: ranges) {
		res.append(QChar::LineFeed % QChar('"') % text.midRef(r.start, r.length) % QChar('"') % QChar::Space % textFormatString(r.format));
	}
	return res;
}
