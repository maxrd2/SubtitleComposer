/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "videoplayer.h"
#include "scconfig.h"

#include "videoplayer/backend/glrenderer.h"

#include <math.h>

#include <QTimer>
#include <QFileInfo>
#include <QApplication>
#include <QEvent>

#include <QDebug>

#include <KLocalizedString>

#define VOLUME_MULTIPLIER (double(qMax(1, SCConfig::volumeAmplification())) / 3.)

using namespace SubtitleComposer;


VideoPlayer::VideoPlayer()
	: m_player(new FFPlayer(this)),
	  m_videoWidget(nullptr),
	  m_filePath(),
	  m_state(Initialized),
	  m_duration(-1.0),
	  m_fps(-1.0),
	  m_playSpeed(.0),
	  m_textStreams(),
	  m_activeAudioStream(-2),
	  m_audioStreams(),
	  m_muted(false),
	  m_volume(100.0)
{
	m_player->renderer()->setOverlay(&m_subOverlay);

	setupNotifications();
}

VideoPlayer::~VideoPlayer()
{
}

VideoPlayer *
VideoPlayer::instance()
{
	static VideoPlayer *player = nullptr;
	if(!player)
		player = new VideoPlayer();
	return player;
}

bool
VideoPlayer::init(QWidget *videoContainer)
{
	if(!m_videoWidget) {
		m_videoWidget = new VideoWidget(videoContainer);
		m_videoWidget->setVideoLayer(m_player->renderer());

		connect(m_videoWidget, &VideoWidget::doubleClicked, this, &VideoPlayer::doubleClicked);
		connect(m_videoWidget, &VideoWidget::rightClicked, this, &VideoPlayer::rightClicked);
		connect(m_videoWidget, &VideoWidget::leftClicked, this, &VideoPlayer::leftClicked);
		connect(m_videoWidget, &VideoWidget::wheelUp, this, &VideoPlayer::wheelUp);
		connect(m_videoWidget, &VideoWidget::wheelDown, this, &VideoPlayer::wheelDown);
	} else {
		m_videoWidget->setParent(videoContainer);
	}
	reset();

	return true;
}

void
VideoPlayer::reset()
{
	m_filePath.clear();

	m_duration = -1.0;
	m_fps = -1.0;

	m_activeAudioStream = -2;
	m_textStreams.clear();
	m_audioStreams.clear();

	m_state = Initialized;

	if(m_videoWidget)
		m_videoWidget->videoLayer()->hide();
}

bool
VideoPlayer::playOnLoad()
{
	const QWidget *topLevel = m_videoWidget->topLevelWidget();
	const QWidget *dockWaveform = topLevel->findChild<QWidget *>(QStringLiteral("waveform_dock"));
	const QWidget *dockVideo = topLevel->findChild<QWidget *>(QStringLiteral("player_dock"));
	return SCConfig::videoAutoPlay() && (dockVideo->isVisible() || dockWaveform->isVisible());
}

bool
VideoPlayer::openFile(const QString &filePath)
{
	Q_ASSERT(m_state == Initialized || m_state == Stopped);

	QFileInfo fileInfo(filePath);
	if(!fileInfo.exists() || !fileInfo.isFile() || !fileInfo.isReadable()) {
		emit fileOpenError(filePath, i18n("File does not exist."));   // operation will never succeed
		return false;
	}

	m_filePath = filePath;
	m_state = Opening;

	if(!m_player->open(fileInfo.absoluteFilePath().toUtf8()))
		return false;

	if(m_player->paused() == playOnLoad())
		m_player->pauseToggle();

	return true;
}

void
VideoPlayer::closeFile()
{
	if(m_state < Opening)
		return;
	m_player->close();
	reset();
	emit fileClosed();
}

void
VideoPlayer::play()
{
	if(m_state <= Opening || m_state == Playing)
		return;
	if(m_state < Playing)
		openFile(m_filePath);
	if(m_state == Paused)
		m_player->pauseToggle();
}

void
VideoPlayer::pause()
{
	if(m_state <= Opening || m_state == Paused)
		return;
	m_player->pauseToggle();
}

void
VideoPlayer::togglePlayPaused()
{
	if(m_state == Playing)
		pause();
	else if(m_state > Opening)
		play();
}

bool
VideoPlayer::seek(double seconds)
{
	if(m_state <= Stopped)
		return false;
	m_player->seek(qBound(0., seconds, m_duration));
	return true;
}

bool
VideoPlayer::step(int frameOffset)
{
	if(m_state <= Stopped)
		return false;
	m_player->stepFrame(frameOffset);
	return true;
}

bool
VideoPlayer::stop()
{
	if(m_state <= Stopped)
		return false;
	m_player->close();
	return true;
}

bool
VideoPlayer::selectAudioStream(int streamIndex)
{
	if(m_state <= VideoPlayer::Opening || streamIndex < 0 || streamIndex >= m_audioStreams.size())
		return false;

	if(m_activeAudioStream != streamIndex) {
		m_activeAudioStream = streamIndex;
		m_player->activeAudioStream(streamIndex);
		emit activeAudioStreamChanged(streamIndex);
	}
	return true;
}

void
VideoPlayer::playSpeed(double newRate)
{
	if(m_state <= Opening || newRate < .05 || newRate > 8.)
		return;

	m_player->setSpeed(newRate);
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
	volume = qBound(0., volume, 100.);
	m_player->setVolume(VOLUME_MULTIPLIER * volume / 100.);

	if(m_volume != volume)
		emit volumeChanged(m_volume = volume);
}

void
VideoPlayer::setMuted(bool muted)
{
	m_player->setMuted(muted);

	if(m_muted == muted)
		return;

	m_muted = muted;

	emit muteChanged(m_muted);
}

void
VideoPlayer::setupNotifications()
{
	connect(m_player, &FFPlayer::mediaLoaded, this, [this](){
		m_player->renderer()->reset();
		emit fileOpened(m_filePath);
		m_fps = m_player->videoFPS();
		emit fpsChanged(m_fps);
		m_videoWidget->videoLayer()->show();
	});
	connect(m_player, &FFPlayer::stateChanged, this, [this](FFPlayer::State ffs){
		if(m_state == Initialized) // video file is closed don't notify play/pause/stop
			return;
		static const QMap<FFPlayer::State, State> stateMap
			{{ FFPlayer::Stopped, Stopped }, { FFPlayer::Playing, Playing }, { FFPlayer::Paused, Paused }};
		const State state = stateMap[ffs];
		if(m_state != state) switch(m_state = state) {
		case Stopped: emit stopped(); break;
		case Playing: emit playing(); break;
		case Paused: emit paused(); break;
		default: break; // not possible
		}
	});
	connect(m_player->renderer(), &GLRenderer::resolutionChanged, this, [this](){
		m_videoWidget->setVideoResolution(m_player->videoWidth(), m_player->videoHeight(), m_player->videoSAR());
		m_videoWidget->videoLayer()->show();
	});

	connect(m_player, &FFPlayer::positionChanged, this, &VideoPlayer::positionChanged);

	connect(m_player, &FFPlayer::durationChanged, this, [this](double dur){
		if(m_duration != dur) emit durationChanged(m_duration = dur);
	});
	connect(m_player, &FFPlayer::speedChanged, this, [this](double speed){
		if(m_playSpeed != speed) emit playSpeedChanged(m_playSpeed = speed);
	});

	connect(m_player, &FFPlayer::volumeChanged, this, [this](double volume){
		if(m_volume != (volume = volume * 100. / VOLUME_MULTIPLIER)) {
			m_volume = volume;
			if(!m_muted) emit volumeChanged(m_volume);
		}
	});
	connect(m_player, &FFPlayer::muteChanged, this, [this](bool muted){
		if(m_muted != muted) emit muteChanged(m_muted = muted);
	});

	//connect(m_player, &FFPlayer::videoStreamsChanged, this, [this](const QStringList &streams){});
	connect(m_player, &FFPlayer::audioStreamsChanged, this, [this](const QStringList &streams){
		emit audioStreamsChanged(m_audioStreams = streams);
		emit activeAudioStreamChanged(m_activeAudioStream = m_player->activeAudioStream());
	});
	connect(m_player, &FFPlayer::subtitleStreamsChanged, this, [this](const QStringList &streams){
		emit textStreamsChanged(m_textStreams = streams);
	});
}
