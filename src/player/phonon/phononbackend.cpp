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

#include "phononbackend.h"
#include "phononconfigwidget.h"

#include <Phonon/MediaObject>
#include <Phonon/MediaController>
#include <Phonon/ObjectDescription>
#include <Phonon/AudioOutput>
#include <Phonon/VideoWidget>

#include <KDebug>

using namespace SubtitleComposer;

PhononBackend::PhononBackend( Player* player ):
	PlayerBackend( player, new PhononConfig() ),
	m_mediaObject( 0 ),
	m_mediaController( 0 ),
	m_audioOutput( 0 ),
	m_videoOutput( 0 )
{
}

PhononBackend::~PhononBackend()
{
	if ( isInitialized() )
		_finalize();
}

bool PhononBackend::doesVolumeCorrection() const
{
	return true;
}

bool PhononBackend::supportsChangingAudioStream( bool* /*onTheFly*/ ) const
{
	return false;
}

void PhononBackend::initMediaObject()
{
	m_mediaObject = new Phonon::MediaObject();
	m_mediaObject->setTickInterval( 40 );

	connect( m_mediaObject, SIGNAL(hasVideoChanged(bool)),
			 this, SLOT(onHasVideoChanged(bool)) );
	connect( m_mediaObject, SIGNAL(finished()),
			 this, SLOT(onFinished()) );
	connect( m_mediaObject, SIGNAL(tick(qint64)),
			 this, SLOT(onTick(qint64)) );
	connect( m_mediaObject, SIGNAL(totalTimeChanged(qint64)),
			 this, SLOT(onTotalTimeChanged(qint64)) );
	connect( m_mediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)),
			 this, SLOT(onStateChanged(Phonon::State,Phonon::State)) );

	Phonon::createPath( m_mediaObject, m_audioOutput );
	Phonon::createPath( m_mediaObject, m_videoOutput );

	m_mediaController = new Phonon::MediaController( m_mediaObject );

	connect( m_mediaController, SIGNAL(availableAudioChannelsChanged()),
			 this, SLOT(onAvailableAudioChannelsChanged()) );

	connect( m_mediaController, SIGNAL(availableSubtitlesChanged()),
			 this, SLOT(onAvailableSubtitlesChanged()) );
}

VideoWidget* PhononBackend::initialize( QWidget* videoWidgetParent )
{
	m_videoOutput = new Phonon::VideoWidget( 0 );
	m_audioOutput = new Phonon::AudioOutput( Phonon::VideoCategory );

	VideoWidget* videoWidget = new VideoWidget( m_videoOutput, videoWidgetParent );

	initMediaObject();

	return videoWidget;
}

void PhononBackend::finalize()
{
	return _finalize();
}

void PhononBackend::_finalize()
{
	delete m_mediaController;
	m_mediaController = 0;

	delete m_mediaObject;
	m_mediaObject = 0;

	delete m_audioOutput;
	m_audioOutput = 0;

	// no need to delete the m_videoOutput as is deleted with the player's videoWidget()
	m_videoOutput = 0;
}

SubtitleComposer::AppConfigGroupWidget* PhononBackend::newAppConfigGroupWidget( QWidget* /*parent*/ )
{
	return 0; // no settings ATM
// 	return new PhononConfigWidget( parent );
}

bool PhononBackend::openFile( const QString& filePath, bool& playingAfterCall )
{
	playingAfterCall = true;

	Phonon::MediaSource mediaSource( filePath );

	if ( mediaSource.type() == Phonon::MediaSource::Invalid )
		return false;

	m_mediaObject->setCurrentSource( mediaSource );

	if ( m_mediaObject->state() == Phonon::ErrorState )
	{
		delete m_mediaObject;
		m_mediaObject = 0;
		initMediaObject();
		return false;
	}

	m_mediaObject->play();

	return true;
}

void PhononBackend::closeFile()
{
	// when can't open a file, the m_mediaObject stops working correctly
	// so it's best to just destroy the old one and create a new one.

	delete m_mediaObject;
	m_mediaObject = 0;
	initMediaObject();

	if ( m_player->videoWidget() )
		m_player->videoWidget()->videoLayer()->hide();
}

bool PhononBackend::play()
{
	m_mediaObject->play();
	return true;
}

bool PhononBackend::pause()
{
	m_mediaObject->pause();
	return true;
}

bool PhononBackend::seek( double seconds, bool /*accurate*/ )
{
	if ( m_mediaObject->isSeekable() )
		m_mediaObject->seek( (qint64)(seconds * 1000) );
	return true;
}

bool PhononBackend::stop()
{
	m_mediaObject->stop();

	return true;
}

bool PhononBackend::setActiveAudioStream( int audioStream )
{
	m_mediaController->setCurrentAudioChannel( Phonon::AudioChannelDescription::fromIndex( audioStream ) );
	return true;
}

bool PhononBackend::setVolume( double volume )
{
	m_audioOutput->setVolume( volume/100 );
	return true;
}


void PhononBackend::onHasVideoChanged( bool /*hasVideo*/ )
{
	/*if ( ! hasVideo )
		m_player->videoWidget()->setVideoResolution( 0, 0 );
	else // TODO how can we get the video resolution
		m_player->videoWidget()->setVideoResolution( 640, 480 );*/
}

void PhononBackend::onFinished()
{
	m_player->updateState( Player::Ready );
}

void PhononBackend::onTick( qint64 currentTime )
{
	m_player->updatePosition( currentTime / 1000.0 );
}

void PhononBackend::onTotalTimeChanged( qint64 newTotalTime )
{
	m_player->updateLength( newTotalTime / 1000.0 );
	// FIXME update frame rate and set tick interval to frame rate
	// can't be done with what Phonon provides ATM
}

void PhononBackend::onAvailableAudioChannelsChanged()
{
	QStringList audioStreams;

	QList<Phonon::AudioChannelDescription> audioChannels = m_mediaController->availableAudioChannels();
	for ( QList<Phonon::AudioChannelDescription>::ConstIterator it = audioChannels.begin(), end = audioChannels.end();
		  it != end; ++it )
		audioStreams << (*it).name();

	m_player->updateAudioStreams( audioStreams, m_mediaController->currentAudioChannel().index() );
}

void PhononBackend::onAvailableSubtitlesChanged()
{
	// Subtitles display is not handled by the video backends
	// If there are subtitles, the backend must hide them.
	m_mediaController->setCurrentSubtitle( Phonon::SubtitleDescription::fromIndex( -1 ) );
}

void PhononBackend::onStateChanged( Phonon::State newState, Phonon::State /*oldState*/ )
{
	if ( ! isInitialized() )
		return;

	switch( newState )
	{
		case Phonon::StoppedState:
			//kDebug() << "changing state to Ready";
			m_player->updateState( Player::Ready );
			break;
		case Phonon::LoadingState:
		//case Phonon::BufferingState:
		case Phonon::PlayingState:
			//kDebug() << "changing state to Playing";
			m_player->updateState( Player::Playing );
			break;
		case Phonon::PausedState:
			//kDebug() << "changing state to Paused";
			m_player->updateState( Player::Paused );
			break;
		case Phonon::ErrorState:
		{
			// TODO what should we do in this case??
			return;
		}
		default:
			return;
	}
}

#include "phononbackend.moc"
