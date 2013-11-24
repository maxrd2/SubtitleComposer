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

#include "language.h"

#include <QtCore/QFile>

#include <KGlobal>
#include <KLocale>
#include <KStandardDirs>

using namespace SubtitleComposer;

const QList<Language::Value> &
Language::all()
{
	static QList<Value> languages;
	if(languages.empty()) {
		for(int index = Auto; index < SIZE; ++index)
			languages << (Value)index;
	}
	return languages;
}

const QList<Language::Value> &
Language::input()
{
	static QList<Value> inputLanguages;
	if(inputLanguages.empty()) {
		inputLanguages = all();
		inputLanguages.removeAll(ChineseT);
	}
	return inputLanguages;
}

const QList<Language::Value> &
Language::output()
{
	static QList<Value> outputLanguages;
	if(outputLanguages.empty()) {
		outputLanguages = all();
		outputLanguages.removeAll(Auto);
	}
	return outputLanguages;
}

Language::Value
Language::fromCode(const QString &code)
{
	for(QList<Value>::ConstIterator it = all().begin(), end = all().end(); it != end; ++it) {
		if(Language::code(*it) == code)
			return *it;
	}
	return INVALID;
}

const QString &
Language::code(Language::Value language)
{
	static QMap<Value, QString> codes;
	static QString notFound;

	if(codes.empty()) {
		codes[Auto] = "auto";
		codes[Arabic] = "ar";
		codes[Bulgarian] = "bg";
		codes[Catalan] = "ca";
		codes[ChineseS] = "zh-CN";
		codes[ChineseT] = "zh-TW";
		codes[Czech] = "cs";
		codes[Danish] = "da";
		codes[German] = "de";
		codes[Greek] = "el";
		codes[English] = "en";
		codes[Spanish] = "es";
		codes[Finnish] = "fi";
		codes[French] = "fr";
		codes[Hindi] = "hi";
		codes[Hungarian] = "hu";
		codes[Croatian] = "hr";
		codes[Indonesian] = "id";
		codes[Italian] = "it";
		codes[Hebrew] = "iw";
		codes[Japanese] = "ja";
		codes[Korean] = "ko";
		codes[Lithuanian] = "lt";
		codes[Latvian] = "lv";
		codes[Dutch] = "nl";
		codes[Norwegian] = "no";
		codes[Polish] = "pl";
		codes[Portuguese] = "pt";
		codes[Romanian] = "ro";
		codes[Russian] = "ru";
		codes[Slovak] = "sk";
		codes[Slovenian] = "sl";
		codes[Serbian] = "sr";
		codes[Swedish] = "sv";
		codes[Filipino] = "tl";
		codes[Ukrainian] = "uk";
		codes[Vietnamese] = "vi";
	}

	return codes.contains(language) ? codes[language] : notFound;
}

QString
Language::name(Language::Value language)
{
	switch(language) {
	case Auto:
		return i18n("Automatic");
	case ChineseS:
		return KGlobal::locale()->languageCodeToName("zh_CN");
	case ChineseT:
		return KGlobal::locale()->languageCodeToName("zh_TW");
	// case Filipino:        return KGlobal::locale()->languageCodeToName( "tl" );
	case Filipino:
		return i18nc("Official language of the Philippines, aka Filipino", "Tagalog");
	case Hebrew:
		return KGlobal::locale()->languageCodeToName("he");
	case Norwegian:
		return KGlobal::locale()->languageCodeToName("nb");
	default:
		return KGlobal::locale()->languageCodeToName(code(language));
	}
}

QString
Language::flagPath(Value language)
{
	static QStringList localesDirs = KGlobal::dirs()->findDirs("locale", "l10n");
	static QString localeDir = localesDirs.isEmpty() ? QString() : localesDirs.first();

	QString localeCode;

	if(localeDir.isEmpty())
		return localeCode;

	switch(language) {
	case Arabic:
		localeCode = "km";
		break;
	case Bulgarian:
		localeCode = "bg";
		break;
	case Catalan:
		localeCode = "es";
		break;
	case ChineseS:
		localeCode = "cn";
		break;
	case ChineseT:
		localeCode = "cn";
		break;
	case Czech:
		localeCode = "cz";
		break;
	case Danish:
		localeCode = "dk";
		break;
	case German:
		localeCode = "de";
		break;
	case Greek:
		localeCode = "gr";
		break;
	case English:
		localeCode = "gb";
		break;
	case Spanish:
		localeCode = "es";
		break;
	case Finnish:
		localeCode = "fi";
		break;
	case French:
		localeCode = "fr";
		break;
	case Hindi:
		localeCode = "in";
		break;
	case Hungarian:
		localeCode = "hu";
		break;
	case Croatian:
		localeCode = "hr";
		break;
	case Indonesian:
		localeCode = "id";
		break;
	case Italian:
		localeCode = "it";
		break;
	case Hebrew:
		localeCode = "il";
		break;
	case Japanese:
		localeCode = "jp";
		break;
	case Korean:
		localeCode = "kr";
		break;
	case Lithuanian:
		localeCode = "lt";
		break;
	case Latvian:
		localeCode = "lv";
		break;
	case Dutch:
		localeCode = "nl";
		break;
	case Norwegian:
		localeCode = "no";
		break;
	case Polish:
		localeCode = "pl";
		break;
	case Portuguese:
		localeCode = "pt";
		break;
	case Romanian:
		localeCode = "ro";
		break;
	case Russian:
		localeCode = "ru";
		break;
	case Slovak:
		localeCode = "sk";
		break;
	case Slovenian:
		localeCode = "si";
		break;
	case Serbian:
		localeCode = "rs";
		break;
	case Swedish:
		localeCode = "se";
		break;
	case Filipino:
		localeCode = "ph";
		break;
	case Ukrainian:
		localeCode = "ua";
		break;
	case Vietnamese:
		localeCode = "vn";
		break;
	default:
		return QString();
	}

	QString flagPath = localeDir + localeCode + "/flag.png";
	return QFile::exists(flagPath) ? flagPath : QString();
}

QStringList
Language::codes(const QList<Language::Value> &languages)
{
	QStringList codes;
	for(QList<Value>::ConstIterator it = languages.begin(), end = languages.end(); it != end; ++it)
		codes << code(*it);
	return codes;
}

QStringList
Language::names(const QList<Language::Value> &languages)
{
	QStringList names;
	for(QList<Value>::ConstIterator it = languages.begin(), end = languages.end(); it != end; ++it)
		names << name(*it);
	return names;
}

QStringList
Language::flagPaths(const QList<Language::Value> &languages)
{
	QStringList flagPaths;
	for(QList<Value>::ConstIterator it = languages.begin(), end = languages.end(); it != end; ++it)
		flagPaths << flagPath(*it);
	return flagPaths;
}
