#ifndef GSTREAMERBACKEND_H
#define GSTREAMERBACKEND_H

/***************************************************************************
 *   Copyright (C) 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)  *
 *   based on Kaffeine by Jürgen Kofler                                    *
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

#include "gstreamerconfig.h"
#include "../playerbackend.h"

#include <QtGui/QWidget>
#include <QtCore/QString>

class QTimer;
struct _GList;
struct _GstElement;
struct _GstBus;

namespace SubtitleComposer
{
	class GStreamerBackend : public PlayerBackend
	{
		Q_OBJECT

		public:

			GStreamerBackend( Player* player );
			virtual ~GStreamerBackend();

			const GStreamerConfig* const config() { return static_cast<const GStreamerConfig* const>( PlayerBackend::config() ); }

			virtual AppConfigGroupWidget* newAppConfigGroupWidget( QWidget* parent );

		protected:

			virtual VideoWidget* initialize( QWidget* videoWidgetParent );
			virtual void finalize();
			void _finalize();

			virtual bool openFile( const QString& filePath, bool& playingAfterCall );
			bool _openFile( const QString& filePath, bool play );
			virtual void closeFile();

			virtual bool play();
			virtual bool pause();
			virtual bool seek( double seconds, bool accurate );
			virtual bool stop();

			virtual bool setActiveAudioStream( int audioStream );

			virtual bool setVolume( double volume );

		protected:

			static void inspect( void* object );

			bool initializeGStreamer();
			bool createPlaybin();
			void deletePlaybin();

			bool changeState( int state, unsigned timeout=0 );

			_GList* streamInfoForType( const char* typestr );
			void updateVideoData();
			void updateAudioData();
			void hideSubtitles();

			void updatePosition();

		protected slots:

			void onBusTimerTimeout();

		protected:

			_GstElement* m_play;
			_GstElement* m_videoSink;
			_GstElement* m_audioSink;

			_GstBus* m_bus;
			QTimer* m_busTimer;

			bool m_updatePosition;
			quint64 m_length;

			bool m_firstPlayback;
	};
}

#endif
