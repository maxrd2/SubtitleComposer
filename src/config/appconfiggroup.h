#ifndef APPCONFIGGROUP_H
#define APPCONFIGGROUP_H

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

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtGui/QColor>

class KConfig;
class KSharedConfig;

namespace SubtitleComposer
{
	class AppConfig;
	class AppConfigGroup : public QObject
	{
		Q_OBJECT

		friend class AppConfig;

		public:

			/// NOTE: keys not in the defaults map are not valid
			AppConfigGroup( const QString& name, const QMap<QString,QString>& defaults );
			AppConfigGroup( const AppConfigGroup& config );
			AppConfigGroup& operator=( const AppConfigGroup& config );
			~AppConfigGroup();

			virtual AppConfigGroup* clone() const;

			const QString& name() const;

			/// returns true if config object has all the keys in this object and viceversa
			bool isCompatibleWith( const AppConfigGroup& config ) const;

			void loadDefaults();
			void readFrom( const KConfig* config );
			void readFrom( const KSharedConfig* config );
			void writeTo( KConfig* config ) const;
			void writeTo( KSharedConfig* config ) const;

			QString option( const QString& optionName ) const;
			bool optionAsBool( const QString& optionName ) const;
			long optionAsLong( const QString& optionName ) const;
			int optionAsInt( const QString& optionName ) const;
			QColor optionAsColor( const QString& optionName ) const;

			void setOption( const QString& optionName, const QString& value );
			void setOption( const QString& optionName, const char* value );
			void setOption( const QString& optionName, bool value );
			void setOption( const QString& optionName, int value );
			void setOption( const QString& optionName, long value );
			void setOption( const QString& optionName, const QColor& value );

			bool operator==( const AppConfigGroup& configGroup ) const;
			bool operator!=( const AppConfigGroup& configGroup ) const;

		signals:

			void optionChanged( const QString& optionName, const QString& value );
			void optionChanged( const QString& groupName, const QString& optionName, const QString& value );

		protected:

			static bool equals( const QString& str1, const QString& str2 );
			static bool nonEquals( const QString& str1, const QString& str2 );

		protected:

			AppConfig* m_config;

			QString m_name;
			QMap<QString,QString> m_values;
			QMap<QString,QString> m_defaultValues;
	};
}

#endif
