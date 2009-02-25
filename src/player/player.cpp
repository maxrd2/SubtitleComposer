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

#include "player.h"

#ifdef HAVE_GSTREAMER
#include "gstreamer/gstreamerbackend.h"
#endif
#include "mplayer/mplayerbackend.h"
#include "phonon/phononbackend.h"
#ifdef HAVE_XINE
#include "xine/xinebackend.h"
#endif

#include <math.h>

#include <QtCore/QTimer>
#include <QtCore/QFileInfo>

#include <KDebug>

#define DEFAULT_MIN_POSITION_DELTA 0.02

using namespace SubtitleComposer;

Player::Player():
	m_activeBackend( 0 ),
	m_videoWidget( 0 ),
	m_filePath(),
	m_state( Player::Uninitialized ),
	m_position( -1.0 ),
	m_savedPosition( -1.0 ),
	m_length( -1.0 ),
	m_framesPerSecond( -1.0 ),
	m_minPositionDelta( DEFAULT_MIN_POSITION_DELTA ),
	m_activeAudioStream( -1 ),
	m_audioStreams(),
	m_muted( false ),
	m_volume( 100.0 ),
	m_backendVolume( 100.0 ),
	m_openFileTimer( new QTimer( this ) ),
	m_applicationClosingDown( false )
{
#ifdef HAVE_GSTREAMER
	m_backendsMap["GStreamer"] = new GStreamerBackend( this );
#endif

	m_backendsMap["MPlayer"] = new MPlayerBackend( this );
	m_backendsMap["Phonon"] = new PhononBackend( this );

#ifdef HAVE_XINE
	m_backendsMap["Xine"] = new XineBackend( this );
#endif

	// the timeout might seem too much, but it only matters when the file couldn't be
	// opened, and it's better to have the user wait a bit in that case than showing
	// an error we there's nothing wrong with the file (a longer time might be needed
	// for example if the computer is under heavy load or is just slow)

	m_openFileTimer->setSingleShot( true );

	connect( m_openFileTimer, SIGNAL( timeout() ), this, SLOT( onOpenFileTimeout() ) );
}

Player::~Player()
{
}

Player* const Player::instance()
{
	static Player player;

	return &player;
}

double Player::logarithmicVolume( double volume )
{
	static const double base = 4.0;
	static const double power = 1.0;
	static const double divisor = pow( base, power );

	double scaledVol = volume*power*power/100.0;
	double factor = pow( base, scaledVol/power );

	return (scaledVol*factor/divisor)*(100.0/(power*power));
}

const QStringList& Player::audioStreams() const
{
	static const QStringList emptyList;

	return m_state <= Player::Opening ? emptyList : m_audioStreams;
}

bool Player::initializeBackend( PlayerBackend* backend, QWidget* videoWidgetParent )
{
	if ( m_activeBackend == backend )
		return true;

	if ( m_activeBackend )
		return false;

	if ( (m_videoWidget = backend->initialize( videoWidgetParent )) )
	{
		m_state = Player::Closed;

		connect( m_videoWidget, SIGNAL( destroyed() ), this, SLOT( onVideoWidgetDestroyed() ) );
		connect( m_videoWidget, SIGNAL( doubleClicked( const QPoint& ) ), this, SIGNAL( doubleClicked( const QPoint& ) ) );
		connect( m_videoWidget, SIGNAL( leftClicked( const QPoint& ) ), this, SIGNAL( leftClicked( const QPoint& ) ) );
		connect( m_videoWidget, SIGNAL( rightClicked( const QPoint& ) ), this, SIGNAL( rightClicked( const QPoint& ) ) );
		connect( m_videoWidget, SIGNAL( wheelUp() ), this, SIGNAL( wheelUp() ) );
		connect( m_videoWidget, SIGNAL( wheelDown() ), this, SIGNAL( wheelDown() ) );

		m_videoWidget->show();

		// NOTE: next is used to make videoWidgetParent update it's geometry
		QRect geometry = videoWidgetParent->geometry();
		geometry.setHeight( geometry.height() + 1 );
		videoWidgetParent->setGeometry( geometry );

		m_activeBackend = backend;
		emit backendInitialized( backend );
	}

	return m_activeBackend == backend;
}

PlayerBackend* Player::finalizeBackend()
{
	if ( m_state == Player::Uninitialized )
		return 0;

	closeFile();
	m_activeBackend->finalize();

	m_state = Player::Uninitialized;

	PlayerBackend* activeBackend = m_activeBackend;
	m_activeBackend = 0;

	if ( m_videoWidget )
	{
		m_videoWidget->disconnect();
		m_videoWidget->hide();
		delete m_videoWidget;
		m_videoWidget = 0;
	}

	emit backendFinalized( activeBackend );
	return activeBackend;
}

PlayerBackend* Player::reinitializeBackend()
{
	if ( m_state == Player::Uninitialized )
		return 0;

	PlayerBackend* activeBackend = m_activeBackend;
	QWidget* videoWidgetParent = m_videoWidget->parentWidget();

	finalizeBackend();
	if ( ! initializeBackend( activeBackend, videoWidgetParent ) )
	{
		for ( QMap<QString,PlayerBackend*>::ConstIterator it = m_backendsMap.begin(), end = m_backendsMap.end(); it != end; ++it )
			if ( initializeBackend( it.value(), videoWidgetParent ) )
				break;
	}

	return m_activeBackend;
}

void Player::onVideoWidgetDestroyed()
{
	Q_ASSERT( m_state == Player::Uninitialized );

	m_videoWidget = 0;

	finalizeBackend();
}

QString Player::activeBackendName()
{
	for ( QMap<QString,PlayerBackend*>::ConstIterator it = m_backendsMap.begin(), end = m_backendsMap.end(); it != end; ++it )
		if ( it.value() == m_activeBackend )
			return it.key();
	return QString();
}

QStringList Player::backendNames()
{
	return m_backendsMap.keys();
}

PlayerBackend* Player::backend( const QString& backendName )
{
	return m_backendsMap.contains( backendName ) ? m_backendsMap[backendName] : 0;
}

PlayerBackend* Player::activeBackend()
{
	return m_activeBackend;
}

PlayerBackend* Player::setActiveBackend( const QString& backendName )
{
	if ( m_state == Player::Uninitialized )
		return 0;

	QString activeBackendName = this->activeBackendName();

	if ( backendName == activeBackendName )
		return m_activeBackend;

	if ( ! m_backendsMap.contains( backendName ) )
	{
		kDebug() << "Attempted to initialize unknonw backend:" << backendName;
		return m_activeBackend;
	}

	QWidget* videoWidgetParent = m_videoWidget->parentWidget();

	finalizeBackend();

	if ( ! initializeBackend( m_backendsMap[backendName], videoWidgetParent ) )
	{
		kDebug() << "Error initializing backend:" << backendName;
		for ( QMap<QString,PlayerBackend*>::ConstIterator it = m_backendsMap.begin(), end = m_backendsMap.end(); it != end; ++it )
		{
			if ( initializeBackend( it.value(), videoWidgetParent ) )
				break;
			else
				kDebug() << "Error initializingbackend:" << it.key();
		}
	}

	return m_activeBackend;
}


bool Player::initialize( QWidget* videoWidgetParent, const QString& preferredBackendName )
{
	if ( m_activeBackend )
	{
		kError() << "Player has already been initialized";
		return false;
	}

	if ( m_backendsMap.contains( preferredBackendName ) )
	{
		// we first try to set the requested backend as active
		initializeBackend( m_backendsMap[preferredBackendName], videoWidgetParent );
	}

	// if that fails, we set the first available backend as active
	if ( ! m_activeBackend )
	{
		for ( QMap<QString,PlayerBackend*>::ConstIterator it = m_backendsMap.begin(), end = m_backendsMap.end(); it != end; ++it )
			if ( initializeBackend( it.value(), videoWidgetParent ) )
				break;
	}

	if ( ! m_activeBackend )
		kError() << "Failed to initialize a player backend";

	return m_activeBackend;
}

void Player::finalize()
{
	finalizeBackend();
}

bool Player::isApplicationClosingDown() const
{
	return m_applicationClosingDown;
}

void Player::setApplicationClosingDown()
{
	m_applicationClosingDown = true;
}

void Player::resetState()
{
	if ( m_openFileTimer->isActive() )
		m_openFileTimer->stop();

	m_filePath.clear();

	m_position = -1.0;
	m_savedPosition = -1.0;
	m_length = -1.0;
	m_framesPerSecond = -1.0;
	m_minPositionDelta = DEFAULT_MIN_POSITION_DELTA;

	m_activeAudioStream = -1;
	m_audioStreams.clear();

	m_state = Player::Closed;

	if ( m_videoWidget )
		m_videoWidget->videoLayer()->hide();
}

void Player::updatePosition( double position )
{
	if ( m_state <= Player::Closed )
		return;

	if ( position > m_length && m_length > 0 )
		updateLength( position );

	if ( m_position != position )
	{
		if ( m_position <= 0 || m_minPositionDelta <= 0.0 )
		{
			m_position = position;
			emit positionChanged( position );
		}
		else
		{
			double positionDelta = m_position - position;
			if ( positionDelta >= m_minPositionDelta || -positionDelta >= m_minPositionDelta )
			{
				m_position = position;
				emit positionChanged( position );
			}
		}
	}
}

void Player::updateLength( double length )
{
	if ( m_state <= Player::Closed )
		return;

	if ( length >= 0 && m_length != length )
	{
		m_length = length;
		emit lengthChanged( length );
	}
}

void Player::updateFramesPerSecond( double framesPerSecond )
{
	if ( m_state <= Player::Closed )
		return;

	if ( framesPerSecond > 0 && m_framesPerSecond != framesPerSecond )
	{
		m_framesPerSecond = framesPerSecond;
		m_minPositionDelta = 1.0/framesPerSecond;
		emit framesPerSecondChanged( framesPerSecond );
	}
}

void Player::updateAudioStreams( const QStringList& audioStreams, int activeAudioStream )
{
	if ( m_state <= Player::Closed )
		return;

	m_audioStreams = audioStreams;

	emit audioStreamsChanged( m_audioStreams );

	if ( audioStreams.isEmpty() )
		m_activeAudioStream = -1;
	else
		m_activeAudioStream = (activeAudioStream >= 0 && activeAudioStream < audioStreams.count()) ? activeAudioStream : 0;

	emit activeAudioStreamChanged( m_activeAudioStream );
}

void Player::updateState( Player::State newState )
{
	if ( m_state == Player::Opening )
	{
		if ( newState == Player::Playing )
		{
			m_openFileTimer->stop();

			m_state = Player::Playing;
			m_videoWidget->videoLayer()->show();
			m_activeBackend->setVolume( m_backendVolume );

			emit fileOpened( m_filePath );

			// we emit this signals in case their values were already set
			emit lengthChanged( m_length );
			emit framesPerSecondChanged( m_framesPerSecond );
			emit audioStreamsChanged( m_audioStreams );
			emit activeAudioStreamChanged( m_activeAudioStream );

			emit playing();
		}
	}
	else if ( m_state > Player::Opening )
	{
		if ( m_state != newState && newState > Player::Opening )
		{
			m_state = newState;
			switch ( m_state )
			{
				case Player::Playing:
					m_videoWidget->videoLayer()->show();
					m_activeBackend->setVolume( m_backendVolume );
					emit playing();
					break;
				case Player::Paused:
					emit paused();
					break;
				case Player::Ready:
					m_videoWidget->videoLayer()->hide();
					emit stopped();
					break;
				default:
					break;
			}
		}
	}
}






bool Player::openFile( const QString& filePath )
{
	if ( m_state != Player::Closed )
		return false;

	QFileInfo fileInfo( filePath );
	if ( ! fileInfo.exists() || ! fileInfo.isFile() || ! fileInfo.isReadable() )
	{
		emit fileOpenError( filePath ); // operation will never succed
		return true;
	}

	m_filePath = filePath;
	m_state = Player::Opening;

	// FIXME add an option to set the error timeout amount when opening video
	m_openFileTimer->start( 6000 );

	bool playingAfterCall = true;
	if ( ! m_activeBackend->openFile( fileInfo.absoluteFilePath(), playingAfterCall ) )
	{
		resetState();
		emit fileOpenError( filePath );
		return true;
	}

	if ( ! playingAfterCall )
		m_activeBackend->play();

	return true;

}

void Player::onOpenFileTimeout()
{
	QString filePath( m_filePath );

	m_activeBackend->stop();
	m_activeBackend->closeFile();

	resetState();

	emit fileOpenError( filePath );
}

bool Player::closeFile()
{
	if ( m_state <= Player::Closed )
		return false;

	bool stop = m_state != Player::Ready;
	if ( stop )
		m_activeBackend->stop(); // we can safely ignore the stop() return value here as we're about to close the file

	m_activeBackend->closeFile();

	resetState();

	if ( stop )
		emit stopped();

	emit fileClosed();

	return true;
}

bool Player::play()
{
	if ( m_state <= Player::Opening || m_state == Player::Playing )
		return false;

	if ( ! m_activeBackend->play() )
	{
		resetState();
		emit playbackError();
	}

	return true;
}

bool Player::pause()
{
	if ( m_state <= Player::Opening || m_state == Player::Paused )
		return false;

	if ( ! m_activeBackend->pause() )
	{
		resetState();
		emit playbackError();
	}

	return true;
}

bool Player::togglePlayPaused()
{
	if ( m_state <= Player::Opening )
		return false;

	bool error;
	if ( m_state == Player::Playing )
		error = ! m_activeBackend->pause();
	else
		error = ! m_activeBackend->play();

	if ( error )
	{
		resetState();
		emit playbackError();
	}

	return true;
}

bool Player::seek( double seconds, bool accurate )
{

	if ( (m_state != Player::Playing && m_state != Player::Paused) || seconds < 0 || seconds > m_length )
		return false;

	if ( seconds == m_position )
		return true;

	if ( ! m_activeBackend->seek( seconds, accurate ) )
	{
		resetState();
		emit playbackError();
	}

	return true;
}

void Player::seekToSavedPosition()
{
	if ( m_savedPosition >= 0.0 )
	{
		seek( m_savedPosition, true );
		m_savedPosition = -1.0;
	}
}

bool Player::stop()
{
	if ( m_state <= Player::Opening || m_state == Player::Ready )
		return false;

	if ( ! m_activeBackend->stop() )
	{
		resetState();
		emit playbackError();
		return true;
	}

	return true;
}

bool Player::setActiveAudioStream( int audioStreamIndex )
{
	if ( m_state <= Player::Opening || m_audioStreams.size() <= 1 )
		return false;

	if ( m_activeAudioStream == audioStreamIndex || audioStreamIndex < 0 || audioStreamIndex >= m_audioStreams.size() )
		return false;

	bool onTheFly;
	if ( ! m_activeBackend->supportsChangingAudioStream( &onTheFly ) )
		return true;

	m_activeAudioStream = audioStreamIndex;

	if ( m_state != Player::Ready )
	{
		double savedPosition = m_position;

		if ( ! m_activeBackend->setActiveAudioStream( audioStreamIndex ) )
		{
			resetState();
			emit playbackError();
			return true;
		}

		if ( ! onTheFly )
		{
			if ( ! m_activeBackend->stop() )
			{
				resetState();
				emit playbackError();
				return true;
			}

			if ( savedPosition > 0.0 )
			{
				if ( ! m_activeBackend->play() )
				{
					resetState();
					emit playbackError();
					return true;
				}

				m_savedPosition = savedPosition;
				QTimer::singleShot( 500, this, SLOT( seekToSavedPosition() ) );
			}
		}
	}

	emit activeAudioStreamChanged( audioStreamIndex );
	return true;
}

void Player::increaseVolume( double amount )
{
	setVolume( m_volume + amount );
	setMuted( false );
}

void Player::decreaseVolume( double amount )
{
	setVolume( m_volume - amount );
}

void Player::setVolume( double volume )
{
	if ( volume < 0.0 )
		volume = 0.0;
	else if ( volume > 100.0 )
		volume = 100.0;

	if ( m_volume != volume )
	{
		m_volume = volume;

		m_backendVolume = m_muted ? 0 : (m_activeBackend->doesVolumeCorrection() ? m_volume : logarithmicVolume( m_volume ));

		if ( ! m_muted && m_state == Player::Playing )
		{
			if ( ! m_activeBackend->setVolume( m_backendVolume ) )
			{
				resetState();
				emit playbackError();
				return;
			}
		}

		emit volumeChanged( m_volume );
	}
}

void Player::setMuted( bool muted )
{
	if ( m_muted != muted )
	{
		m_muted = muted;

		m_backendVolume = m_muted ? 0 : (m_activeBackend->doesVolumeCorrection() ? m_volume : logarithmicVolume( m_volume ));

		if ( m_state == Player::Playing )
		{
			if ( ! m_activeBackend->setVolume( m_backendVolume ) )
			{
				resetState();
				emit playbackError();
				return;
			}
		}

		emit muteChanged( m_muted );
	}
}

#include "player.moc"
