#ifndef LANGCODES_H
#define LANGCODES_H

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

#include <QtCore/QString>

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
