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

#include "mplayerplayerbackend.h"
#include "mplayerplayerprocess.h"
#include "mplayerconfigwidget.h"
#include "mediadata.h"
#include "../../common/qxtsignalwaiter.h"

#include <KDebug>
#include <KLocale>
#include <KStandardDirs>

using namespace SubtitleComposer;

MPlayerPlayerBackend::MPlayerPlayerBackend(Player *player) :
	PlayerBackend(player, "MPlayer", new MPlayerConfig()),
	m_process(new MPlayerPlayerProcess(config(), this)),
	m_position(0.0),
	m_reportUpdates(true)
{
	connect(m_process, SIGNAL(mediaDataLoaded()), this, SLOT(onMediaDataLoaded()));
	connect(m_process, SIGNAL(playingReceived()), this, SLOT(onPlayingReceived()));
	connect(m_process, SIGNAL(pausedReceived()), this, SLOT(onPausedReceived()));
	connect(m_process, SIGNAL(positionReceived(double)), this, SLOT(onPositionReceived(double)));
	connect(m_process, SIGNAL(processExited()), this, SLOT(onProcessExited()));
}

MPlayerPlayerBackend::~MPlayerPlayerBackend()
{
	if(isInitialized())
		_finalize();
}

VideoWidget *
MPlayerPlayerBackend::initialize(QWidget *videoWidgetParent)
{
	return new VideoWidget(videoWidgetParent);
}

void
MPlayerPlayerBackend::finalize()
{
	_finalize();
}

void
MPlayerPlayerBackend::_finalize()
{}

SubtitleComposer::AppConfigGroupWidget *
MPlayerPlayerBackend::newAppConfigGroupWidget(QWidget *parent)
{
	return new MPlayerConfigWidget(parent);
}

bool
MPlayerPlayerBackend::openFile(const QString &filePath, bool &playingAfterCall)
{
	m_position = 0.0;

	playingAfterCall = true;

	if(!m_process->start(filePath, (int)player()->videoWidget()->videoLayer()->winId(), player()->activeAudioStream(), player()->audioStreams().count()
	                     ))
		return false;

	return true;
}

void
MPlayerPlayerBackend::closeFile()
{}

bool
MPlayerPlayerBackend::stop()
{
	if(m_process->state() == QProcess::NotRunning)
		return true;

	if(!player()->isApplicationClosingDown()) {
		// NOTE the quit message is not processed when the application
		// is closing down so we don't even send it because it would
		// just be a waste of 3 seconds.
		m_process->sendQuit();
		m_process->waitForFinished(3000);
	}

	if(m_process->state() == QProcess::Running) {
		m_process->terminate();
		m_process->waitForFinished(3000);
	}

	if(m_process->state() == QProcess::Running) {
		m_process->kill();
		m_process->waitForFinished(3000);
	}
	// return m_process->state() == QProcess::NotRunning;
	return true;
}

bool
MPlayerPlayerBackend::play()
{
	if(m_process->state() == QProcess::NotRunning) { // player state was Ready
		m_position = 0.0;

		if(!m_process->start(player()->filePath(), (int)player()->videoWidget()->videoLayer()->winId(), player()->activeAudioStream(), player()->audioStreams().count()
		                     ))
			return false;

		if(m_process->state() == QProcess::NotRunning)
			return false; // process ended prematurely
	} else // player state was Playing or Paused
		m_process->sendTogglePause();

	return true;
}

bool
MPlayerPlayerBackend::pause()
{
	if(m_process->state() == QProcess::NotRunning) { // player state was Ready
		m_position = 0.0;

		if(!m_process->start(player()->filePath(), (int)player()->videoWidget()->videoLayer()->winId(), player()->activeAudioStream(), player()->audioStreams().count()
		                     ))
			return false;

		if(m_process->state() == QProcess::NotRunning)
			return false; // process ended prematurely

		m_process->sendTogglePause();
	} else // player state was Playing or Paused
		m_process->sendTogglePause();

	return true;
}

bool
MPlayerPlayerBackend::seek(double seconds, bool accurate)
{
	if(accurate) {
		bool wasPaused = player()->isPaused();
		bool wasMuted = player()->isMuted();

		m_reportUpdates = false;

		if(m_process->version() == 1) {
			if(!wasPaused)
				m_process->sendTogglePause();

			if(!wasMuted)
				m_process->sendToggleMute();
		}

		double seekPosition = seconds;
		do {
			m_process->sendSeek(seekPosition);

			if(seekPosition > 0.0) {
				seekPosition -= 1.0;
				if(seekPosition < 0.0)
					seekPosition = 0.0;
			} else
				break;
		} while(m_position > seconds);

		if(m_process->version() == 1) {
			if(!wasMuted)
				m_process->sendToggleMute();

			if(!wasPaused)
				m_process->sendTogglePause();
		}

		m_reportUpdates = true;

		setPlayerPosition(m_position);
	} else
		m_process->sendFastSeek(seconds);

	return true;
}

bool
MPlayerPlayerBackend::setActiveAudioStream(int audioStream)
{
	if(m_process->state() != QProcess::NotRunning) {
		bool wasMuted = player()->isMuted();
		m_process->sendAudioStream(m_process->mediaData().idForIndex(audioStream));
		m_process->sendVolume(player()->volume());
		if(wasMuted)
			m_process->sendToggleMute();
	}

	return true;
}

bool
MPlayerPlayerBackend::setVolume(double volume)
{
	m_process->sendVolume(volume);
	return true;
}

void
MPlayerPlayerBackend::onMediaDataLoaded()
{
	const MediaData &mediaData = m_process->mediaData();

	QStringList audioStreams;

	int index = 0;
	for(QMap<int, TrackData>::ConstIterator it = mediaData.audioTracks.begin(), end = mediaData.audioTracks.end(); it != end; ++it) {
		index++;
		QString audioStreamName;
		if(!it.value().language.isEmpty())
			audioStreamName = it.value().language;
		if(!it.value().name.isEmpty()) {
			if(!audioStreamName.isEmpty())
				audioStreamName += " / ";
			audioStreamName += it.value().name;
		}
		if(audioStreamName.isEmpty())
			audioStreamName = i18n("Audio Stream #%1", index);
		audioStreams << audioStreamName;
	}

	if(mediaData.videoWidth && mediaData.videoHeight) {
		player()->videoWidget()->setVideoResolution(mediaData.videoWidth, mediaData.videoHeight, mediaData.videoDAR);
	}

	setPlayerAudioStreams(audioStreams, audioStreams.isEmpty() ? -1 : 0);

	if(mediaData.duration)
		setPlayerLength(mediaData.duration);

	if(mediaData.videoFPS)
		setPlayerFramesPerSecond(mediaData.videoFPS);
}

void
MPlayerPlayerBackend::onPlayingReceived()
{
	if(!m_reportUpdates)
		return;

	setPlayerState(Player::Playing);
}

void
MPlayerPlayerBackend::onPausedReceived()
{
	if(!m_reportUpdates)
		return;

	setPlayerState(Player::Paused);
}

void
MPlayerPlayerBackend::onPositionReceived(double seconds)
{
	m_position = seconds;

	if(!m_reportUpdates)
		return;

	if(!player()->isPlaying())
		setPlayerState(Player::Playing);

	setPlayerPosition(seconds);
}

void
MPlayerPlayerBackend::onProcessExited()
{
	setPlayerState(Player::Ready);
}

#include "mplayerplayerbackend.moc"
