#ifndef ERRORSCONFIG_H
#define ERRORSCONFIG_H

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

namespace SubtitleComposer
{
	class ErrorsConfig : public AppConfigGroup
	{
		friend class Application;
		friend class ErrorsConfigWidget;

		public:

			virtual AppConfigGroup* clone() const { return new ErrorsConfig( *this ); }

			int minDuration() { return optionAsInt( keyMinDuration() ); } /// in milliseconds
			void setMinDuration( int mseconds ) { setOption( keyMinDuration(), mseconds ); }

			int maxDuration() { return optionAsInt( keyMaxDuration() ); } /// in milliseconds
			void setMaxDuration( int mseconds ) { setOption( keyMaxDuration(), mseconds ); }

			int minDurationPerChar() { return optionAsInt( keyMinDurationPerChar() ); } /// in milliseconds
			void setMinDurationPerChar( int mseconds ) { setOption( keyMinDurationPerChar(), mseconds ); }

			int maxDurationPerChar() { return optionAsInt( keyMaxDurationPerChar() ); } /// in milliseconds
			void setMaxDurationPerChar( int mseconds ) { setOption( keyMaxDurationPerChar(), mseconds ); }

			int maxCharacters() { return optionAsInt( keyMaxCharacters() ); }
			void setMaxCharacters( int characters ) { setOption( keyMaxCharacters(), characters ); }

			int maxLines() { return optionAsInt( keyMaxLines() ); }
			void setMaxLines( int lines ) { setOption( keyMaxLines(), lines ); }

			bool autoClearFixed() { return optionAsBool( keyAutoClearFixed() ); }
			void setAutoClearFixed( bool autoClearFixed ) { setOption( keyAutoClearFixed(), autoClearFixed ); }


			static const QString& keyMinDuration() { static const QString key( "MinDuration" ); return key; }
			static const QString& keyMaxDuration() { static const QString key( "MaxDuration" ); return key; }
			static const QString& keyMinDurationPerChar() { static const QString key( "MinDurationPerCharacter" ); return key; }
			static const QString& keyMaxDurationPerChar() { static const QString key( "MaxDurationPerCharacter" ); return key; }
			static const QString& keyMaxCharacters() { static const QString key( "MaxCharacters" ); return key; }
			static const QString& keyMaxLines() { static const QString key( "MaxLines" ); return key; }
			static const QString& keyAutoClearFixed() { static const QString key( "AutoClearFixed" ); return key; }

		private:

			ErrorsConfig():AppConfigGroup( "Errors", defaults() ) {}

			static QMap<QString,QString> defaults()
			{
				QMap<QString,QString> defaults;

				defaults[keyMinDuration()] = "700"; // in milliseconds
				defaults[keyMaxDuration()] = "5000"; // in milliseconds
				defaults[keyMinDurationPerChar()] = "30"; // in milliseconds
				defaults[keyMaxDurationPerChar()] = "185"; // in milliseconds
				defaults[keyMaxCharacters()] = "80";
				defaults[keyMaxLines()] = "2";
				defaults[keyAutoClearFixed()] = "false";

				return defaults;
			}
	};
}

#endif
