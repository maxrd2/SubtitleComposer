/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2019 Mladen Milinkovic <max@smoothware.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "phononplayerbackend.h"

#include <Phonon/MediaObject>
#include <Phonon/MediaController>
#include <Phonon/ObjectDescription>
#include <Phonon/AudioOutput>
#include <Phonon/VideoWidget>

using namespace SubtitleComposer;

PhononPlayerBackend::PhononPlayerBackend()
	: PlayerBackend(),
	  m_state(STOPPED),
	  m_mediaObject(nullptr),
	  m_mediaController(nullptr),
	  m_audioOutput(nullptr),
	  m_videoOutput(nullptr)
{
	m_name = QStringLiteral("Phonon");
}

PhononPlayerBackend::~PhononPlayerBackend()
{
	cleanup();
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

bool
PhononPlayerBackend::init(QWidget *videoWidget)
{
	if(!m_videoOutput) {
		m_videoOutput = new Phonon::VideoWidget(videoWidget);
		connect(m_videoOutput, &QWidget::destroyed, [&](){ m_videoOutput = nullptr; });
	} else {
		m_videoOutput->setParent(videoWidget);
	}

	m_audioOutput = new Phonon::AudioOutput(Phonon::VideoCategory);

	initMediaObject();

	return true;
}

void
PhononPlayerBackend::cleanup()
{
	m_mediaController->disconnect();
	m_mediaController->deleteLater();
	m_mediaController = nullptr;

	m_mediaObject->disconnect();
	m_mediaObject->deleteLater();
	m_mediaObject = nullptr;

	m_audioOutput->disconnect();
	m_audioOutput->deleteLater();
	m_audioOutput = nullptr;

	// no need to delete the m_videoOutput as is deleted with the player's videoWidget()
	m_videoOutput = nullptr;

	m_state = STOPPED;
}

bool
PhononPlayerBackend::openFile(const QString &filePath)
{
	Phonon::MediaSource mediaSource(QUrl::fromLocalFile(filePath));

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

bool
PhononPlayerBackend::closeFile()
{
	// when can't open a file, the m_mediaObject stops working correctly
	// so it's best to just destroy the old one and create a new one.

	delete m_mediaObject;
	m_mediaObject = nullptr;
	initMediaObject();

	m_videoOutput->hide();
	return true;
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
PhononPlayerBackend::seek(double seconds)
{
	if(m_mediaObject->isSeekable())
		m_mediaObject->seek(qint64(seconds * 1000));
	return true;
}

bool
PhononPlayerBackend::stop()
{
	m_mediaObject->stop();

	return true;
}

bool
PhononPlayerBackend::selectAudioStream(int streamIndex)
{
	QList<Phonon::AudioChannelDescription> audioChannels = m_mediaController->availableAudioChannels();
	if(audioChannels.length() > streamIndex && streamIndex >= 0) {
		m_mediaController->setCurrentAudioChannel(audioChannels[streamIndex]);
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
	setState(STOPPED);
}

void
PhononPlayerBackend::onTick(qint64 currentTime)
{
	emit positionChanged(currentTime / 1000.0);
}

void
PhononPlayerBackend::onTotalTimeChanged(qint64 newTotalTime)
{
	emit lengthChanged(newTotalTime / 1000.0);
	// FIXME: update frame rate and set tick interval to frame rate
	// can't be done with what Phonon provides ATM
}

void
PhononPlayerBackend::onAvailableAudioChannelsChanged()
{
	QStringList audioStreams;

	QList<Phonon::AudioChannelDescription> audioChannels = m_mediaController->availableAudioChannels();
	int idx = -1, i = 0;
	for(QList<Phonon::AudioChannelDescription>::ConstIterator it = audioChannels.constBegin(), end = audioChannels.constEnd(); it != end; ++it) {
		audioStreams << (*it).name();
		if(it->index() == m_mediaController->currentAudioChannel().index())
			idx = i;
		i++;
	}

	emit audioStreamsChanged(audioStreams, idx);
}

void
PhononPlayerBackend::onAvailableSubtitlesChanged()
{
	// Subtitles display is not handled by the video backends
	// If there are subtitles, the backend must hide them.
	m_mediaController->setCurrentSubtitle(Phonon::SubtitleDescription::fromIndex(1));
}

void
PhononPlayerBackend::onStateChanged(Phonon::State newState, Phonon::State /*oldState*/)
{
	switch(newState) {
	case Phonon::StoppedState:
	case Phonon::ErrorState:
		setState(STOPPED);
		break;
	case Phonon::LoadingState:
	case Phonon::PlayingState:
		setState(PLAYING);
		break;
	case Phonon::PausedState:
	case Phonon::BufferingState:
		setState(PAUSED);
		break;
	}
}

void
PhononPlayerBackend::setState(PlayState state)
{
	static VideoPlayer::State vpState[] = { VideoPlayer::Ready, VideoPlayer::Paused, VideoPlayer::Playing };
	if(m_state == state)
		return;
	m_state = state;
	emit stateChanged(vpState[state]);
}
