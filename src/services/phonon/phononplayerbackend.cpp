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

#include "phononplayerbackend.h"
#include "phononconfigwidget.h"

#include <Phonon/MediaObject>
#include <Phonon/MediaController>
#include <Phonon/ObjectDescription>
#include <Phonon/AudioOutput>
#include <Phonon/VideoWidget>

#include <KDebug>

using namespace SubtitleComposer;

PhononPlayerBackend::PhononPlayerBackend(Player *player) :
	PlayerBackend(player, "Phonon", new PhononConfig()),
	m_mediaObject(0),
	m_mediaController(0),
	m_audioOutput(0),
	m_videoOutput(0)
{}

PhononPlayerBackend::~PhononPlayerBackend()
{
	if(isInitialized())
		_finalize();
}

bool
PhononPlayerBackend::doesVolumeCorrection() const
{
	return true;
}

void
PhononPlayerBackend::initMediaObject()
{
	m_mediaObject = new Phonon::MediaObject();
	m_mediaObject->setTickInterval(40);

	connect(m_mediaObject, SIGNAL(hasVideoChanged(bool)), this, SLOT(onHasVideoChanged(bool)));
	connect(m_mediaObject, SIGNAL(finished()), this, SLOT(onFinished()));
	connect(m_mediaObject, SIGNAL(tick(qint64)), this, SLOT(onTick(qint64)));
	connect(m_mediaObject, SIGNAL(totalTimeChanged(qint64)), this, SLOT(onTotalTimeChanged(qint64)));
	connect(m_mediaObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(onStateChanged(Phonon::State, Phonon::State)));

	Phonon::createPath(m_mediaObject, m_audioOutput);
	Phonon::createPath(m_mediaObject, m_videoOutput);

	m_mediaController = new Phonon::MediaController(m_mediaObject);

	connect(m_mediaController, SIGNAL(availableAudioChannelsChanged()), this, SLOT(onAvailableAudioChannelsChanged()));

	connect(m_mediaController, SIGNAL(availableSubtitlesChanged()), this, SLOT(onAvailableSubtitlesChanged()));
}

VideoWidget *
PhononPlayerBackend::initialize(QWidget *videoWidgetParent)
{
	m_videoOutput = new Phonon::VideoWidget(0);
	m_audioOutput = new Phonon::AudioOutput(Phonon::VideoCategory);

	VideoWidget *videoWidget = new VideoWidget(m_videoOutput, videoWidgetParent);

	initMediaObject();

	return videoWidget;
}

void
PhononPlayerBackend::finalize()
{
	return _finalize();
}

void
PhononPlayerBackend::_finalize()
{
//	delete m_mediaController;
//	m_mediaController = 0;

//	delete m_mediaObject;
//	m_mediaObject = 0;

//	delete m_audioOutput;
//	m_audioOutput = 0;

	m_mediaController->disconnect();
	m_mediaController->deleteLater();
	m_mediaController = 0;

	m_mediaObject->disconnect();
	m_mediaObject->deleteLater();
	m_mediaObject = 0;

	m_audioOutput->disconnect();
	m_audioOutput->deleteLater();
	m_audioOutput = 0;

	// no need to delete the m_videoOutput as is deleted with the player's videoWidget()
	m_videoOutput = 0;
}

SubtitleComposer::AppConfigGroupWidget *
PhononPlayerBackend::newAppConfigGroupWidget(QWidget * /*parent */)
{
	return 0;                                       // no settings ATM
//	return new PhononConfigWidget( parent );
}

bool
PhononPlayerBackend::openFile(const QString &filePath, bool &playingAfterCall)
{
	playingAfterCall = true;

	Phonon::MediaSource mediaSource(filePath);

	if(mediaSource.type() == Phonon::MediaSource::Invalid)
		return false;

	m_mediaObject->setCurrentSource(mediaSource);

	if(m_mediaObject->state() == Phonon::ErrorState) {
		delete m_mediaObject;
		m_mediaObject = 0;
		initMediaObject();
		return false;
	}

	m_mediaObject->play();

	return true;
}

void
PhononPlayerBackend::closeFile()
{
	// when can't open a file, the m_mediaObject stops working correctly
	// so it's best to just destroy the old one and create a new one.

	delete m_mediaObject;
	m_mediaObject = 0;
	initMediaObject();

	if(player()->videoWidget())
		player()->videoWidget()->videoLayer()->hide();
}

bool
PhononPlayerBackend::play()
{
	m_mediaObject->play();
	return true;
}

bool
PhononPlayerBackend::pause()
{
	m_mediaObject->pause();
	return true;
}

bool
PhononPlayerBackend::seek(double seconds, bool /*accurate */)
{
	if(m_mediaObject->isSeekable())
		m_mediaObject->seek((qint64)(seconds * 1000));
	return true;
}

bool
PhononPlayerBackend::stop()
{
	m_mediaObject->stop();

	return true;
}

bool
PhononPlayerBackend::setActiveAudioStream(int audioStream)
{
	QList<Phonon::AudioChannelDescription> audioChannels = m_mediaController->availableAudioChannels();
	if(audioChannels.length() > audioStream && audioStream >= 0) {
		m_mediaController->setCurrentAudioChannel(audioChannels[audioStream]);
		return true;
	}
	return false;
}

bool
PhononPlayerBackend::setVolume(double volume)
{
	m_audioOutput->setVolume(volume / 100);
	return true;
}

void
PhononPlayerBackend::onHasVideoChanged(bool /*hasVideo */)
{
//	if(!hasVideo)
//		player()->videoWidget()->setVideoResolution(0, 0);
//	else // TODO: how can we get the video resolution
//		player()->videoWidget()->setVideoResolution(640, 480);
}

void
PhononPlayerBackend::onFinished()
{
	setPlayerState(Player::Ready);
}

void
PhononPlayerBackend::onTick(qint64 currentTime)
{
	setPlayerPosition(currentTime / 1000.0);
}

void
PhononPlayerBackend::onTotalTimeChanged(qint64 newTotalTime)
{
	setPlayerLength(newTotalTime / 1000.0);
	// FIXME: update frame rate and set tick interval to frame rate
	// can't be done with what Phonon provides ATM
}

void
PhononPlayerBackend::onAvailableAudioChannelsChanged()
{
	QStringList audioStreams;

	QList<Phonon::AudioChannelDescription> audioChannels = m_mediaController->availableAudioChannels();
	int idx = -1, i = 0;
	for(QList<Phonon::AudioChannelDescription>::ConstIterator it = audioChannels.begin(), end = audioChannels.end(); it != end; ++it) {
		audioStreams << (*it).name();
		if(it->index() == m_mediaController->currentAudioChannel().index())
			idx = i;
		i++;
	}

	setPlayerAudioStreams(audioStreams, idx);
}

void
PhononPlayerBackend::onAvailableSubtitlesChanged()
{
	// Subtitles display is not handled by the video backends
	// If there are subtitles, the backend must hide them.
	m_mediaController->setCurrentSubtitle(Phonon::SubtitleDescription::fromIndex(-1));
}

void
PhononPlayerBackend::onStateChanged(Phonon::State newState, Phonon::State /*oldState */)
{
	if(!isInitialized())
		return;

	switch(newState) {
	case Phonon::StoppedState:
		setPlayerState(Player::Ready);
		break;
	case Phonon::LoadingState:
	case Phonon::PlayingState:
		setPlayerState(Player::Playing);
		break;
	case Phonon::PausedState:
		setPlayerState(Player::Paused);
		break;
	case Phonon::ErrorState:
		setPlayerErrorState();
		break;
	default:
		break;
	}
}

#include "phononplayerbackend.moc"
