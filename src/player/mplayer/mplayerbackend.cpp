/***************************************************************************
 *   Copyright (C) 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)  *
 *   based on smplayer by Ricardo Villalba                                 *
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

#include "mplayerbackend.h"
#include "mplayerconfigwidget.h"
#include "mplayerprocess.h"
#include "mediadata.h"
#include "../../common/qxtsignalwaiter.h"

#include <KDebug>
#include <KLocale>
#include <KStandardDirs>

using namespace SubtitleComposer;

MPlayerBackend::MPlayerBackend( Player* player ):
	PlayerBackend( player, new MPlayerConfig() ),
	m_process( new MPlayerProcess( config(), this ) ),
	m_position( 0.0 ),
	m_reportUpdates( true )
{
	connect( m_process, SIGNAL(mediaDataLoaded()), this, SLOT(onMediaDataLoaded()) );
	connect( m_process, SIGNAL(playingReceived()), this, SLOT(onPlayingReceived()) );
	connect( m_process, SIGNAL(pausedReceived()), this, SLOT(onPausedReceived()) );
	connect( m_process, SIGNAL(positionReceived(double)), this, SLOT(onPositionReceived(double)) );
	connect( m_process, SIGNAL(processExited()), this, SLOT(onProcessExited()) );
}

MPlayerBackend::~MPlayerBackend()
{
	if ( isInitialized() )
		_finalize();
}

VideoWidget* MPlayerBackend::initialize( QWidget* videoWidgetParent )
{
	return new VideoWidget( videoWidgetParent );
}

void MPlayerBackend::finalize()
{
	_finalize();
}

void MPlayerBackend::_finalize()
{
}

SubtitleComposer::AppConfigGroupWidget* MPlayerBackend::newAppConfigGroupWidget( QWidget* parent )
{
	return new MPlayerConfigWidget( parent );
}


bool MPlayerBackend::openFile( const QString& filePath, bool& playingAfterCall )
{
	m_position = 0.0;

	playingAfterCall = true;

	if ( ! m_process->start(
			filePath,
			(int)m_player->videoWidget()->videoLayer()->winId(),
			m_player->activeAudioStream(),
			m_player->audioStreams().count()
		) )
		return false;

	return true;
}

void MPlayerBackend::closeFile()
{
}

bool MPlayerBackend::stop()
{
	if ( m_process->state() == QProcess::NotRunning )
		return true;

	if ( ! m_player->isApplicationClosingDown() )
	{
 		// NOTE the quit message is not processed when the application
		// is closing down so we don't even send it because it would
		// just be a waste of 3 seconds.
		m_process->sendQuit();
		m_process->waitForFinished( 3000 );
	}

	if ( m_process->state() == QProcess::Running )
	{
		m_process->terminate();
		m_process->waitForFinished( 3000 );
	}

	if ( m_process->state() == QProcess::Running )
	{
		m_process->kill();
		m_process->waitForFinished( 3000 );
	}

	//return m_process->state() == QProcess::NotRunning;
	return true;
}

bool MPlayerBackend::play()
{
	if ( m_process->state() == QProcess::NotRunning ) // player state was Ready
	{
		m_position = 0.0;

		if ( ! m_process->start(
				m_player->filePath(),
				(int)m_player->videoWidget()->videoLayer()->winId(),
				m_player->activeAudioStream(),
				m_player->audioStreams().count()
			) )
			return false;

		if ( m_process->state() == QProcess::NotRunning )
			return false; // process ended prematurely
	}
	else // player state was Playing or Paused
		m_process->sendTogglePause();

	return true;
}

bool MPlayerBackend::pause()
{
	if ( m_process->state() == QProcess::NotRunning ) // player state was Ready
	{
		m_position = 0.0;

		if ( ! m_process->start(
				m_player->filePath(),
				(int)m_player->videoWidget()->videoLayer()->winId(),
				m_player->activeAudioStream(),
				m_player->audioStreams().count()
			) )
			return false;

		if ( m_process->state() == QProcess::NotRunning )
			return false; // process ended prematurely

		m_process->sendTogglePause();
	}
	else // player state was Playing or Paused
		m_process->sendTogglePause();

	return true;
}


bool MPlayerBackend::seek( double seconds, bool accurate )
{
	if ( accurate )
	{
		bool wasPaused = m_player->isPaused();
		bool wasMuted = m_player->isMuted();

		m_reportUpdates = false;

		if ( ! wasPaused )
			m_process->sendTogglePause();

		if ( ! wasMuted )
			m_process->sendToggleMute();

		double seekPosition = seconds;
		do
		{
			m_process->sendSeek( seekPosition );

			if ( seekPosition > 0.0 )
			{
				seekPosition -= 1.0;
				if ( seekPosition < 0.0 )
					seekPosition = 0.0;
			}
			else
				break;
		}
		while ( m_position > seconds );

		if ( ! wasMuted )
			m_process->sendToggleMute();

		if ( ! wasPaused )
			m_process->sendTogglePause();

		m_reportUpdates = true;

		m_player->updatePosition( m_position );
	}
	else
		m_process->sendFastSeek( seconds );

	return true;
}

bool MPlayerBackend::setActiveAudioStream( int audioStream )
{
	if ( m_process->state() != QProcess::NotRunning )
	{
		bool wasMuted = m_player->isMuted();
		m_process->sendAudioStream( audioStream );
		m_process->sendVolume( m_player->volume() );
		if ( wasMuted )
			m_process->sendToggleMute();
	}

	return true;
}

bool MPlayerBackend::setVolume( double volume )
{
	m_process->sendVolume( volume );
	return true;
}


void MPlayerBackend::onMediaDataLoaded()
{
	const MediaData& mediaData = m_process->mediaData();

	QStringList audioStreams;

	for ( QMap<int,TrackData>::ConstIterator it = mediaData.audioTracks.begin(), end = mediaData.audioTracks.end();
		  it != end; ++it)
	{
		QString audioStreamName;
		if ( ! it.value().language.isEmpty() )
			audioStreamName = it.value().language;
		if ( ! it.value().name.isEmpty() )
		{
			if ( ! audioStreamName.isEmpty() )
				audioStreamName += " / ";
			audioStreamName += it.value().name;
		}
		if ( audioStreamName.isEmpty() )
			audioStreamName = i18n( "Audio Stream #%1", it.key() );

		audioStreams << audioStreamName;
	}

	if ( mediaData.videoWidth && mediaData.videoHeight )
	{
		m_player->videoWidget()->setVideoResolution(
			mediaData.videoWidth,
			mediaData.videoHeight,
			mediaData.videoDAR
		);
	}

	m_player->updateAudioStreams( audioStreams, audioStreams.isEmpty() ? -1 : 0 );

	if ( mediaData.duration )
		m_player->updateLength( mediaData.duration );

	if ( mediaData.videoFPS )
		m_player->updateFramesPerSecond( mediaData.videoFPS );
}

void MPlayerBackend::onPlayingReceived()
{
	if ( ! m_reportUpdates )
		return;

	m_player->updateState( Player::Playing );
}

void MPlayerBackend::onPausedReceived()
{
	if ( ! m_reportUpdates )
		return;

	m_player->updateState( Player::Paused );
}

void MPlayerBackend::onPositionReceived( double seconds )
{
	m_position = seconds;

	if ( ! m_reportUpdates )
		return;

	if ( ! m_player->isPlaying() )
		m_player->updateState( Player::Playing );

	m_player->updatePosition( seconds );
}

void MPlayerBackend::onProcessExited()
{
	m_player->updateState( Player::Ready );
}

#include "mplayerbackend.moc"
