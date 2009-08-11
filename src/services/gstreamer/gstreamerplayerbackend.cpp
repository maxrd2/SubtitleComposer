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

#include "gstreamerplayerbackend.h"
#include "gstreamerconfigwidget.h"

#include <QtCore/QTimer>

#include <KDebug>
#include <KLocale>
#include <KUrl>

#include <gst/gst.h>
#include <gst/interfaces/xoverlay.h>

using namespace SubtitleComposer;

GStreamerPlayerBackend::GStreamerPlayerBackend( Player* player ):
	PlayerBackend( player, new GStreamerConfig() ),
	m_play( NULL ),
	m_videoSink( NULL ),
	m_audioSink( NULL ),
	m_bus( NULL ),
	m_busTimer( new QTimer( this ) ),
	m_updatePosition( false ),
	m_length( GST_CLOCK_TIME_NONE )
{
	connect( m_busTimer, SIGNAL(timeout()), this, SLOT(onBusTimerTimeout()) );
}

GStreamerPlayerBackend::~GStreamerPlayerBackend()
{
	if ( isInitialized() )
		_finalize();
}

VideoWidget* GStreamerPlayerBackend::initialize( QWidget* videoWidgetParent )
{
	if ( ! initializeGStreamer() )
	{
		kError() << "GStreamer initialization failed!";
		return 0;
	}

	return new VideoWidget( videoWidgetParent );
}

void GStreamerPlayerBackend::finalize()
{
	return _finalize();
}

void GStreamerPlayerBackend::_finalize()
{
	if ( GST_IS_X_OVERLAY( m_videoSink ) )
	{
		gst_x_overlay_set_xwindow_id( GST_X_OVERLAY( m_videoSink ), 0 );
		//gst_x_overlay_expose( GST_X_OVERLAY( m_videoSink ) );
	}

	deletePlaybin();
}

SubtitleComposer::AppConfigGroupWidget* GStreamerPlayerBackend::newAppConfigGroupWidget( QWidget* parent )
{
	return new GStreamerConfigWidget( parent );
}

bool GStreamerPlayerBackend::openFile( const QString& filePath, bool& playingAfterCall )
{
	playingAfterCall = true;

	m_firstPlayback = true;

	return _openFile( filePath, true );
}

bool GStreamerPlayerBackend::_openFile( const QString& filePath, bool play )
{
	if ( ! m_play && ! createPlaybin() )
		return false;

	changeState( GST_STATE_READY, 1000 );

	KUrl fileUrl;
	fileUrl.setProtocol( "file" );
	fileUrl.setPath( filePath );

	gchar* uri = g_strdup( fileUrl.url().toLocal8Bit() );
	g_object_set( G_OBJECT( m_play ), "uri", uri, NULL );
	g_free( uri );

	g_object_set( G_OBJECT( m_play ), "suburi", NULL, NULL );

	m_length = GST_CLOCK_TIME_NONE;


	if ( play ) // needed when first opening file to get stream data
	{
		// the volume is adjusted when file playback starts and it's best if it's initially at 0
		g_object_set( G_OBJECT( m_play ), "volume", (gdouble)0.0, NULL );

		changeState( GST_STATE_PLAYING, 0 /*don't block waiting*/ );
	}
	else
		changeState( GST_STATE_READY, 0 /*don't block waiting*/ );

	return true;
}

void GStreamerPlayerBackend::closeFile()
{
	changeState( GST_STATE_NULL, 0 ); // flushes all buffers
}

bool GStreamerPlayerBackend::play()
{
	if ( ! m_play )
		return false;

	changeState( GST_STATE_PLAYING, 0 /*don't block waiting*/ );

	return true;
}

bool GStreamerPlayerBackend::pause()
{
	if ( ! m_play )
		return false;

	changeState( GST_STATE_PAUSED, 0 /*don't block waiting*/ );
	return true;
}

bool GStreamerPlayerBackend::seek( double seconds, bool accurate )
{
	if ( ! m_play )
		return false;

	gst_element_seek_simple(
		m_play,
		GST_FORMAT_TIME, // time in nanoseconds
		(GstSeekFlags)(GST_SEEK_FLAG_FLUSH|(accurate?GST_SEEK_FLAG_ACCURATE:GST_SEEK_FLAG_KEY_UNIT)),
		(gint64)(seconds*GST_SECOND)
	);

	return true;
}

bool GStreamerPlayerBackend::stop()
{
	if ( ! m_play )
		return false;

// 	changeState( GST_STATE_NULL, 3000 ); // flushes all buffers
	changeState( GST_STATE_READY, 1000 );

	return true;
}

bool GStreamerPlayerBackend::setActiveAudioStream( int audioStream )
{
	if ( ! m_play )
		return false;

	g_object_set( G_OBJECT( m_play ), "current-audio", (gint)audioStream, NULL );

	return true;
}

bool GStreamerPlayerBackend::setVolume( double volume )
{
	if ( ! m_play )
		return false;

	g_object_set( G_OBJECT( m_play ), "volume", (gdouble)(volume * 0.01), NULL );

	return true;
}

bool GStreamerPlayerBackend::initializeGStreamer()
{
	if ( ! gst_init_check( NULL, NULL, NULL ) )
		return false;

	// Init audio driver

	QStringList audioSinkNames = QString( "alsasink osssink gconfaudiosink artsdsink autoaudiosink" ).split( ' ' );
	if ( config()->hasAudioSink() )
	{
		audioSinkNames.removeAll( config()->audioSink() );
		audioSinkNames.prepend( config()->audioSink() );
	}

	for ( QStringList::ConstIterator it = audioSinkNames.begin(), end = audioSinkNames.end(); it != end; ++it )
		if ( ! (*it).isEmpty() && (m_audioSink = gst_element_factory_make( (*it).toAscii(), "audiosink" )) != NULL )
			break;

	if ( m_audioSink == NULL )
		return false;
	else
		gst_element_set_state( m_audioSink, GST_STATE_READY );


	// Init video driver

	QStringList videoSinkNames = QString( "xvimagesink ximagesink gconfvideosink autovideosink" ).split( ' ' );
	if ( config()->hasVideoSink() )
	{
		videoSinkNames.removeAll( config()->videoSink() );
		videoSinkNames.prepend( config()->videoSink() );
	}

	for ( QStringList::ConstIterator it = videoSinkNames.begin(), end = videoSinkNames.end(); it != end; ++it )
		if ( ! (*it).isEmpty() && (m_videoSink = gst_element_factory_make( (*it).toAscii(), "videosink" )) != NULL )
			break;

	if ( m_videoSink == NULL )
		return false;
	else
		gst_element_set_state( m_videoSink, GST_STATE_READY );

	return true;
}

bool GStreamerPlayerBackend::createPlaybin()
{
	m_play = gst_element_factory_make( "playbin", "play" );

	if ( ! m_play )
		return false;

	if ( ! m_videoSink || ! m_audioSink )
	{
		gst_object_unref( GST_OBJECT( m_play ) );
		return false;
	}

	g_object_set( G_OBJECT( m_play ), "video-sink", m_videoSink, NULL );
	g_object_set( G_OBJECT( m_play ), "audio-sink", m_audioSink, NULL );
	gst_element_set_state( m_play, GST_STATE_READY );

	m_bus = gst_pipeline_get_bus( GST_PIPELINE( m_play ) );

	m_busTimer->start( 20 );

	return true;
}

void GStreamerPlayerBackend::deletePlaybin()
{
	if ( m_bus )
	{
		m_busTimer->stop();
		gst_object_unref( GST_OBJECT( m_bus ) );
		m_bus = NULL;
	}

	if ( m_play )
	{
		gst_element_set_state( m_play, GST_STATE_NULL );
		gst_object_unref( GST_OBJECT( m_play ) );
		m_play = NULL;
	}
}

bool GStreamerPlayerBackend::changeState( int state, unsigned timeout )
{
	if ( GST_STATE( m_play ) == state )
		return true;

	if ( m_videoSink && GST_IS_X_OVERLAY( m_videoSink ) )
	{
		gst_x_overlay_set_xwindow_id( GST_X_OVERLAY( m_videoSink ), player()->videoWidget()->videoLayer()->winId() );
		gst_x_overlay_expose( GST_X_OVERLAY( m_videoSink ) );
	}

	GstStateChangeReturn ret = gst_element_set_state( m_play, (GstState)state );
	if ( ret == GST_STATE_CHANGE_SUCCESS )
		return true;

	if ( ret != GST_STATE_CHANGE_ASYNC )
		return false;

	if ( ! timeout )
		return true;

	// wait for state change or timeout
	return gst_element_get_state( m_play, NULL, NULL, timeout*GST_MSECOND ) == GST_STATE_CHANGE_SUCCESS;
}

void GStreamerPlayerBackend::updatePosition()
{
	if ( ! m_play )
		return;

	gint64 time;
	GstFormat fmt = GST_FORMAT_TIME;

	if ( ! GST_CLOCK_TIME_IS_VALID ( m_length ) )
	{
		if ( gst_element_query_duration( m_play, &fmt, &time ) )
		{
			m_length = time;
			player()->updateLength( (double)time / GST_SECOND );
		}
	}

	if ( gst_element_query_position( m_play, &fmt, &time ) )
		player()->updatePosition( ((double)time / GST_SECOND) );
}

void GStreamerPlayerBackend::onBusTimerTimeout()
{
	if ( ! isInitialized() )
		return;

	if ( m_updatePosition )
		updatePosition();

	if ( ! m_bus )
		return;

	GstMessage* msg;
	while ( (msg = gst_bus_pop( m_bus )) != 0 )
	{
		GstElement* src = (GstElement*)GST_MESSAGE_SRC( msg );

		switch( GST_MESSAGE_TYPE( msg ) )
		{
			case GST_MESSAGE_TAG:
				// TODO do stream metadata parsing here
				break;

			case GST_MESSAGE_STATE_CHANGED:
			{
				if ( src != m_play )
					break;

				GstState old, cur, pending;
				gst_message_parse_state_changed( msg, &old, &cur, &pending );
				if ( cur == old )
					break;

				if ( cur == GST_STATE_PAUSED )
				{
					m_updatePosition = false;
					player()->updateState( Player::Paused );
				}
				else if ( cur == GST_STATE_PLAYING )
				{
					m_updatePosition = true;
					player()->updateState( Player::Playing );

					if ( m_firstPlayback )
					{
						m_firstPlayback = false;
						hideSubtitles();
					}
				}
				else if ( cur == GST_STATE_READY )
				{
					m_updatePosition = false;
					player()->updateState( Player::Ready );
				}

				if ( old == GST_STATE_READY )
				{
					updateAudioData();
					updateVideoData();
				}

				break;
			}
			case GST_MESSAGE_SEGMENT_DONE:
			{
				GstFormat format = GST_FORMAT_TIME;
				gint64 position;
				gst_message_parse_segment_done( msg, &format, &position );
				player()->updatePosition( (double)position / GST_SECOND );
			}
			case GST_MESSAGE_DURATION:
			{
				GstFormat format = GST_FORMAT_TIME;
				gint64 duration;
				gst_message_parse_duration( msg, &format, &duration );
				m_length = duration;
				player()->updateLength( (double)duration / GST_SECOND );
			}
			case GST_MESSAGE_EOS:
			{
				m_updatePosition = false;
				player()->updateState( Player::Ready );
				_openFile( player()->filePath(), false );
				return;
			}
			case GST_MESSAGE_ERROR:
			{
				gchar* debug = NULL;
				GError* error = NULL;

				gst_message_parse_error( msg, &error, &debug );
				kDebug() << "GST_MESSAGE_ERROR:" << QString( error->message ) << QString( debug );
				g_error_free( error );
				g_free( debug );
				gst_element_set_state( m_play, GST_STATE_NULL );
				break;
			}

			default:
				break;
		}

		gst_message_unref( msg );
	}
}


GList* GStreamerPlayerBackend::streamInfoForType( const char* type /*"AUDIO", "TEXT", "SUBPICTURE" or "VIDEO"*/ )
{
	if ( m_play == NULL )
		return NULL;

	GValueArray* infoArray = NULL;
	g_object_get( G_OBJECT( m_play ), "stream-info-value-array", &infoArray, NULL );
	if ( infoArray == NULL )
		return NULL;

	GList* ret = NULL;
	for ( guint idx = 0; idx < infoArray->n_values; ++idx )
	{
		GObject* info = (GObject*)g_value_get_object( g_value_array_get_nth( infoArray, idx ) );
		if ( info )
		{
			gint typeID = -1;
			g_object_get( G_OBJECT( info ), "type", &typeID, NULL );
			GParamSpec* pspec = g_object_class_find_property( G_OBJECT_GET_CLASS( info ), "type" );
			GEnumValue* val = g_enum_get_value( G_PARAM_SPEC_ENUM( pspec )->enum_class, typeID );
			if ( val )
			{
				if( g_ascii_strcasecmp( val->value_nick, type ) == 0 || g_ascii_strcasecmp( val->value_name, type ) == 0 )
					ret = g_list_prepend( ret, g_object_ref( info ) );
			}
		}
	}
	g_value_array_free( infoArray );

	return g_list_reverse( ret );
}

void GStreamerPlayerBackend::updateVideoData()
{
	if ( ! m_play )
		return;

	GList* list = streamInfoForType( "VIDEO" );
	if ( list == NULL )
		return;

	for ( GList* l = list; l != NULL; l = l->next )
	{
		GstPad* pad = NULL;
		g_object_get( l->data, "object", &pad, NULL );

		GstCaps* caps = gst_pad_get_negotiated_caps( pad );
		if ( ! caps )
			continue;

		const GstStructure* capsStruct = gst_caps_get_structure( caps, 0 );
		if ( ! capsStruct )
			continue;

		int width = 0, height = 0;
		gst_structure_get_int( capsStruct, "width", &width );
		gst_structure_get_int( capsStruct, "height", &height );

		double dar = 0.0;
		const GValue* par;
		if ( (par = gst_structure_get_value( capsStruct, "pixel-aspect-ratio" )) )
		{
			dar = (double)gst_value_get_fraction_numerator( par ) / gst_value_get_fraction_denominator( par );
			dar = dar * width / height;
		}

		player()->videoWidget()->setVideoResolution( width, height, dar );

		const GValue* fps;
		if ( (fps = gst_structure_get_value( capsStruct, "framerate" )) )
		{
			int num = gst_value_get_fraction_numerator( fps );
			int den = gst_value_get_fraction_denominator( fps );
			player()->updateFramesPerSecond( (double)num/den );
		}

		gst_caps_unref( caps );
		return;
	}
}

void GStreamerPlayerBackend::updateAudioData()
{
	QStringList audioStreams;

	GList* list = streamInfoForType( "AUDIO" );
	if ( list == NULL )
		return;

	gint trackNumber = 1;
	for ( GList* l = list; l != NULL; l = l->next, ++trackNumber )
	{
		gchar* languageCode = NULL;
		gchar* codec = NULL;
		g_object_get( l->data, "codec", &codec, "language-code", &languageCode, NULL );

		QString audioStreamName;
		if ( languageCode )
		{
			audioStreamName = QString( languageCode );
			g_free( languageCode );
		}
		if ( codec )
		{
			if ( ! audioStreamName.isEmpty() )
				audioStreamName += " / ";
			audioStreamName += QString( codec );
			g_free( codec );
		}
		if ( audioStreamName.isEmpty() )
			audioStreamName = i18n( "Audio Stream #%1", trackNumber );
		audioStreams << audioStreamName;
	}

	g_list_foreach( list, (GFunc)g_object_unref, NULL ); // krazy:exclude=c++/foreach
	g_list_free( list );

	// NOTE this value is incorrectly reported by GStreamer. It seems to report the
	// last value set on the property but when video restarts playing (after a stop)
	// the real audio stream playing is always the first stream found (BTW, the order
	// of the streams found can also change after a stop).
	gint activeAudioStream = audioStreams.isEmpty() ? -1 : 0;
	// gint activeAudioStream = -1;
	// g_object_get( G_OBJECT( m_play ), "current-audio", &activeAudioStream, NULL );

	player()->updateAudioStreams( audioStreams, activeAudioStream );
}

void GStreamerPlayerBackend::hideSubtitles()
{
	// NOTE hiding embedded subtitles works unreliably on GStreamer ATM.
	// (it makes playback stutter or halt). We worked around that by making
	// the font as smalls as possible instead:
	g_object_set( G_OBJECT( m_play ), "subtitle-font-desc", "Sans 0", NULL );

/*	GList* list = streamInfoForType( "SUBPICTURE" );
	if ( list != NULL )
	{
		g_object_set( G_OBJECT( m_play ), "current-subpicture", (gint)-1, NULL );
		g_list_foreach( list, (GFunc)g_object_unref, NULL ); // krazy:exclude=c++/foreach
		g_list_free (list);
	}

	list = streamInfoForType( "TEXT" );
	if ( list != NULL )
	{
		g_object_set( G_OBJECT( m_play ), "current-text", (gint)-1, NULL );
		g_list_foreach( list, (GFunc)g_object_unref, NULL ); // krazy:exclude=c++/foreach
		g_list_free (list);
	}*/
}

#include "gstreamerplayerbackend.moc"
