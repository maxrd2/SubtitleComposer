/*
    SPDX-FileCopyrightText: 2020-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef DEBUG_H
#define DEBUG_H

#include <QVector>
#include <QTextLayout>

namespace SubtitleComposer {

QString propertyName(QTextFormat::Property key);
QString textFormatString(const QTextFormat &format);
QString dumpFormatRanges(const QString &text, const QVector<QTextLayout::FormatRange> &ranges);

}

#endif // DEBUG_H
