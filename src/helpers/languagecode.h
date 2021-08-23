/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LANGCODES_H
#define LANGCODES_H

#include <QString>

class LanguageCode
{
public:
	/**
	 * @brief toIso2
	 * @param iso3Code ISO-3 language code
	 * @return ISO-2 language code
	 */
	static QString toIso2(const QString &iso3Code);
	/**
	 * @brief toIso3
	 * @param iso2Code ISO-2 language code
	 * @return ISO-3 language code
	 */
	static QString toIso3(const QString &iso2Code);

	/**
	 * @brief nameFromIso2
	 * @param iso2Code ISO-2 language code
	 * @return Localized language name or "Unknown"
	 */
	static QString nameFromIso2(const QString &iso2Code);
	/**
	 * @brief nameFromIso3
	 * @param iso3Code ISO-3 language code
	 * @return Localized language name or "Unknown"
	 */
	static QString nameFromIso3(const QString &iso3Code);
	/**
	 * @brief nameFromIso
	 * @param isoCode ISO-2 or ISO-3 language code
	 * @return Localized language name or "Unknown (isoCode)"
	 */
	static QString nameFromIso(const QString &isoCode);
};

#endif
