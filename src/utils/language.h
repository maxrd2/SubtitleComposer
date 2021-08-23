/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LANGUAGE_H
#define LANGUAGE_H

#include <QMap>
#include <QList>
#include <QStringList>

namespace SubtitleComposer {
class Language
{
public:
	typedef enum {
		INVALID = -1,
		Auto = 0, Arabic, Bulgarian, Catalan, ChineseS, ChineseT, Croatian,
		Czech, Danish, Dutch, English, Filipino, Finnish, French, German,
		Greek, Hebrew, Hindi, Hungarian, Indonesian, Italian, Japanese, Korean, Latvian,
		Lithuanian, Norwegian, Polish, Portuguese, Romanian, Russian, Serbian,
		Slovak, Slovenian, Spanish, Swedish, Ukrainian, Vietnamese,
		SIZE
	} Value;

	static const QList<Value> & all();
	static const QList<Value> & input();
	static const QList<Value> & output();

	static Value fromCode(const QString &code);

	static const QString & code(Value language);
	static QString name(Value language);
	static QString flagPath(Value language);

	static QStringList codes(const QList<Value> &languages);
	static QStringList names(const QList<Value> &languages);
	static QStringList flagPaths(const QList<Value> &languages);
};
}
#endif
