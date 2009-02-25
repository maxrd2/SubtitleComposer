#ifndef SPELLINGCONFIG_H
#define SPELLINGCONFIG_H

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

#include "../../config/appconfiggroup.h"

#include <KGlobal>
#include <KLocale>

namespace SubtitleComposer
{
	class SpellingConfig : public AppConfigGroup
	{
		friend class Application;
		friend class SpellingConfigWidget;

		public:

			virtual AppConfigGroup* clone() const { return new SpellingConfig( *this ); }

			QString defaultLanguage() const { return option( keyDefaultLanguage() ); }
			void setDefaultLanguage( const QString& defaultLanguage ) { setOption( keyDefaultLanguage(), defaultLanguage ); }

			QString defaultClient() const { return option( keyDefaultClient() ); }
			void setDefaultClient( const QString& defaultClient ) { setOption( keyDefaultClient(), defaultClient ); }

			bool checkUppercase() const { return optionAsBool( keyCheckUppercase() ); }
			void setCheckUppercase( bool checkUppercase ) { setOption( keyCheckUppercase(), checkUppercase ); }

			bool skipRunTogether() const { return optionAsBool( keySkipRunTogether() ); }
			void setSkipRunTogether( bool skipRunTogether ) { setOption( keySkipRunTogether(), skipRunTogether ); }

			static const QString& keyDefaultLanguage() { static const QString key( "defaultLanguage" ); return key; }
			static const QString& keyDefaultClient() { static const QString key( "defaultClient" ); return key; }
			static const QString& keyCheckUppercase() { static const QString key( "checkUppercase" ); return key; }
			static const QString& keySkipRunTogether() { static const QString key( "skipRunTogether" ); return key; }

		private:

			SpellingConfig():AppConfigGroup( "Spelling", defaults() ) {}
			SpellingConfig( const SpellingConfig& config ):AppConfigGroup( config ) {}

			static QMap<QString,QString> defaults()
			{
				QMap<QString,QString> defaults;

				defaults[keyDefaultLanguage()] = KGlobal::locale()->language();
				defaults[keyDefaultClient()] = "";
				defaults[keyCheckUppercase()] = "true";
				defaults[keySkipRunTogether()] = "true";

				return defaults;
			}
	};
}

#endif
