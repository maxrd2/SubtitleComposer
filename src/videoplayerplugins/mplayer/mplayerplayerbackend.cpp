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

#include "mplayerplayerbackend.h"
#include "mplayerplayerprocess.h"
#include "mplayerconfigwidget.h"
#include "mplayerconfig.h"
#include "mediadata.h"

#include <QDebug>

#include <KLocalizedString>

#define waitState(condition) \
	while(m_process->state() != QProcess::NotRunning && !(condition)) { \
		QCoreApplication::processEvents(); \
		QCoreApplication::sendPostedEvents(); \
	}

using namespace SubtitleComposer;

MPlayerBackend::MPlayerBackend()
	: PlayerBackend(),
	  m_state(STOPPED),
	  m_initialized(false),
	  m_muted(false),
	  m_process(new MPlayerProcess(this)),
	  m_nativeWindow(nullptr),
	  m_position(0.0),
	  m_reportUpdates(true)
{
	m_name = QStringLiteral("MPlayer");
	connect(m_process, SIGNAL(mediaDataLoaded()), this, SLOT(onMediaDataLoaded()));
	connect(m_process, SIGNAL(playingReceived()), this, SLOT(onPlayingReceived()));
	connect(m_process, SIGNAL(pausedReceived()), this, SLOT(onPausedReceived()));
	connect(m_process, SIGNAL(positionReceived(double)), this, SLOT(onPositionReceived(double)));
	connect(m_process, SIGNAL(processExited()), this, SLOT(onProcessExited()));
}

MPlayerBackend::~MPlayerBackend()
{
}

void
MPlayerBackend::setState(PlayState state)
{
	static VideoPlayer::State vpState[] = { VideoPlayer::Ready, VideoPlayer::Paused, VideoPlayer::Playing };
	if(m_state == state)
		return;
	m_state = state;
	emit stateChanged(vpState[state]);
}

bool
MPlayerBackend::init(QWidget *videoWidget)
{
	if(!m_nativeWindow) {
		m_nativeWindow = new QWidget(videoWidget);
		m_nativeWindow->setAttribute(Qt::WA_DontCreateNativeAncestors);
		m_nativeWindow->setAttribute(Qt::WA_NativeWindow);
		connect(m_nativeWindow, &QWidget::destroyed, [&](){ m_nativeWindow = nullptr; });
	} else {
		m_nativeWindow->setParent(videoWidget);
	}
	return true;
}

void
MPlayerBackend::cleanup()
{
}

QWidget *
MPlayerBackend::newConfigWidget(QWidget *parent)
{
	return new MPlayerConfigWidget(parent);
}

KCoreConfigSkeleton *
MPlayerBackend::config() const
{
	return MPlayerConfig::self();
}

bool
MPlayerBackend::openFile(const QString &path)
{
	m_position = 0.0;

	m_currentFilePath = path;
	m_initialized = false;
	m_state = STOPPED;
	m_muted = false;
	if(!m_process->start(path, m_nativeWindow->winId()))
		return false;

	waitState(m_initialized && m_state == PAUSED);

	return m_state != STOPPED;
}

bool
MPlayerBackend::closeFile()
{
	m_currentFilePath.clear();
	return true;
}

bool
MPlayerBackend::stop()
{
	if(m_process->state() == QProcess::NotRunning)
		return true;

	if(!QApplication::closingDown()) {
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
MPlayerBackend::play()
{
	if(m_process->state() == QProcess::NotRunning) { // player state was Ready
		m_position = 0.0;

		if(!m_process->start(m_currentFilePath, m_nativeWindow->winId()))
			return false;

		if(m_process->state() == QProcess::NotRunning)
			return false; // process ended prematurely
	} else // player state was Playing or Paused
		m_process->sendTogglePause();

	return true;
}

bool
MPlayerBackend::pause()
{
	if(m_process->state() == QProcess::NotRunning) { // player state was Ready
		m_position = 0.0;

		if(!m_process->start(m_currentFilePath, m_nativeWindow->winId()))
			return false;

		if(m_process->state() == QProcess::NotRunning)
			return false; // process ended prematurely

		m_process->sendTogglePause();
	} else // player state was Playing or Paused
		m_process->sendTogglePause();

	return true;
}

bool
MPlayerBackend::seek(double seconds)
{
	const bool wasPaused = m_state == PAUSED;
	const bool wasMuted = m_muted;

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

	emit positionChanged(m_position);

	return true;
}

bool
MPlayerBackend::step(int frameOffset)
{
	// TODO: handle backward stepping somehow
	for(int i = 0; i < frameOffset; i++)
		m_process->sendFrameStep();
	return true;
}

bool
MPlayerBackend::playbackRate(double newRate)
{
	m_process->sendSpeedSet(newRate);
	return true;
}

bool
MPlayerBackend::selectAudioStream(int streamIndex)
{
	if(m_process->state() != QProcess::NotRunning) {
		const bool wasMuted = m_muted;
		m_process->sendAudioStream(m_process->mediaData().idForIndex(streamIndex));
//		m_process->sendVolume(player()->volume());
		if(wasMuted)
			m_process->sendToggleMute();
	}

	return true;
}

bool
MPlayerBackend::setVolume(double volume)
{
	m_process->sendVolume(volume);
	return true;
}

void
MPlayerBackend::onMediaDataLoaded()
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

	if(mediaData.videoWidth && mediaData.videoHeight)
		emit resolutionChanged(mediaData.videoWidth, mediaData.videoHeight, mediaData.videoDAR);

	emit audioStreamsChanged(audioStreams, audioStreams.isEmpty() ? -1 : 0);

	if(mediaData.duration)
		emit lengthChanged(mediaData.duration);

	if(mediaData.videoFPS)
		emit fpsChanged(mediaData.videoFPS);

	m_initialized = true;
}

void
MPlayerBackend::onPlayingReceived()
{
	if(!m_reportUpdates)
		return;

	setState(PLAYING);
}

void
MPlayerBackend::onPausedReceived()
{
	if(!m_reportUpdates)
		return;

	setState(PAUSED);
}

void
MPlayerBackend::onPositionReceived(double seconds)
{
	m_position = seconds;

	if(!m_reportUpdates)
		return;

	emit positionChanged(seconds);
}

void
MPlayerBackend::onProcessExited()
{
	setState(STOPPED);
}

bool
MPlayerBackend::reconfigure()
{
	if(m_state != STOPPED)
		return true;

	const double oldPosition = m_position;
	const bool wasPaused = m_state == PAUSED;

	stop();
	play();
	if(wasPaused)
		pause();
	seek(oldPosition);

	return true;
}
