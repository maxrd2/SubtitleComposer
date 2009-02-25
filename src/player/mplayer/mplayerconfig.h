#ifndef MPLAYERCONFIG_H
#define MPLAYERCONFIG_H

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

#include <KStandardDirs>

namespace SubtitleComposer
{
	class MPlayerConfig : public AppConfigGroup
	{
		friend class MPlayerBackend;
		friend class MPlayerConfigWidget;

		public:

			virtual AppConfigGroup* clone() const { return new MPlayerConfig( *this ); }

			QString executablePath() const { return option( keyExecutablePath() ); }
			void setExecutablePath( const QString& execPath ) { setOption( keyExecutablePath(), execPath ); }

			bool hasAudioOutput() const { return ! option( keyAudioOutput() ).isEmpty(); }
			QString audioOutput() const { return option( keyAudioOutput() ); }
			void setAudioOutput( const QString& audioOutput ) { setOption( keyAudioOutput(), audioOutput ); }

			bool hasVideoOutput() const { return ! option( keyVideoOutput() ).isEmpty(); }
			QString videoOutput() const { return option( keyVideoOutput() ); }
			void setVideoOutput( const QString& videoOutput ) { setOption( keyVideoOutput(), videoOutput ); }

			bool hasCacheSize() const { return ! option( keyCacheSize() ).isEmpty(); }
			int cacheSize() const { return optionAsInt( keyCacheSize() ); } /// in kbytes
			void setCacheSize( int kbytes ) { setOption( keyCacheSize(), kbytes < 0 ? "" : QString::number( kbytes ) ); }

			bool volumeNormalizationEnabled() const { return optionAsBool( keyVolumeNormalizationEnabled() ); }
			void setVolumeNormalizationEnabled( bool enabled ) { setOption( keyVolumeNormalizationEnabled(), enabled ); }

			bool hasInputConfigPath() const { return ! option( keyInputConfigPath() ).isEmpty(); }
			QString inputConfigPath() const { return option( keyInputConfigPath() ); }
			void setInputConfigPath( const QString& inputConfigPath ) { setOption( keyInputConfigPath(), inputConfigPath ); }

			bool frameDropping() const { return optionAsBool( keyFrameDropping() ); }
			void setFrameDropping( bool frameDropping ) { setOption( keyFrameDropping(), frameDropping ); }

			bool hardFrameDropping() const { return optionAsBool( keyHardFrameDropping() ); }
			void setHardFrameDropping( bool hardFrameDropping ) { setOption( keyHardFrameDropping(), hardFrameDropping ); }

			bool hasAutoSyncFactor() const { return ! option( keyAutoSyncFactor() ).isEmpty(); }
			int autoSyncFactor() const { return optionAsInt( keyAutoSyncFactor() ); }
			void setAutoSyncFactor( int factor ) { setOption( keyAutoSyncFactor(), factor < 0 ? "" : QString::number( factor ) );}


			static const QString& keyExecutablePath() { static const QString key( "ExecutablePath" ); return key; }
			static const QString& keyAudioOutput() { static const QString key( "AudioOutput" ); return key; }
			static const QString& keyVideoOutput() { static const QString key( "VideoOutput" ); return key; }
			static const QString& keyCacheSize() { static const QString key( "CacheSize" ); return key; }
			static const QString& keyVolumeNormalizationEnabled() { static const QString key( "VolumeNormalizationEnabled" ); return key; }
			static const QString& keyInputConfigPath() { static const QString key( "InputConfigPath" ); return key; }
			static const QString& keyFrameDropping() { static const QString key( "FrameDropping" ); return key; }
			static const QString& keyHardFrameDropping() { static const QString key( "HardFrameDropping" ); return key; }
			static const QString& keyAutoSyncFactor() { static const QString key( "AutoSyncFactor" ); return key; }

		private:

			MPlayerConfig():AppConfigGroup( "MPlayer", defaults() ) {}
			MPlayerConfig( const MPlayerConfig& config ):AppConfigGroup( config ) {}

			static QMap<QString,QString> defaults()
			{
				QMap<QString,QString> defaults;

				defaults[keyExecutablePath()] = "mplayer";
				defaults[keyAudioOutput()] = "";
				defaults[keyVideoOutput()] = "";

				defaults[keyCacheSize()] = "5120"; // in kbytes

				defaults[keyVolumeNormalizationEnabled()] = "false";

				defaults[keyInputConfigPath()] = KGlobal::dirs()->locate( "appdata", "input.conf" );

				defaults[keyFrameDropping()] = "false";
				defaults[keyHardFrameDropping()] = "false";
				defaults[keyAutoSyncFactor()] = "";

				return defaults;
			}
	};
}

#endif
