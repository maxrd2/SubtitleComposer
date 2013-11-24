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

#include "player.h"
#include "playerbackend.h"

#ifdef HAVE_GSTREAMER
#include "gstreamer/gstreamerplayerbackend.h"
#endif
#include "mplayer/mplayerplayerbackend.h"
#include "phonon/phononplayerbackend.h"
#ifdef HAVE_XINE
#include "xine/xineplayerbackend.h"
#endif

#include <math.h>

#include <QtCore/QTimer>
#include <QtCore/QFileInfo>
#include <QApplication>

#include <KDebug>

#define DEFAULT_MIN_POSITION_DELTA 0.02

namespace SubtitleComposer {
class DummyPlayerBackend : public PlayerBackend
{
public:
	DummyPlayerBackend(Player *player) : PlayerBackend(player, "Dummy", new AppConfigGroup("Dummy", QMap<QString, QString>())) {}

	virtual ~DummyPlayerBackend() {}

	virtual AppConfigGroupWidget * newAppConfigGroupWidget(QWidget * /*parent */) { return 0; }

protected:
	virtual QWidget * initialize(QWidget *widgetParent) { return new VideoWidget(widgetParent); }

	virtual void finalize() {}

	virtual bool openFile(const QString &/*filePath*/, bool &/*playingAfterCall*/) { return false; }

	virtual void closeFile() {}

	virtual bool play() { return false; }

	virtual bool pause() { return false; }

	virtual bool seek(double /*seconds*/, bool /*accurate*/) { return false; }

	virtual bool stop() { return false; }

	virtual bool setActiveAudioStream(int /*audioStream*/) { return false; }

	virtual bool setVolume(double /*volume*/) { return false; }
};
}

using namespace SubtitleComposer;

Player::Player() :
	m_videoWidget(0),
	m_filePath(),
	m_position(-1.0),
	m_savedPosition(-1.0),
	m_length(-1.0),
	m_framesPerSecond(-1.0),
	m_minPositionDelta(DEFAULT_MIN_POSITION_DELTA),
	m_activeAudioStream(-1),
	m_audioStreams(),
	m_muted(false),
	m_volume(100.0),
	m_backendVolume(100.0),
	m_openFileTimer(new QTimer(this))
{
	addBackend(new DummyPlayerBackend(this));

#ifdef HAVE_GSTREAMER
	addBackend(new GStreamerPlayerBackend(this));
#endif

	addBackend(new MPlayerPlayerBackend(this));
	addBackend(new PhononPlayerBackend(this));

#ifdef HAVE_XINE
	addBackend(new XinePlayerBackend(this));
#endif

	// the timeout might seem too much, but it only matters when the file couldn't be
	// opened, and it's better to have the user wait a bit in that case than showing
	// an error when there's nothing wrong with the file (a longer time might be needed
	// for example if the computer is under heavy load or is just slow)

	m_openFileTimer->setSingleShot(true);

	connect(m_openFileTimer, SIGNAL(timeout()), this, SLOT(onOpenFileTimeout()));
}

Player::~Player()
{}

Player *
Player::instance()
{
	static Player player;

	return &player;
}

bool
Player::initializeBackend(ServiceBackend *backend, QWidget *widgetParent)
{
	if((m_videoWidget = static_cast<VideoWidget *>(backend->initialize(widgetParent)))) {
		connect(m_videoWidget, SIGNAL(destroyed()), this, SLOT(onVideoWidgetDestroyed()));
		connect(m_videoWidget, SIGNAL(doubleClicked(const QPoint &)), this, SIGNAL(doubleClicked(const QPoint &)));
		connect(m_videoWidget, SIGNAL(leftClicked(const QPoint &)), this, SIGNAL(leftClicked(const QPoint &)));
		connect(m_videoWidget, SIGNAL(rightClicked(const QPoint &)), this, SIGNAL(rightClicked(const QPoint &)));
		connect(m_videoWidget, SIGNAL(wheelUp()), this, SIGNAL(wheelUp()));
		connect(m_videoWidget, SIGNAL(wheelDown()), this, SIGNAL(wheelDown()));

		m_videoWidget->show();

		// NOTE: next is used to make videoWidgetParent update it's geometry
		QRect geometry = widgetParent->geometry();
		geometry.setHeight(geometry.height() + 1);
		widgetParent->setGeometry(geometry);

		return true;
	}

	return false;
}

void
Player::finalizeBackend(ServiceBackend *backend)
{
	closeFile();

	backend->finalize();

	if(m_videoWidget) {
		m_videoWidget->disconnect();
		m_videoWidget->hide();
		m_videoWidget->deleteLater();
		m_videoWidget = 0;
	}
}

void
Player::onVideoWidgetDestroyed()
{
	Q_ASSERT(m_state == Player::Uninitialized);

	m_videoWidget = 0;

	finalize();
}

double
Player::logarithmicVolume(double volume)
{
	static const double base = 4.0;
	static const double power = 1.0;
	static const double divisor = pow(base, power);

	double scaledVol = volume * power * power / 100.0;
	double factor = pow(base, scaledVol / power);

	return (scaledVol * factor / divisor) * (100.0 / (power * power));
}

const QStringList &
Player::audioStreams() const
{
	static const QStringList emptyList;

	return m_state <= Player::Opening ? emptyList : m_audioStreams;
}

void
Player::resetState()
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
	m_audioStreams.clear();

	m_state = Player::Closed;

	if(m_videoWidget)
		m_videoWidget->videoLayer()->hide();
}

void
Player::setPosition(double position)
{
	if(m_state <= Player::Closed)
		return;

	if(position > m_length && m_length > 0)
		setLength(position);

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
Player::setLength(double length)
{
	if(m_state <= Player::Closed)
		return;

	if(length >= 0 && m_length != length) {
		m_length = length;
		emit lengthChanged(length);
	}
}

void
Player::setFramesPerSecond(double framesPerSecond)
{
	if(m_state <= Player::Closed)
		return;

	if(framesPerSecond > 0 && m_framesPerSecond != framesPerSecond) {
		m_framesPerSecond = framesPerSecond;
		m_minPositionDelta = 1.0 / framesPerSecond;
		emit framesPerSecondChanged(framesPerSecond);
	}
}

void
Player::setAudioStreams(const QStringList &audioStreams, int activeAudioStream)
{
	if(m_state <= Player::Closed)
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
Player::setState(Player::State newState)
{
	if(m_state == Player::Opening) {
		if(newState == Player::Playing) {
			m_openFileTimer->stop();

			m_state = Player::Playing;
			m_videoWidget->videoLayer()->show();
			activeBackend()->setVolume(m_backendVolume);

			emit fileOpened(m_filePath);

			// we emit this signals in case their values were already set
			emit lengthChanged(m_length);
			emit framesPerSecondChanged(m_framesPerSecond);
			emit audioStreamsChanged(m_audioStreams);
			emit activeAudioStreamChanged(m_activeAudioStream);

			emit playing();
		}
	} else if(m_state > Player::Opening) {
		if(m_state != newState && newState > Player::Opening) {
			m_state = newState;
			switch(m_state) {
			case Player::Playing:
				m_videoWidget->videoLayer()->show();
				activeBackend()->setVolume(m_backendVolume);
				emit playing();
				break;
			case Player::Paused:
				emit paused();
				break;
			case Player::Ready:
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
Player::setErrorState(const QString &errorMessage)
{
	if(!isInitialized())
		return;

	if(m_state <= Player::Opening) {
		m_openFileTimer->stop();
		QString filePath(m_filePath);
		resetState();
		emit fileOpenError(filePath);
	} else {
		activeBackend()->stop();
		m_state = Player::Ready;
		emit playbackError(errorMessage);
		emit stopped();
	}
}

bool
Player::openFile(const QString &filePath)
{
	if(m_state != Player::Closed)
		return false;

	QFileInfo fileInfo(filePath);
	if(!fileInfo.exists() || !fileInfo.isFile() || !fileInfo.isReadable()) {
		emit fileOpenError(filePath);   // operation will never succed
		return true;
	}

	m_filePath = filePath;
	m_state = Player::Opening;

	// FIXME add an option to set the error timeout amount when opening video
	m_openFileTimer->start(6000);

	bool playingAfterCall = true;
	if(!activeBackend()->openFile(fileInfo.absoluteFilePath(), playingAfterCall)) {
		resetState();
		emit fileOpenError(filePath);
		return true;
	}

	if(!playingAfterCall)
		activeBackend()->play();

	return true;
}

void
Player::onOpenFileTimeout()
{
	QString filePath(m_filePath);

	activeBackend()->stop();
	activeBackend()->closeFile();

	resetState();

	emit fileOpenError(filePath);
}

bool
Player::closeFile()
{
	if(m_state <= Player::Closed)
		return false;

	bool stop = m_state != Player::Ready;
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
Player::play()
{
	if(m_state <= Player::Opening || m_state == Player::Playing)
		return false;

	if(!activeBackend()->play()) {
		resetState();
		emit playbackError();
	}

	return true;
}

bool
Player::pause()
{
	if(m_state <= Player::Opening || m_state == Player::Paused)
		return false;

	if(!activeBackend()->pause()) {
		resetState();
		emit playbackError();
	}

	return true;
}

bool
Player::togglePlayPaused()
{
	if(m_state <= Player::Opening)
		return false;

	bool hadError;
	if(m_state == Player::Playing)
		hadError = !activeBackend()->pause();
	else
		hadError = !activeBackend()->play();

	if(hadError) {
		resetState();
		emit playbackError();
	}

	return true;
}

bool
Player::seek(double seconds, bool accurate)
{
	if((m_state != Player::Playing && m_state != Player::Paused) || seconds < 0 || seconds > m_length)
		return false;

	if(seconds == m_position)
		return true;

	if(!activeBackend()->seek(seconds, accurate)) {
		resetState();
		emit playbackError();
	}

	return true;
}

void
Player::seekToSavedPosition()
{
	if(m_savedPosition >= 0.0) {
		seek(m_savedPosition, true);
		m_savedPosition = -1.0;
	}
}

bool
Player::stop()
{
	if(m_state <= Player::Opening || m_state == Player::Ready)
		return false;

	if(!activeBackend()->stop()) {
		resetState();
		emit playbackError();
		return true;
	}

	return true;
}

bool
Player::setActiveAudioStream(int audioStreamIndex)
{
	if(m_state <= Player::Opening || m_audioStreams.size() <= 1)
		return false;

	if(m_activeAudioStream == audioStreamIndex || audioStreamIndex < 0 || audioStreamIndex >= m_audioStreams.size())
		return false;

	bool onTheFly;
	if(!activeBackend()->supportsChangingAudioStream(&onTheFly))
		return true;

	m_activeAudioStream = audioStreamIndex;

	if(m_state != Player::Ready) {
		double savedPosition = m_position;

		if(!activeBackend()->setActiveAudioStream(audioStreamIndex)) {
			resetState();
			emit playbackError();
			return true;
		}

		if(!onTheFly) {
			if(!activeBackend()->stop()) {
				resetState();
				emit playbackError();
				return true;
			}

			if(savedPosition > 0.0) {
				if(!activeBackend()->play()) {
					resetState();
					emit playbackError();
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
Player::increaseVolume(double amount)
{
	setVolume(m_volume + amount);
	setMuted(false);
}

void
Player::decreaseVolume(double amount)
{
	setVolume(m_volume - amount);
}

void
Player::setVolume(double volume)
{
	if(volume < 0.0)
		volume = 0.0;
	else if(volume > 100.0)
		volume = 100.0;

	if(m_volume != volume) {
		m_volume = volume;

		m_backendVolume = m_muted ? 0 : (activeBackend()->doesVolumeCorrection() ? m_volume : logarithmicVolume(m_volume));

		if(!m_muted && m_state == Player::Playing) {
			if(!activeBackend()->setVolume(m_backendVolume)) {
				resetState();
				emit playbackError();
				return;
			}
		}

		emit volumeChanged(m_volume);
	}
}

void
Player::setMuted(bool muted)
{
	if(m_muted != muted) {
		m_muted = muted;

		m_backendVolume = m_muted ? 0 : (activeBackend()->doesVolumeCorrection() ? m_volume : logarithmicVolume(m_volume));

		if(m_state == Player::Playing) {
			if(!activeBackend()->setVolume(m_backendVolume)) {
				resetState();
				emit playbackError();
				return;
			}
		}

		emit muteChanged(m_muted);
	}
}

#include "player.moc"
