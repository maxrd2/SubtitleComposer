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

#include "videoplayer.h"
#include "subtitletextoverlay.h"
#include "scconfig.h"

#include <math.h>

#include <QTimer>
#include <QFileInfo>
#include <QApplication>
#include <QEvent>

#include <QDebug>

#include <QtAVWidgets>

#include <KLocalizedString>

#define DEFAULT_MIN_POSITION_DELTA 0.02

using namespace SubtitleComposer;

namespace SubtitleComposer {
class VideoPlayerSubtitleOverlay : public QtAV::VideoFilter
{
    Q_OBJECT

public:
	VideoPlayerSubtitleOverlay(QWidget *parent = nullptr) : QtAV::VideoFilter(parent) {
		parent->installEventFilter(this);
	}
	virtual ~VideoPlayerSubtitleOverlay() {}

	bool isSupported(QtAV::VideoFilterContext::Type ct) const override {
		return ct == QtAV::VideoFilterContext::QtPainter || ct == QtAV::VideoFilterContext::X11;
	}

	bool eventFilter(QObject *object, QEvent *event) override {
		if(object == parent() && event->type() == QEvent::Resize)
			m_overlay.setImageSize(reinterpret_cast<QWidget *>(parent())->size());

		return QObject::eventFilter(object, event);
	}

protected:
    void process(QtAV::Statistics *statistics, QtAV::VideoFrame *frame) override {
		Q_UNUSED(statistics)
		Q_UNUSED(frame)
		if(!isEnabled())
			return;

		const QWidget *widget = reinterpret_cast<QWidget *>(parent());
		const QImage &img = m_overlay.image();
		const QSizeF imgSize(widget->width(), m_overlay.textSize().height());
		const qreal offY = widget->height() - widget->contentsMargins().bottom() - imgSize.height();
		context()->drawImage(QPointF(0, offY), img, QRectF(QPointF(), imgSize));
	}

private:
	friend class VideoPlayer;
	SubtitleTextOverlay m_overlay;
};
}



VideoPlayer::VideoPlayer()
	: m_renderer(new QtAV::VideoOutput(QtAV::VideoRendererId_Widget, this)),
	  m_player(new QtAV::AVPlayer(this)),
	  m_state(Initialized),
	  m_videoWidget(nullptr),
	  m_filePath(),
	  m_position(-1.0),
	  m_duration(-1.0),
	  m_fps(-1.0),
	  m_playbackRate(.0),
	  m_minPositionDelta(DEFAULT_MIN_POSITION_DELTA),
	  m_textStreams(),
	  m_activeAudioStream(-2),
	  m_audioStreams(),
	  m_muted(false),
	  m_volume(100.0),
	  m_overlay(nullptr)
{
	m_player->setRenderer(m_renderer);
	m_player->setNotifyInterval(10);
	m_player->setMediaEndAction(QtAV::MediaEndAction_Pause);

	// NOTE: Qt::QueuedConnection is used to avoid racing conditions in QtAV which cause position to stop updating
	connect(m_renderer, &QtAV::VideoOutput::videoFrameSizeChanged, this, &VideoPlayer::onResolutionChange);
	connect(m_player, &QtAV::AVPlayer::loaded, this, &VideoPlayer::onMediaLoaded, Qt::QueuedConnection);
	connect(m_player->audio(), &QtAV::AudioOutput::volumeChanged, this, &VideoPlayer::onVolumeChange);
	connect(m_player->audio(), &QtAV::AudioOutput::muteChanged, this, &VideoPlayer::onMuteChange);
	connect(m_player, &QtAV::AVPlayer::seekFinished, this, &VideoPlayer::onPositionChange, Qt::QueuedConnection);
	connect(m_player, &QtAV::AVPlayer::positionChanged, this, &VideoPlayer::onPositionChange, Qt::QueuedConnection);
	connect(m_player, &QtAV::AVPlayer::durationChanged, this, &VideoPlayer::onDurationChange);
	connect(m_player, &QtAV::AVPlayer::speedChanged, this, &VideoPlayer::onSpeedChange);
	connect(m_player, &QtAV::AVPlayer::stateChanged, this, &VideoPlayer::onStateChange);

	m_overlay = new VideoPlayerSubtitleOverlay(m_renderer->widget());
	m_renderer->installFilter(m_overlay);
}

void
VideoPlayer::onMediaLoaded()
{
	emit fileOpened(m_filePath);

	m_fps = m_player->statistics().video.frame_rate;
	m_minPositionDelta = 1.0 / m_fps;
	emit framesPerSecondChanged(m_fps);

	m_audioStreams.clear();
	const QVariantList &audioTracks = m_player->internalAudioTracks();
	for(auto i = audioTracks.cbegin(); i != audioTracks.cend(); ++i) {
		const QVariantMap &trk = i->toMap();
		QString audioStreamName = i18n("Audio Stream #%1", trk[QStringLiteral("id")].toInt());
		if(trk.contains(QStringLiteral("lang")))
			audioStreamName += QStringLiteral(": ") + trk[QStringLiteral("lang")].toString();
		if(trk.contains(QStringLiteral("title")))
			audioStreamName += QStringLiteral(": ") + trk[QStringLiteral("title")].toString();
		if(trk.contains(QStringLiteral("codec")))
			audioStreamName += QStringLiteral(" [") + trk[QStringLiteral("codec")].toString() + QStringLiteral("]");
		Q_ASSERT(m_audioStreams.size() == trk["id"]);
		m_audioStreams.append(audioStreamName);
	}
	emit audioStreamsChanged(m_audioStreams);

	if(m_duration <= 0) // we need video duration before notifying activeAudioStreamChanged
		VideoPlayer::onDurationChange(m_player->duration());
	m_activeAudioStream = m_player->currentAudioStream();
	emit activeAudioStreamChanged(m_activeAudioStream);

	m_textStreams.clear();
	const QVariantList &subTracks = m_player->internalSubtitleTracks();
	for(auto i = subTracks.cbegin(); i != subTracks.cend(); ++i) {
		const QVariantMap &trk = i->toMap();

		const QString &codec = trk[QStringLiteral("codec")].toString();
		if(codec != QStringLiteral("mov_text") && codec != QStringLiteral("subrip"))
			continue;

		QString textStreamName = i18n("Text Stream #%1", trk[QStringLiteral("id")].toInt());
		if(trk.contains(QStringLiteral("lang")))
			textStreamName += QStringLiteral(": ") + trk[QStringLiteral("lang")].toString();
		if(trk.contains(QStringLiteral("title")))
			textStreamName += QStringLiteral(": ") + trk[QStringLiteral("title")].toString();

		Q_ASSERT(m_textStreams.size() == trk["id"]);
		m_textStreams.append(textStreamName);
	}
	emit textStreamsChanged(m_textStreams);
}

void
VideoPlayer::onResolutionChange()
{
	const QSize fs = m_renderer->videoFrameSize();
	m_videoWidget->setVideoResolution(fs.width(), fs.height(), m_renderer->sourceAspectRatio());
}

void
VideoPlayer::onStateChange(QtAV::AVPlayer::State state)
{
	switch(state) {
	case QtAV::AVPlayer::StoppedState:
		if(m_state != Stopped) {
			m_state = Stopped;
			onSpeedChange(0.);
			emit stopped();
		}
		break;
	case QtAV::AVPlayer::PlayingState:
		if(m_state != Playing) {
			m_state = Playing;
			onSpeedChange(m_player->speed());
			emit playing();
		}
		break;
	case QtAV::AVPlayer::PausedState:
		if(m_state != Paused) {
			m_state = Paused;
			emit paused();
		}
		break;
	}
}

void
VideoPlayer::onPositionChange(qint64 position)
{
	const double pos = double(position) / 1000.;
	if(m_position == pos || pos >= m_duration)
		return;

	if(m_position <= 0 || m_minPositionDelta <= .0) {
		m_position = pos;
		emit positionChanged(pos);
	} else if(qAbs(m_position - pos) >= m_minPositionDelta) {
		m_position = pos;
		emit positionChanged(pos);
	}
}

void
VideoPlayer::onDurationChange(qint64 duration)
{
	const double dur = double(duration) / 1000.;
	if(m_duration == dur)
		return;

	m_duration = dur;
	emit lengthChanged(dur);
}

void
VideoPlayer::onSpeedChange(double speed)
{
	if(m_playbackRate == speed)
		return;

	m_playbackRate = speed;
	emit playbackRateChanged(m_playbackRate);
}

void
VideoPlayer::onVolumeChange(double volume)
{
	if(m_muted)
		return;

	Q_ASSERT(volume >= 0. && volume <= 1.);
	volume *= 100.;

	if(m_volume == volume)
		return;

	m_volume = volume;
	emit volumeChanged(m_volume);

}

void
VideoPlayer::onMuteChange(bool muted)
{
	if(m_muted == muted)
		return;

	m_muted = muted;
	emit muteChanged(m_muted);
}

VideoPlayer::~VideoPlayer()
{
	cleanup();
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
VideoPlayer::init(QWidget *videoContainer)
{
	// TODO: drop init() and reparent videoWidget elsewhere
	if(!m_videoWidget) {
		m_videoWidget = new VideoWidget(videoContainer);
		m_videoWidget->setVideoLayer(m_renderer->widget());

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
VideoPlayer::cleanup()
{
	m_renderer->widget()->setParent(nullptr);
}

void
VideoPlayer::reset()
{
	m_filePath.clear();

	m_position = -1.0;
	m_duration = -1.0;
	m_fps = -1.0;
	m_minPositionDelta = DEFAULT_MIN_POSITION_DELTA;

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
	Q_ASSERT(m_state == Initialized);

	QFileInfo fileInfo(filePath);
	if(!fileInfo.exists() || !fileInfo.isFile() || !fileInfo.isReadable()) {
		emit fileOpenError(filePath, i18n("File does not exist."));   // operation will never succeed
		return true;
	}

	m_filePath = filePath;
	m_state = Opening;

	m_videoWidget->videoLayer()->show();

	m_player->setFile(fileInfo.absoluteFilePath());

	if(playOnLoad())
		m_player->play();
	else
		m_player->load();

	return true;
}

bool
VideoPlayer::closeFile()
{
	if(m_state < Opening)
		return false;

	if(m_state != Stopped)
		m_player->stop();

	reset();

	emit fileClosed();

	return true;
}

bool
VideoPlayer::play()
{
	if(m_state <= Opening || m_state == Playing)
		return false;

	m_videoWidget->videoLayer()->show();

	if(m_state == Paused)
		m_player->togglePause();
	else
		m_player->play();

	return true;
}

bool
VideoPlayer::pause()
{
	if(m_state <= Opening || m_state == Paused)
		return false;

	m_player->togglePause();

	return true;
}

bool
VideoPlayer::togglePlayPaused()
{
	if(m_state <= Opening)
		return false;
	return m_state == Playing ? pause() : play();
}

bool
VideoPlayer::seek(double seconds)
{
	if(m_state <= Stopped || seconds < 0. || seconds > m_duration)
		return false;

	if(seconds == m_position)
		return true;

	m_player->setPosition(seconds * 1000.);
	return true;
}

bool
VideoPlayer::step(int frameOffset)
{
	if(m_state <= Stopped)
		return false;

	// m_player->stepForward()/stepBackward() are not working well, so we'll seek()

	if(m_state != Paused)
		m_player->pause();

	seek(m_position + double(frameOffset) / m_fps);
	return true;
}

bool
VideoPlayer::stop()
{
	if(m_state <= Stopped)
		return false;

	m_player->stop();

	return true;
}

bool
VideoPlayer::selectAudioStream(int audioStreamIndex)
{
	if(m_state <= VideoPlayer::Opening || m_audioStreams.size() <= 0)
		return false;

	if(audioStreamIndex < 0 || audioStreamIndex >= m_audioStreams.size())
		return false;

	if(m_activeAudioStream != audioStreamIndex) {
		m_activeAudioStream = audioStreamIndex;
		m_player->setAudioStream(audioStreamIndex);
		emit activeAudioStreamChanged(audioStreamIndex);
	}
	return true;
}

void
VideoPlayer::playbackRate(double newRate)
{
	if(m_state <= Opening || newRate < .125 || newRate > 128)
		return;

	m_player->setSpeed(newRate);
}

SubtitleTextOverlay &
VideoPlayer::subtitleOverlay()
{
	return m_overlay->m_overlay;
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

	if(m_volume == volume)
		return;

	m_volume = volume;

	m_player->audio()->setVolume(volume / 100.);

	emit volumeChanged(m_volume);
}

void
VideoPlayer::setMuted(bool muted)
{
	if(m_muted == muted)
		return;

	m_muted = muted;

	m_player->audio()->setMute(muted);
	emit muteChanged(m_muted);
}


#include "videoplayer.moc"
