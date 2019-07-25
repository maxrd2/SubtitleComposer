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
	m_activeBackend(0),
	m_widgetParent(0),
	m_applicationClosingDown(false),
	m_state(VideoPlayer::Uninitialized),
	m_videoWidget(NULL),
	m_filePath(),
	m_position(-1.0),
	m_savedPosition(-1.0),
	m_length(-1.0),
	m_framesPerSecond(-1.0),
	m_playbackRate(.0),
	m_minPositionDelta(DEFAULT_MIN_POSITION_DELTA),
	m_textStreams(),
	m_activeAudioStream(-1),
	m_audioStreams(),
	m_muted(false),
	m_volume(100.0),
	m_backendVolume(100.0),
	m_openFileTimer(new QTimer(this))
{
	PluginHelper<VideoPlayer, PlayerBackend> ph(this);
	ph.loadAll(QStringLiteral("videoplayerplugins"));
	for(auto p: m_plugins) {
		p->setSCConfig(SCConfig::self());
		p->setPlayer(this);
	}

	// the timeout might seem too much, but it only matters when the file couldn't be
	// opened, and it's better to have the user wait a bit in that case than showing
	// an error when there's nothing wrong with the file (a longer time might be needed
	// for example if the computer is under heavy load or is just slow)
	m_openFileTimer->setSingleShot(true);

	connect(m_openFileTimer, SIGNAL(timeout()), this, SLOT(onOpenFileTimeout()));
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
VideoPlayer::initialize(QWidget *widgetParent, const QString &prefBackendName)
{
	if(isInitialized()) {
		qCritical() << "VideoPlayer has already been initialized";
		return false;
	}

	m_widgetParent = widgetParent;

	if(m_plugins.contains(prefBackendName)) {
		// we first try to set the requested backend as active
		backendInitializePrivate(m_plugins[prefBackendName]);
	}
	// if that fails, we set the first available backend as active
	if(!m_activeBackend) {
		for(QMap<QString, PlayerBackend *>::ConstIterator it = m_plugins.begin(), end = m_plugins.end(); it != end; ++it)
			if(backendInitializePrivate(it.value()))
				break;
	}

	if(!m_activeBackend)
		qCritical() << "Failed to initialize a player backend";

	return m_activeBackend;
}

bool
VideoPlayer::reinitialize(const QString &prefBackendName)
{
	if(!isInitialized())
		return false;

	QString currentFile = m_filePath;

	PlayerBackend *targetBackend = m_plugins.contains(prefBackendName) ? m_plugins[prefBackendName] : m_activeBackend;

	finalize();

	if(!backendInitializePrivate(targetBackend)) {
		for(QMap<QString, PlayerBackend *>::ConstIterator it = m_plugins.begin(), end = m_plugins.end(); it != end; ++it)
			if(backendInitializePrivate(it.value()))
				break;
	}

	if(!m_activeBackend) {
		qCritical() << "Failed to initialize a player backend";
		return false;
	}

	if(!currentFile.isEmpty())
		openFile(currentFile);

	return true;
}

void
VideoPlayer::finalize()
{
	if(!isInitialized())
		return;

	PlayerBackend *wasActiveBackend = m_activeBackend;

	backendFinalize(m_activeBackend);

	m_state = VideoPlayer::Uninitialized;
	m_activeBackend = nullptr;

	emit backendFinalized(wasActiveBackend);
}

/*virtual*/ bool
VideoPlayer::reconfigure()
{
	if(!isInitialized() || !m_activeBackend)
		return false;

	return m_activeBackend->reconfigure();
}

bool
VideoPlayer::backendInitializePrivate(PlayerBackend *backend)
{
	if(m_activeBackend == backend)
		return true;

	if(m_activeBackend)
		return false;

	if(backendInitialize(backend, m_widgetParent)) {
		m_state = VideoPlayer::Initialized;
		m_activeBackend = backend;
		emit backendInitialized(backend);
	}

	return m_activeBackend == backend;
}

bool
VideoPlayer::isApplicationClosingDown() const
{
	return m_applicationClosingDown;
}

void
VideoPlayer::setApplicationClosingDown()
{
	m_applicationClosingDown = true;
}

QString
VideoPlayer::activeBackendName() const
{
	for(QMap<QString, PlayerBackend *>::ConstIterator it = m_plugins.begin(), end = m_plugins.end(); it != end; ++it)
		if(it.value() == m_activeBackend)
			return it.key();
	return QString();
}

QStringList
VideoPlayer::backendNames() const
{
	return m_plugins.keys();
}

bool
VideoPlayer::backendInitialize(PlayerBackend *backend, QWidget *widgetParent)
{
	Q_ASSERT(m_videoWidget == NULL);

	m_videoWidget = new VideoWidget(widgetParent);
	backend->initialize(m_videoWidget);

	connect(m_videoWidget, SIGNAL(destroyed()), this, SLOT(onVideoWidgetDestroyed()));
	connect(m_videoWidget, SIGNAL(doubleClicked(const QPoint &)), this, SIGNAL(doubleClicked(const QPoint &)));
	connect(m_videoWidget, SIGNAL(leftClicked(const QPoint &)), this, SIGNAL(leftClicked(const QPoint &)));
	connect(m_videoWidget, SIGNAL(rightClicked(const QPoint &)), this, SIGNAL(rightClicked(const QPoint &)));
	connect(m_videoWidget, SIGNAL(wheelUp()), this, SIGNAL(wheelUp()));
	connect(m_videoWidget, SIGNAL(wheelDown()), this, SIGNAL(wheelDown()));

	m_videoWidget->show();
	m_videoWidget->videoLayer()->hide();

	// NOTE: next is used to make videoWidgetParent update it's geometry
	QRect geometry = widgetParent->geometry();
	geometry.setHeight(geometry.height() + 1);
	widgetParent->setGeometry(geometry);

	return true;
}

void
VideoPlayer::backendFinalize(PlayerBackend *backend)
{
	closeFile();

	backend->finalize();

	if(m_videoWidget) {
		m_videoWidget->disconnect();
		m_videoWidget->hide();
		m_videoWidget->deleteLater();
		m_videoWidget = NULL;
	}
}

void
VideoPlayer::onVideoWidgetDestroyed()
{
	Q_ASSERT(m_state == VideoPlayer::Uninitialized);

	m_videoWidget = 0;

	finalize();
}

double
VideoPlayer::logarithmicVolume(double volume)
{
	static const double base = 4.0;
	static const double power = 1.0;
	static const double divisor = pow(base, power);

	double scaledVol = volume * power * power / 100.0;
	double factor = pow(base, scaledVol / power);

	return (scaledVol * factor / divisor) * (100.0 / (power * power));
}

const QStringList &
VideoPlayer::textStreams() const
{
	return m_textStreams;
}

const QStringList &
VideoPlayer::audioStreams() const
{
	static const QStringList emptyList;

	return m_state <= VideoPlayer::Opening ? emptyList : m_audioStreams;
}

void
VideoPlayer::resetState()
{
	if(m_openFileTimer->isActive())
		m_openFileTimer->stop();

	m_filePath.clear();

	m_position = -1.0;
	m_savedPosition = -1.0;
	m_length = -1.0;
	m_framesPerSecond = -1.0;
	m_minPositionDelta = DEFAULT_MIN_POSITION_DELTA;

	m_activeAudioStream = -1;
	m_textStreams.clear();
	m_audioStreams.clear();

	m_state = VideoPlayer::Closed;

	if(m_videoWidget)
		m_videoWidget->videoLayer()->hide();
}

void
VideoPlayer::notifyVolume(double volume)
{
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
VideoPlayer::notifyMute(bool muted)
{
	if(m_muted != muted) {
		m_muted = muted;
		emit muteChanged(m_muted);
	}
}

void
VideoPlayer::notifyPosition(double position)
{
	if(m_state <= VideoPlayer::Closed)
		return;

	if(position > m_length && m_length > 0)
		notifyLength(position);

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
VideoPlayer::notifyLength(double length)
{
	if(m_state <= VideoPlayer::Closed)
		return;

	if(length >= 0 && m_length != length) {
		m_length = length;
		emit lengthChanged(length);
	}
}

void
VideoPlayer::notifyFramesPerSecond(double framesPerSecond)
{
	if(m_state <= VideoPlayer::Closed)
		return;

	if(framesPerSecond > 0 && m_framesPerSecond != framesPerSecond) {
		m_framesPerSecond = framesPerSecond;
		m_minPositionDelta = 1.0 / framesPerSecond;
		emit framesPerSecondChanged(framesPerSecond);
	}
}

void
VideoPlayer::notifyTextStreams(const QStringList &textStreams)
{
	m_textStreams = textStreams;
	emit textStreamsChanged(m_textStreams);
}

void
VideoPlayer::notifyAudioStreams(const QStringList &audioStreams, int activeAudioStream)
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
VideoPlayer::notifyState(VideoPlayer::State newState)
{
	if(m_state == VideoPlayer::Opening) {
		if(newState > VideoPlayer::Opening) {
			m_openFileTimer->stop();

			m_state = newState;
			m_videoWidget->videoLayer()->show();
			activeBackend()->setVolume(m_backendVolume);

			emit fileOpened(m_filePath);

			// we emit this signals in case their values were already set
			emit lengthChanged(m_length);
			emit framesPerSecondChanged(m_framesPerSecond);
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
				activeBackend()->setVolume(m_backendVolume);
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
VideoPlayer::notifyErrorState(const QString &errorMessage)
{
	if(m_state < VideoPlayer::Opening)
		return;

	if(m_state == VideoPlayer::Opening) {
		resetState();
		emit fileOpenError(m_filePath, errorMessage);
	} else {
		activeBackend()->stop();
		m_state = VideoPlayer::Ready;
		emit playbacqCritical(errorMessage);
		emit stopped();
	}
}

bool
VideoPlayer::playOnLoad()
{
	const QWidget *topLevel = m_widgetParent->topLevelWidget();
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

	// FIXME add an option to set the error timeout amount when opening video
	m_openFileTimer->start(6000);

	m_videoWidget->videoLayer()->show();

	bool playingAfterCall = true;
	if(!activeBackend()->openFile(fileInfo.absoluteFilePath(), playingAfterCall)) {
		resetState();
		emit fileOpenError(filePath, QString());
		return true;
	}

	if(!playingAfterCall && playOnLoad())
		activeBackend()->play();
	else if(playingAfterCall && !playOnLoad())
		activeBackend()->pause();

	return true;
}

void
VideoPlayer::onOpenFileTimeout(const QString &reason)
{
	activeBackend()->stop();
	activeBackend()->closeFile();

	resetState();

	emit fileOpenError(m_filePath, reason);
}

bool
VideoPlayer::closeFile()
{
	if(m_state <= VideoPlayer::Closed)
		return false;

	bool stop = m_state != VideoPlayer::Ready;
	if(stop)
		activeBackend()->stop(); // we can safely ignore the stop() return value here as we're about to close the file

	activeBackend()->closeFile();

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

	if(!activeBackend()->play()) {
		resetState();
		emit playbacqCritical();
	}

	return true;
}

bool
VideoPlayer::pause()
{
	if(m_state <= VideoPlayer::Opening || m_state == VideoPlayer::Paused)
		return false;

	if(!activeBackend()->pause()) {
		resetState();
		emit playbacqCritical();
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
		hadError = !activeBackend()->pause();
	else
		hadError = !activeBackend()->play();

	if(hadError) {
		resetState();
		emit playbacqCritical();
	}

	return true;
}

bool
VideoPlayer::seek(double seconds, bool accurate)
{
	if((m_state != VideoPlayer::Playing && m_state != VideoPlayer::Paused) || seconds < 0 || seconds > m_length)
		return false;

	if(seconds == m_position)
		return true;

	if(!activeBackend()->seek(seconds, accurate)) {
		resetState();
		emit playbacqCritical();
	}

	return true;
}

bool
VideoPlayer::step(int frameOffset)
{
	if(m_state != VideoPlayer::Playing && m_state != VideoPlayer::Paused)
		return false;

	if(!activeBackend()->step(frameOffset)) {
		resetState();
		emit playbacqCritical();
	}

	return true;
}

void
VideoPlayer::seekToSavedPosition()
{
	if(m_savedPosition >= 0.0) {
		seek(m_savedPosition, true);
		m_savedPosition = -1.0;
	}
}

bool
VideoPlayer::stop()
{
	if(m_state <= VideoPlayer::Opening || m_state == VideoPlayer::Ready)
		return false;

	if(!activeBackend()->stop()) {
		resetState();
		emit playbacqCritical();
		return true;
	}

	return true;
}

bool
VideoPlayer::setActiveAudioStream(int audioStreamIndex)
{
	if(m_state <= VideoPlayer::Opening || m_audioStreams.size() <= 1)
		return false;

	if(m_activeAudioStream == audioStreamIndex || audioStreamIndex < 0 || audioStreamIndex >= m_audioStreams.size())
		return false;

	bool onTheFly;
	if(!activeBackend()->supportsChangingAudioStream(&onTheFly))
		return true;

	m_activeAudioStream = audioStreamIndex;

	if(m_state != VideoPlayer::Ready) {
		double savedPosition = m_position;

		if(!activeBackend()->setActiveAudioStream(audioStreamIndex)) {
			resetState();
			emit playbacqCritical();
			return true;
		}

		if(!onTheFly) {
			if(!activeBackend()->stop()) {
				resetState();
				emit playbacqCritical();
				return true;
			}

			if(savedPosition > 0.0) {
				if(!activeBackend()->play()) {
					resetState();
					emit playbacqCritical();
					return true;
				}

				m_savedPosition = savedPosition;
				QTimer::singleShot(500, this, SLOT(seekToSavedPosition()));
			}
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

	activeBackend()->playbackRate(newRate);
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

		m_backendVolume = m_muted ? 0 : (activeBackend()->doesVolumeCorrection() ? m_volume : logarithmicVolume(m_volume));

		if(!m_muted && m_state == VideoPlayer::Playing) {
			if(!activeBackend()->setVolume(m_backendVolume)) {
				resetState();
				emit playbacqCritical();
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

		m_backendVolume = m_muted ? 0 : (activeBackend()->doesVolumeCorrection() ? m_volume : logarithmicVolume(m_volume));

		if(m_state == VideoPlayer::Playing) {
			if(!activeBackend()->setVolume(m_backendVolume)) {
				resetState();
				emit playbacqCritical();
				return;
			}
		}

		emit muteChanged(m_muted);
	}
}
