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

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "helpers/pluginhelper.h"
#include "videoplayer.h"
#include "playerbackend.h"
#include "scconfig.h"

#include <math.h>

#include <QTimer>
#include <QFileInfo>
#include <QApplication>
#include <QEvent>

#include <QDebug>

#include <KLocalizedString>

#define DEFAULT_MIN_POSITION_DELTA 0.02

using namespace SubtitleComposer;

VideoPlayer::VideoPlayer() :
	m_backend(nullptr),
	m_videoContainer(nullptr),
	m_state(VideoPlayer::Uninitialized),
	m_videoWidget(nullptr),
	m_filePath(),
	m_position(-1.0),
	m_length(-1.0),
	m_fps(-1.0),
	m_playbackRate(.0),
	m_minPositionDelta(DEFAULT_MIN_POSITION_DELTA),
	m_textStreams(),
	m_activeAudioStream(-1),
	m_audioStreams(),
	m_muted(false),
	m_volume(100.0),
	m_backendVolume(100.0)
{
	PluginHelper<VideoPlayer, PlayerBackend>(this).loadAll(QStringLiteral("videoplayerplugins"));
}

VideoPlayer::~VideoPlayer()
{
}

VideoPlayer *
VideoPlayer::instance()
{
	static VideoPlayer *player = nullptr;
	if(!player) {
		player = new VideoPlayer();
		player->setParent(QApplication::instance());
	}
	return player;
}

bool
VideoPlayer::init(QWidget *widgetParent, const QString &backendName)
{
	if(isInitialized()) {
		qCritical() << "VideoPlayer has already been initialized";
		return false;
	}

	m_videoContainer = widgetParent;

	if(m_plugins.contains(backendName)) {
		// we first try to set the requested backend as active
		backendInit(m_plugins[backendName]);
	}
	// if that fails, we set the first available backend as active
	if(!m_backend) {
		for(QMap<QString, PlayerBackend *>::ConstIterator it = m_plugins.begin(), end = m_plugins.end(); it != end; ++it)
			if(backendInit(it.value()))
				break;
	}

	if(!m_backend)
		qCritical() << "Failed to initialize a player backend";

	return m_backend;
}

bool
VideoPlayer::switchBackend(const QString &backendName)
{
	if(!isInitialized())
		return false;

	if(backendName == m_backend->name())
		return true;

	QString currentFile = m_filePath;

	Q_ASSERT(m_plugins.contains(backendName));

	cleanup();

	if(!backendInit(m_plugins[backendName])) {
		for(QMap<QString, PlayerBackend *>::ConstIterator it = m_plugins.begin(), end = m_plugins.end(); it != end; ++it)
			if(backendInit(it.value()))
				break;
	}

	if(!m_backend) {
		qCritical() << "Failed to initialize a player backend";
		return false;
	}

	if(!currentFile.isEmpty())
		openFile(currentFile);

	return true;
}

void
VideoPlayer::cleanup()
{
	if(!isInitialized())
		return;

	PlayerBackend *wasActiveBackend = m_backend;

	backendCleanup();

	m_state = VideoPlayer::Uninitialized;

	emit backendFinalized(wasActiveBackend);
}

bool
VideoPlayer::backendInit(PlayerBackend *backend)
{
	if(m_backend == backend)
		return true;

	if(m_backend)
		return false;

	Q_ASSERT(m_videoWidget == nullptr);

	m_videoWidget = new VideoWidget(m_videoContainer);
	QWidget layer(m_videoWidget); // FIXME: ugly to have extra widget?
	if(!backend->init(&layer))
		return false;

	m_videoWidget->setVideoLayer(layer.findChild<QWidget *>());

	connect(m_videoWidget, &VideoWidget::destroyed, this, [&](){
		Q_ASSERT(m_state == VideoPlayer::Uninitialized);
		m_videoWidget = nullptr;
		cleanup();
	});
	connect(m_videoWidget, &VideoWidget::doubleClicked, this, &VideoPlayer::doubleClicked);
	connect(m_videoWidget, &VideoWidget::leftClicked, this, &VideoPlayer::leftClicked);
	connect(m_videoWidget, &VideoWidget::rightClicked, this, &VideoPlayer::rightClicked);
	connect(m_videoWidget, &VideoWidget::wheelUp, this, &VideoPlayer::wheelUp);
	connect(m_videoWidget, &VideoWidget::wheelDown, this, &VideoPlayer::wheelDown);

	m_videoWidget->show();
	m_videoWidget->videoLayer()->hide();

	// NOTE: next is used to make videoWidgetParent update it's geometry
	QRect geometry = m_videoContainer->geometry();
	geometry.setHeight(geometry.height() + 1);
	m_videoContainer->setGeometry(geometry);

	m_state = VideoPlayer::Initialized;
	m_backend = backend;
	connect(m_backend, &PlayerBackend::resolutionChanged, this, &VideoPlayer::changeResolution);
	connect(m_backend, &PlayerBackend::fpsChanged, this, &VideoPlayer::changeFPS);
	connect(m_backend, &PlayerBackend::textStreamsChanged, this, &VideoPlayer::updateTextStreams);
	connect(m_backend, &PlayerBackend::audioStreamsChanged, this, &VideoPlayer::updateAudioStreams);
	connect(m_backend, &PlayerBackend::errorOccured, this, &VideoPlayer::onError);

	connect(m_backend, &PlayerBackend::stateChanged, this, &VideoPlayer::changeState);
	connect(m_backend, &PlayerBackend::positionChanged, this, &VideoPlayer::changePosition);
	connect(m_backend, &PlayerBackend::lengthChanged, this, &VideoPlayer::changeLength);
	connect(m_backend, &PlayerBackend::speedChanged, this, &VideoPlayer::changePlaySpeed);
	connect(m_backend, &PlayerBackend::volumeChanged, this, &VideoPlayer::changeVolume);
	connect(m_backend, &PlayerBackend::muteChanged, this, &VideoPlayer::changeMute);

	emit backendInitialized(backend);
	return true;
}

void
VideoPlayer::backendCleanup()
{
	closeFile();

	m_backend->cleanup();
	m_backend = nullptr;

	if(m_videoWidget) {
		m_videoWidget->disconnect();
		m_videoWidget->hide();
		m_videoWidget->deleteLater();
		m_videoWidget = nullptr;
	}
}

void
VideoPlayer::resetState()
{
	m_filePath.clear();

	m_position = -1.0;
	m_length = -1.0;
	m_fps = -1.0;
	m_minPositionDelta = DEFAULT_MIN_POSITION_DELTA;

	m_activeAudioStream = -1;
	m_textStreams.clear();
	m_audioStreams.clear();

	m_state = VideoPlayer::Closed;

	if(m_videoWidget)
		m_videoWidget->videoLayer()->hide();
}

void
VideoPlayer::changeResolution(int width, int height, double aspectRatio)
{
	videoWidget()->setVideoResolution(width, height, aspectRatio);
}

void
VideoPlayer::changeFPS(double fps)
{
	if(m_state <= VideoPlayer::Closed)
		return;

	if(fps > 0 && m_fps != fps) {
		m_fps = fps;
		m_minPositionDelta = 1.0 / fps;
		emit framesPerSecondChanged(fps);
	}
}

void
VideoPlayer::updateTextStreams(const QStringList &textStreams)
{
	m_textStreams = textStreams;
	emit textStreamsChanged(m_textStreams);
}

void
VideoPlayer::updateAudioStreams(const QStringList &audioStreams, int activeAudioStream)
{
	if(m_state <= VideoPlayer::Closed)
		return;

	m_audioStreams = audioStreams;

	emit audioStreamsChanged(m_audioStreams);

	if(audioStreams.isEmpty())
		m_activeAudioStream = -1;
	else
		m_activeAudioStream = (activeAudioStream >= 0 && activeAudioStream < audioStreams.count()) ? activeAudioStream : 0;

	emit activeAudioStreamChanged(m_activeAudioStream);
}

void
VideoPlayer::onError(const QString &message)
{
	if(m_state < VideoPlayer::Opening)
		return;

	if(m_state == VideoPlayer::Opening) {
		resetState();
		emit fileOpenError(m_filePath, message);
	} else {
		m_backend->stop();
		m_state = VideoPlayer::Ready;
		emit playbackError(message);
		emit stopped();
	}
}

void
VideoPlayer::changeState(VideoPlayer::State newState)
{
	if(m_state == VideoPlayer::Opening) {
		if(newState > VideoPlayer::Opening) {
			m_state = newState;
			m_videoWidget->videoLayer()->show();
			m_backend->setVolume(m_backendVolume);

			emit fileOpened(m_filePath);

			// we emit this signals in case their values were already set
			emit lengthChanged(m_length);
			emit framesPerSecondChanged(m_fps);
			emit playbackRateChanged(m_playbackRate);
			emit textStreamsChanged(m_textStreams);
			emit audioStreamsChanged(m_audioStreams);
			emit activeAudioStreamChanged(m_activeAudioStream);

			switch(m_state) {
			case VideoPlayer::Playing:
				emit playing();
				break;
			case VideoPlayer::Paused:
				emit paused();
				break;
			case VideoPlayer::Ready:
				emit stopped();
				break;
			default:
				break;
			}
		}
	} else if(m_state > VideoPlayer::Opening) {
		if(m_state != newState && newState > VideoPlayer::Opening) {
			m_state = newState;
			switch(m_state) {
			case VideoPlayer::Playing:
				m_videoWidget->videoLayer()->show();
				m_backend->setVolume(m_backendVolume);
				emit playing();
				break;
			case VideoPlayer::Paused:
				emit paused();
				break;
			case VideoPlayer::Ready:
				m_videoWidget->videoLayer()->hide();
				emit stopped();
				break;
			default:
				break;
			}
		}
	}
}

void
VideoPlayer::changePosition(double position)
{
	if(m_state <= VideoPlayer::Closed)
		return;

	if(position > m_length && m_length > 0)
		changeLength(position);

	if(m_position != position) {
		if(m_position <= 0 || m_minPositionDelta <= 0.0) {
			m_position = position;
			emit positionChanged(position);
		} else {
			double positionDelta = m_position - position;
			if(positionDelta >= m_minPositionDelta || -positionDelta >= m_minPositionDelta) {
				m_position = position;
				emit positionChanged(position);
			}
		}
	}
}

void
VideoPlayer::changeLength(double length)
{
	if(m_state <= VideoPlayer::Closed)
		return;

	if(length >= 0 && m_length != length) {
		m_length = length;
		emit lengthChanged(length);
	}
}

void
VideoPlayer::changePlaySpeed(double playbackRate)
{
	if(m_playbackRate != playbackRate) {
		m_playbackRate = playbackRate;
		emit playbackRateChanged(m_playbackRate);
	}
}

void
VideoPlayer::changeVolume(double volume)
{
	if(m_muted)
		return;

	if(volume < 0.0)
		volume = 0.0;
	else if(volume > 100.0)
		volume = 100.0;

	if(m_volume != volume) {
		m_volume = volume;
		emit volumeChanged(m_volume);
	}
}

void
VideoPlayer::changeMute(bool muted)
{
	if(m_muted != muted) {
		m_muted = muted;
		emit muteChanged(m_muted);
	}
}


bool
VideoPlayer::playOnLoad()
{
	const QWidget *topLevel = m_videoContainer->topLevelWidget();
	const QWidget *dockWaveform = topLevel->findChild<QWidget *>(QStringLiteral("waveform_dock"));
	const QWidget *dockVideo = topLevel->findChild<QWidget *>(QStringLiteral("player_dock"));
	return SCConfig::videoAutoPlay() && (dockVideo->isVisible() || dockWaveform->isVisible());
}

bool
VideoPlayer::openFile(const QString &filePath)
{
	if(m_state != VideoPlayer::Closed)
		return false;

	QFileInfo fileInfo(filePath);
	if(!fileInfo.exists() || !fileInfo.isFile() || !fileInfo.isReadable()) {
		emit fileOpenError(filePath, i18n("File does not exist."));   // operation will never succed
		return true;
	}

	m_filePath = filePath;
	m_state = VideoPlayer::Opening;

	m_videoWidget->videoLayer()->show();

	if(!m_backend->openFile(fileInfo.absoluteFilePath())) {
		resetState();
		emit fileOpenError(filePath, QString());
		return true;
	}

	if(m_state != VideoPlayer::Playing && playOnLoad())
		m_backend->play();
	else if(m_state != VideoPlayer::Paused && !playOnLoad())
		m_backend->pause();

	return true;
}

bool
VideoPlayer::closeFile()
{
	if(m_state <= VideoPlayer::Closed)
		return false;

	bool stop = m_state != VideoPlayer::Ready;
	if(stop)
		m_backend->stop(); // we can safely ignore the stop() return value here as we're about to close the file

	m_backend->closeFile();

	resetState();

	if(stop)
		emit stopped();

	emit fileClosed();

	return true;
}

bool
VideoPlayer::play()
{
	if(m_state <= VideoPlayer::Opening || m_state == VideoPlayer::Playing)
		return false;

	m_videoWidget->videoLayer()->show();

	if(!m_backend->play()) {
		resetState();
		emit playbackError();
	}

	return true;
}

bool
VideoPlayer::pause()
{
	if(m_state <= VideoPlayer::Opening || m_state == VideoPlayer::Paused)
		return false;

	if(!m_backend->pause()) {
		resetState();
		emit playbackError();
	}

	return true;
}

bool
VideoPlayer::togglePlayPaused()
{
	if(m_state <= VideoPlayer::Opening)
		return false;

	m_videoWidget->videoLayer()->show();

	bool hadError;
	if(m_state == VideoPlayer::Playing)
		hadError = !m_backend->pause();
	else
		hadError = !m_backend->play();

	if(hadError) {
		resetState();
		emit playbackError();
	}

	return true;
}

bool
VideoPlayer::seek(double seconds)
{
	if((m_state != VideoPlayer::Playing && m_state != VideoPlayer::Paused) || seconds < 0 || seconds > m_length)
		return false;

	if(seconds == m_position)
		return true;

	if(!m_backend->seek(seconds)) {
		resetState();
		emit playbackError();
	}

	return true;
}

bool
VideoPlayer::step(int frameOffset)
{
	if(m_state != VideoPlayer::Playing && m_state != VideoPlayer::Paused)
		return false;

	if(!m_backend->step(frameOffset)) {
		resetState();
		emit playbackError();
	}

	return true;
}

bool
VideoPlayer::stop()
{
	if(m_state <= VideoPlayer::Opening || m_state == VideoPlayer::Ready)
		return false;

	if(!m_backend->stop()) {
		resetState();
		emit playbackError();
		return true;
	}

	return true;
}

bool
VideoPlayer::selectAudioStream(int audioStreamIndex)
{
	if(m_state <= VideoPlayer::Opening || m_audioStreams.size() <= 1)
		return false;

	if(m_activeAudioStream == audioStreamIndex || audioStreamIndex < 0 || audioStreamIndex >= m_audioStreams.size())
		return false;

	m_activeAudioStream = audioStreamIndex;

	if(m_state != VideoPlayer::Ready) {
		if(!m_backend->selectAudioStream(audioStreamIndex)) {
			resetState();
			emit playbackError();
			return true;
		}
	}

	emit activeAudioStreamChanged(audioStreamIndex);
	return true;
}

void
VideoPlayer::playbackRate(double newRate)
{
	if(m_state != VideoPlayer::Playing || newRate < .125 || newRate > 128)
		return;

	m_backend->playbackRate(newRate);
}

void
VideoPlayer::increaseVolume(double amount)
{
	setVolume(m_volume + amount);
	setMuted(false);
}

void
VideoPlayer::decreaseVolume(double amount)
{
	setVolume(m_volume - amount);
}

void
VideoPlayer::setVolume(double volume)
{
	if(volume < 0.0)
		volume = 0.0;
	else if(volume > 100.0)
		volume = 100.0;

	if(m_volume != volume) {
		m_volume = volume;

		m_backendVolume = m_muted ? 0 : m_volume; //(m_backend->doesVolumeCorrection() ? m_volume : logarithmicVolume(m_volume));

		if(!m_muted && m_state == VideoPlayer::Playing) {
			if(!m_backend->setVolume(m_backendVolume)) {
				resetState();
				emit playbackError();
				return;
			}
		}

		emit volumeChanged(m_volume);
	}
}

void
VideoPlayer::setMuted(bool muted)
{
	if(m_muted != muted) {
		m_muted = muted;

		m_backendVolume = m_muted ? 0 : m_volume;//(m_backend->doesVolumeCorrection() ? m_volume : logarithmicVolume(m_volume));

		if(m_state == VideoPlayer::Playing) {
			if(!m_backend->setVolume(m_backendVolume)) {
				resetState();
				emit playbackError();
				return;
			}
		}

		emit muteChanged(m_muted);
	}
}
