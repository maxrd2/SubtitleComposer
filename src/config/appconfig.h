#ifndef APPCONFIG_H
#define APPCONFIG_H

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

#include "appconfiggroup.h"

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QMap>

class KConfig;
class KSharedConfig;

namespace SubtitleComposer
{
	class AppConfig : public QObject
	{
		Q_OBJECT

		public:

			AppConfig();
			AppConfig( const AppConfig& config );
			AppConfig& operator=( const AppConfig& config );
			~AppConfig();

			/// returns true if config object has the same groups as this object
			/// and all corresponding groups are compatible between them
			bool isCompatibleWith( const AppConfig& config );

			void loadDefaults();

			void readFrom( const KConfig* config );
			void readFrom( const KSharedConfig* config );
			void writeTo( KConfig* config ) const;
			void writeTo( KSharedConfig* config ) const;

			AppConfigGroup* group( const QString& name );
			const AppConfigGroup* const group( const QString& name ) const;

			void setGroup( AppConfigGroup* group ); /// ownership is transferred to this object
			AppConfigGroup* removeGroup( const QString& name ); /// ownership is transferred to the caller

		signals:

			void optionChanged( const QString& groupName, const QString& optionName, const QString& value );

		protected:

			QMap<QString,AppConfigGroup*> m_groups;
	};
}

#endif
