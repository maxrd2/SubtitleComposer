#ifndef PLAYERCONFIG_H
#define PLAYERCONFIG_H

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

#include <QtGui/QColor>

namespace SubtitleComposer
{
	class PlayerConfig : public AppConfigGroup
	{
		friend class Application;
		friend class PlayerConfigWidget;

		public:

			virtual AppConfigGroup* clone() const { return new PlayerConfig( *this ); }

			QString backend() const { return option( keyBackend() ); }
			void setBackend( const QString& backend ) { setOption( keyBackend(), backend ); }

			int seekJumpLength() const { return optionAsInt( keySeekJumpLength() ); } /// in seconds
			void setSeekJumpLength( int seconds ) { setOption( keySeekJumpLength(), seconds ); }

			bool showPositionTimeEdit() const { return optionAsBool( keyShowPositionTimeEdit() ); }
			void setShowPositionTimeEdit( bool show ) { setOption( keyShowPositionTimeEdit(), show ); }
			void toggleShowPositionTimeEdit() { setShowPositionTimeEdit( ! showPositionTimeEdit() ); }

			QString fontFamily() const { return option( keyFontFamily() ); }
			void setFontFamily( const QString& family ) { setOption( keyFontFamily(), family ); }

			int fontPointSize() const { return optionAsInt( keyFontPointSize() ); }
			void setFontPointSize( int pointSize ) { setOption( keyFontPointSize(), pointSize ); }
			void incFontPointSize( int delta ) { setFontPointSize( fontPointSize() + delta ); }

			QColor fontColor() const { return optionAsColor( keyFontColor() ); }
			void setFontColor( const QColor& color ) { setOption( keyFontColor(), color ); }

			QColor outlineColor() const { return optionAsColor( keyOutlineColor() ); }
			void setOutlineColor( const QColor& color ) { setOption( keyOutlineColor(), color ); }

			int outlineWidth() const { return optionAsInt( keyOutlineWidth() ); }
			void setOutlineWidth( int width ) { setOption( keyOutlineWidth(), width ); }


			static const QString& keyBackend() { static const QString key( "Backend" ); return key; }
			static const QString& keySeekJumpLength() { static const QString key( "SeekJumpLength" ); return key; }
			static const QString& keyShowPositionTimeEdit() { static const QString key( "ShowPositionTimeEdit" ); return key; }
			static const QString& keyFontFamily() { static const QString key( "FontFamily" ); return key; }
			static const QString& keyFontPointSize() { static const QString key( "FontPointSize" ); return key; }
			static const QString& keyFontColor() { static const QString key( "FontColor" ); return key; }
			static const QString& keyOutlineColor() { static const QString key( "OutlineColor" ); return key; }
			static const QString& keyOutlineWidth() { static const QString key( "OutlineWidth" ); return key; }

		private:

			PlayerConfig():AppConfigGroup( "Player", defaults() ) {}
			PlayerConfig( const PlayerConfig& config ):AppConfigGroup( config ) {}

			static QMap<QString,QString> defaults()
			{
				QMap<QString,QString> defaults;

#ifdef HAVE_GSTREAMER
				defaults[keyBackend()] = "GStreamer";
#else
				defaults[keyBackend()] = "MPlayer";
#endif
				defaults[keySeekJumpLength()] = "15"; // in seconds
				defaults[keyShowPositionTimeEdit()] = "false";

				defaults[keyFontFamily()] = "Sans Serif";
				defaults[keyFontPointSize()] = "15";
				defaults[keyFontColor()] = QColor( Qt::yellow ).name();
				defaults[keyOutlineColor()] = QColor( Qt::black ).name();
				defaults[keyOutlineWidth()] = "1";

				return defaults;
			}
	};
}

#endif
