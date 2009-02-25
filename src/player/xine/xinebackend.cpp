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

#include "xinebackend.h"
#include "xineconfigwidget.h"
#include "xinevideolayerwidget.h"

#include <QtCore/QEventLoop>
#include <QtCore/QEvent>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtGui/QApplication>

#include <KDebug>
#include <KLocale>
#include <KUrl>

#include <xine/xineutils.h>
#include <X11/Xlib.h>

using namespace SubtitleComposer;

#define EVENT_PLAYBACK_FINISHED		 QEvent::User + 1
#define EVENT_CHANNELS_CHANGED		 QEvent::User + 2
#define EVENT_FRAME_FORMAT_CHANGED	 QEvent::User + 3

#define UPDATE_INTERVAL 50

XineBackend::XineBackend( Player* player ):
	PlayerBackend( player, new XineConfig() ),
	m_connection( 0 ),
	m_xineEngine( 0 ),
	m_audioDriver( 0 ),
	m_videoDriver( 0 ),
	m_xineStream( 0 ),
	m_eventQueue( 0 ),
	m_updatePosition( true ),
	m_softwareMixer( false ),
	m_streamIsSeekable( false )
{
	connect( &m_timesTimer, SIGNAL(timeout()), this, SLOT(updatePosition()) );
}

XineBackend::~XineBackend()
{
	if ( isInitialized() )
		_finalize();
}

VideoWidget* XineBackend::initialize( QWidget* videoWidgetParent )
{
	XineVideoLayerWidget* videoLayer = new XineVideoLayerWidget( 0 );
	VideoWidget* videoWidget = new VideoWidget( videoLayer, videoWidgetParent );
	if ( ! initializeXine( videoWidget->videoLayer()->winId() ) )
	{
		delete videoWidget;
		finalizeXine();
		kError() << "xine initialization failed!";
		return 0;
	}

	videoLayer->setVideoDriver( m_videoDriver );
	connect( videoLayer, SIGNAL(geometryChanged()), this, SLOT(onVideoLayerGeometryChanged()) );

	return videoWidget;
}

void XineBackend::finalize()
{
	return _finalize();
}

void XineBackend::_finalize()
{
	m_timesTimer.stop();

	finalizeXine();
}

SubtitleComposer::AppConfigGroupWidget* XineBackend::newAppConfigGroupWidget( QWidget* parent )
{
	return new XineConfigWidget( parent );
}

bool XineBackend::openFile( const QString& filePath, bool& playingAfterCall )
{
	playingAfterCall = true;

	// the volume is adjusted when file playback starts and it's best if it's initially at 0
	xine_set_param( m_xineStream, m_softwareMixer ? XINE_PARAM_AUDIO_AMP_LEVEL : XINE_PARAM_AUDIO_VOLUME, 0 );

	m_streamIsSeekable = false;

	KUrl fileUrl;
	fileUrl.setProtocol( "file" );
	fileUrl.setPath( filePath );

	if ( ! xine_open( m_xineStream, fileUrl.url().toLocal8Bit() ) )
		return false;

	// no subtitles
	xine_set_param( m_xineStream, XINE_PARAM_SPU_CHANNEL, -1 );

	if ( ! xine_play( m_xineStream, 0, 0 ) )
		return false;

	m_player->updateState( Player::Playing );

	// this methods do nothing if the information is not available
	updateVideoData();
	updateAudioData();
	updatePosition();

	m_timesTimer.start( UPDATE_INTERVAL );

	return true;
}

void XineBackend::closeFile()
{
}

bool XineBackend::play()
{
	if ( xine_get_status( m_xineStream ) != XINE_STATUS_PLAY )
		xine_play( m_xineStream, 0, 0 ); // was stopped
	else
		xine_set_param( m_xineStream, XINE_PARAM_SPEED, XINE_SPEED_NORMAL ); // was paused

	m_player->updateState( Player::Playing );

	m_timesTimer.start( UPDATE_INTERVAL );

	return true;
}

bool XineBackend::pause()
{
	m_timesTimer.stop();

	xine_set_param( m_xineStream, XINE_PARAM_SPEED, XINE_SPEED_PAUSE );
	m_player->updateState( Player::Paused );

	return true;
}

bool XineBackend::seek( double seconds, bool accurate )
{
	if ( m_streamIsSeekable )
	{
		int targetTime = (int)(seconds*1000+0.5);

		if ( m_player->isPaused() )
			xine_set_param( m_xineStream, m_softwareMixer ? XINE_PARAM_AUDIO_AMP_MUTE : XINE_PARAM_AUDIO_MUTE, 1 );

		xine_play( m_xineStream, 0, targetTime );

		if ( accurate )
		{
			int time;

			if ( ! xine_get_pos_length( m_xineStream, 0, &time, 0 ) )
				return true;

			m_updatePosition = false;

			xine_set_param( m_xineStream, XINE_PARAM_SPEED, XINE_SPEED_FAST_4 );

			while ( targetTime - time > 200 /*|| time - targetTime > 100 ||*/ )
			{
				QApplication::processEvents( QEventLoop::ExcludeUserInputEvents );

				if ( ! xine_get_pos_length( m_xineStream, 0, &time, 0 ) )
					break;
			}

			if ( ! m_player->isPaused() )
				xine_set_param( m_xineStream, XINE_PARAM_SPEED, XINE_SPEED_NORMAL );

			m_updatePosition = true;
		}

		if ( m_player->isPaused() )
		{
			xine_set_param( m_xineStream, XINE_PARAM_SPEED, XINE_SPEED_PAUSE );
			xine_set_param( m_xineStream, m_softwareMixer ? XINE_PARAM_AUDIO_AMP_MUTE : XINE_PARAM_AUDIO_MUTE, 0 );
		}
	}

	return true;
}

bool XineBackend::stop()
{
	m_timesTimer.stop();

	xine_stop( m_xineStream );
	m_player->updateState( Player::Ready );

	return true;
}

bool XineBackend::setActiveAudioStream( int audioStream )
{
	xine_set_param( m_xineStream, XINE_PARAM_AUDIO_CHANNEL_LOGICAL, audioStream );

	return true;
}

bool XineBackend::setVolume( double volume )
{
	xine_set_param( m_xineStream, m_softwareMixer ? XINE_PARAM_AUDIO_AMP_LEVEL : XINE_PARAM_AUDIO_VOLUME, (int)(volume+0.5) );

	return true;
}


void XineBackend::updateVideoData()
{
	if ( ! m_xineStream )
		return;

	if ( xine_get_stream_info( m_xineStream, XINE_STREAM_INFO_HAS_VIDEO ) )
	{
		int fps = xine_get_stream_info( m_xineStream, XINE_STREAM_INFO_FRAME_DURATION );
		if ( fps > 0 )
		{
			m_player->updateFramesPerSecond( 90000.0 / fps );

			// tweak prebuffer so we can be sure to show only a single frame
			// kDebug() << "PREBUFFER " << xine_get_param( m_xineStream, XINE_PARAM_METRONOM_PREBUFFER );
			xine_set_param( m_xineStream, XINE_PARAM_METRONOM_PREBUFFER, fps );
			// kDebug() << "PREBUFFER " << xine_get_param( m_xineStream, XINE_PARAM_METRONOM_PREBUFFER );
		}

		int width = xine_get_stream_info( m_xineStream, XINE_STREAM_INFO_VIDEO_WIDTH );
		int height = xine_get_stream_info( m_xineStream, XINE_STREAM_INFO_VIDEO_HEIGHT );
		double dar = xine_get_stream_info( m_xineStream, XINE_STREAM_INFO_VIDEO_RATIO ) / 10000.0;
		m_player->videoWidget()->setVideoResolution( width, height, dar );
	}
	else
		m_player->videoWidget()->setVideoResolution( 0, 0 );

	m_streamIsSeekable = xine_get_stream_info( m_xineStream, XINE_STREAM_INFO_SEEKABLE );

	QSize size = m_player->videoWidget()->videoLayer()->size();
	QPoint globalPos = m_player->videoWidget()->videoLayer()->mapToGlobal( QPoint( 0, 0 ) );
	m_videoLayerGeometry = QRect( globalPos.x(), globalPos.y(), size.width(), size.height() );
}

void XineBackend::updateAudioData()
{
	if ( ! m_xineStream )
		return;

	QStringList audioStreams;

	int channels = xine_get_stream_info( m_xineStream, XINE_STREAM_INFO_MAX_AUDIO_CHANNEL );
	for( int index = 0; index < channels; ++index )
	{
		QString audioStreamName = i18n( "Audio Stream #%1", index + 1 );
		char lang[128];
		if ( xine_get_audio_lang( m_xineStream, index, lang ) )
			audioStreamName += QString( " - " ) + lang;
		audioStreams << audioStreamName;
	}

	m_player->updateAudioStreams( audioStreams, audioStreams.isEmpty() ? -1 : 0 );
}

void XineBackend::updateLengthData()
{
	if ( ! m_xineStream )
		return;

	int time, length;
	if ( xine_get_pos_length( m_xineStream, 0, &time, &length ) )
	{
		m_player->updateLength( length / 1000.0 );
	}
}

void XineBackend::updatePosition()
{
	if ( ! m_xineStream || ! m_updatePosition )
		return;

	// some streams make xine sometimes report spurious position data during playback
	// to compensate this we check if the received position is not too far away from the
	// previously received one (not further than 200ms).

	static int prevTime;

	static int time, length;
	if ( xine_get_pos_length( m_xineStream, 0, &time, &length ) )
	{
		if ( time < prevTime + 200 || time < prevTime )
			m_player->updatePosition( time / 1000.0 );

		prevTime = time;
	}
}

void XineBackend::onVideoLayerGeometryChanged()
{
	QSize size = m_player->videoWidget()->videoLayer()->size();
	QPoint globalPos = m_player->videoWidget()->videoLayer()->mapToGlobal( QPoint( 0, 0 ) );
	m_videoLayerGeometry = QRect( globalPos.x(), globalPos.y(), size.width(), size.height() );
}

void XineBackend::xineEventListener( void* p, const xine_event_t* event )
{
	if ( p == NULL )
		return;

	XineBackend* xinePlayer = (XineBackend*)p;

	switch ( event->type )
	{
		case XINE_EVENT_UI_PLAYBACK_FINISHED:
			QApplication::postEvent( xinePlayer, new QEvent( (QEvent::Type)(EVENT_PLAYBACK_FINISHED) ) );
			break;

		case XINE_EVENT_UI_CHANNELS_CHANGED:
			QApplication::postEvent( xinePlayer, new QEvent( (QEvent::Type)(EVENT_CHANNELS_CHANGED) ) );
			break;

		case XINE_EVENT_FRAME_FORMAT_CHANGE:
			QApplication::postEvent( xinePlayer, new QEvent( (QEvent::Type)(EVENT_FRAME_FORMAT_CHANGED) ) );
			break;

		default:
			break;
	}
}

void XineBackend::customEvent( QEvent* event )
{
	switch ( event->type() )
	{
		case EVENT_PLAYBACK_FINISHED:
			stop();
			break;

		case EVENT_CHANNELS_CHANGED:
			updateAudioData();
			updateLengthData();
			break;

		case EVENT_FRAME_FORMAT_CHANGED:
			//updateLengthData();
			updateVideoData();
			break;

		default:
			break;
	}
}

bool XineBackend::initializeXine( WId winId )
{
#ifdef HAVE_XCB
	int screen_nbr = 0;
	m_connection = xcb_connect( NULL, &screen_nbr );
#else
	XInitThreads();
	m_connection = XOpenDisplay( NULL );
#endif

	if ( ! m_connection )
	{
		kDebug() << "Failed to connect to X-Server!";
		return false;
	}

	if ( ! (m_xineEngine = xine_new()) )
	{
		kDebug() << "Couldn't init xine Engine!";
		return false;
	}

	QString configFilePath( QDir::homePath() + "/.xine/config" );
	if ( QFile::exists( configFilePath ) )
		xine_config_load( m_xineEngine, QFile::encodeName( configFilePath ) );

	xine_init( m_xineEngine );

	m_softwareMixer = (bool)xine_config_register_bool(
		m_xineEngine,
		"audio.mixer_software",
		1, NULL, NULL, 10,
		&XineBackend::audioMixerMethodChangedCallback,
		this
	);

#ifdef HAVE_XCB
	xcb_screen_iterator_t screen_it = xcb_setup_roots_iterator( xcb_get_setup( m_connection ) );
	while ( screen_it.rem > 1 && screen_nbr > 0 )
	{
		xcb_screen_next(&screen_it);
		--screen_nbr;
	}
	m_x11Visual.connection = m_connection;
	m_x11Visual.screen = screen_it.data;
	m_x11Visual.window = winId;
#else
	m_x11Visual.display = m_connection;
	m_x11Visual.screen = DefaultScreen( m_connection );
	m_x11Visual.d = winId;
#endif
// 	m_x11Visual.dest_size_cb = &XineBackend::destSizeCallback;
	m_x11Visual.frame_output_cb = &XineBackend::frameOutputCallback;
	m_x11Visual.user_data = (void*)this;


	QStringList videoDriverNames = QString( "xv xvmc opengl xxmc sdl xshm fb XDirectFB DirectFB aa caca auto" ).split( ' ' );
	videoDriverNames.prepend( config()->videoDriver() );
	for ( QStringList::Iterator it = videoDriverNames.begin(), end = videoDriverNames.end(); it != end; ++it )
	{
		if ( (*it).isEmpty() )
			continue;

		// NOTE: magical fix follows... (taken from Phonon Xine backend)
		// make sure all Qt<->X communication is done, else xine_open_video_driver will crash
		QApplication::syncX();

		m_videoDriver = xine_open_video_driver(
			m_xineEngine,
			(*it).toAscii(),
#ifdef HAVE_XCB
			XINE_VISUAL_TYPE_XCB,
#else
			XINE_VISUAL_TYPE_X11,
#endif
			(void*)&(m_x11Visual)
		);

		if ( m_videoDriver )
			break;
	}

	if ( ! m_videoDriver )
	{
		kDebug() << "All video drivers failed to initialize!";
		return false;
	}


	QStringList audioDriverNames = QString( "alsa oss jack pulseaudio esd auto" ).split( ' ' );
	audioDriverNames.prepend( config()->audioDriver() );
	for ( QStringList::Iterator it = audioDriverNames.begin(), end = audioDriverNames.end(); it != end; ++it )
		if ( ! (*it).isEmpty() && (m_audioDriver = xine_open_audio_driver( m_xineEngine, (*it).toAscii(), NULL )) != NULL )
			break;

	if ( ! m_audioDriver )
	{
		kDebug() << "All audio drivers failed to initialize!";
		return false;
	}

	m_xineStream = xine_stream_new(
		m_xineEngine,
		m_audioDriver,
		m_videoDriver
	);

	if ( ! m_xineStream )
	{
		kDebug() << "Couldn't create a new xine stream!";
		return false;
	}

	m_eventQueue = xine_event_new_queue( m_xineStream );
	xine_event_create_listener_thread( m_eventQueue, &XineBackend::xineEventListener, (void*)this );

	return true;
}

void XineBackend::finalizeXine()
{
	if ( m_xineStream )
		xine_close( m_xineStream );

	if ( m_eventQueue )
	{
		xine_event_dispose_queue( m_eventQueue );
		m_eventQueue = 0;
	}

	if ( m_xineStream )
	{
		xine_dispose( m_xineStream );
		m_xineStream = 0;
	}

	if ( m_audioDriver )
	{
		xine_close_audio_driver( m_xineEngine, m_audioDriver );
		m_audioDriver = 0;
	}

	if ( m_videoDriver )
	{
		xine_close_video_driver( m_xineEngine, m_videoDriver );
		m_videoDriver = 0;
	}

	if ( m_xineEngine )
	{
		xine_exit( m_xineEngine );
		m_xineEngine = 0;
	}

	if ( m_connection )
	{
#ifdef HAVE_XCB
		xcb_disconnect( m_connection );
#else
		XCloseDisplay( m_connection );
#endif
		m_connection = NULL;
	}
}

void XineBackend::destSizeCallback(	void* p, int /*video_width*/, int /*video_height*/, double /*video_aspect*/,
											int* dest_width, int* dest_height, double* dest_aspect )
{
	if ( p == NULL )
		return;

	XineBackend* xinePlayer = (XineBackend*)p;
	*dest_width = xinePlayer->m_videoLayerGeometry.width();
	*dest_height = xinePlayer->m_videoLayerGeometry.height();
	*dest_aspect = 1.0;
}

void XineBackend::frameOutputCallback(	void* p, int /*video_width*/, int /*video_height*/, double /*video_aspect*/,
												int* dest_x, int* dest_y, int* dest_width, int* dest_height,
												double* dest_aspect, int* win_x, int* win_y )
{
	if ( p == NULL )
		 return;

	XineBackend* xinePlayer = (XineBackend*)p;

	*dest_x = 0;
	*dest_y = 0 ;
	*dest_width = xinePlayer->m_videoLayerGeometry.width();
	*dest_height = xinePlayer->m_videoLayerGeometry.height();
	*dest_aspect = 1.0;
	*win_x = xinePlayer->m_videoLayerGeometry.x();
	*win_y = xinePlayer->m_videoLayerGeometry.y();
}

void XineBackend::audioMixerMethodChangedCallback( void* p, xine_cfg_entry_t* entry )
{
	if ( p == NULL )
		return;

	XineBackend* xinePlayer = (XineBackend*)p;
	xinePlayer->m_softwareMixer = (bool)entry->num_value;
}

#include "xinebackend.moc"
