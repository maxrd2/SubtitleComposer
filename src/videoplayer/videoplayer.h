#ifndef PLAYER_H
#define PLAYER_H

/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2017 Mladen Milinkovic <max@smoothware.net>
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

#include "videowidget.h"

#include <QString>
#include <QStringList>
#include <QMap>
#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QTimer)

namespace SubtitleComposer {
class PlayerBackend;

class VideoPlayer : public QObject
{
	Q_OBJECT

public:
	typedef enum {
		Uninitialized = 0,
		Initialized,
		Closed = Initialized,
		Opening,
		// Opened, same as >Opening
		Playing,
		Paused,
		Ready                                   // same as Stopped or Finished
	} State;

	static VideoPlayer * instance();

	/**
	 * @brief initialize - attempts to initialize the backend defined by prefBackendName; if that fails, attempts to initialize any other.
	 * @param widgetParent
	 * @param prefBackendName
	 * @return false if there was already an initialized backend or none could be initialized; true otherwise
	 */
	virtual bool initialize(QWidget *widgetParent, const QString &prefBackendName = QString());

	/**
	 * @brief reinitialize - finalizes the active backend and attempts to initialize the one defined by
	 *  prefBackendName (or the active one if is not valid a valid name); if that fails, attempts to
	 *  initialize any other backend.
	 * @param prefBackendName
	 * @return false if there was no initialized backend or none could be initialized; true otherwise
	 */
	virtual bool reinitialize(const QString &prefBackendName = QString());

	/**
	 * @brief finalize - finalizes the active backend
	 */
	virtual void finalize();

	/**
	 * @brief reconfigure - re-read active backend configuration
	 * @return false if there is no active backend or if error occured
	 */
	virtual bool reconfigure();

	/**
	 * @brief dummyBackendName - players should provide a dummy backend (one that implements its
	 *  operations as noops) so that the application can act reasonably even in absence of (real)
	 *  supported backends.
	 * @return
	 */
	virtual QString dummyBackendName() const { return QStringLiteral("Dummy"); }

	QString activeBackendName() const;
	QStringList backendNames() const;

	inline bool isActiveBackendDummy() const;

	inline PlayerBackend * backend(const QString &name) const;
	inline PlayerBackend * activeBackend() const;

	/**
	 * @brief isApplicationClosingDown - Indicates that the application is closing down and the backends
	 *  shouldn't rely on it for some things (such as processing events).
	 * @return
	 */
	bool isApplicationClosingDown() const;

	inline State state() const;
	inline bool isInitialized() const;

	inline VideoWidget * videoWidget();

	inline const QString & filePath() const;

	inline bool isPlaying() const;
	inline bool isPaused() const;
	inline double position() const;
	inline double length() const;
	inline double framesPerSecond() const;
	inline double playbackRate() const;
	void playbackRate(double newRate);

	inline bool isStopped() const;

	inline double volume() const;
	inline bool isMuted() const;
	const QStringList & textStreams() const;
	inline int activeAudioStream() const;
	const QStringList & audioStreams() const;

	bool playOnLoad();

public slots:
	/**
	 * @brief setApplicationClosingDown - Used to indicate the active backend that the application is closing down
	 */
	void setApplicationClosingDown();

	/**
	 * @brief return values of open/closeFile() don't imply that the operation was performed OK but that
	 * it was attempted. Error/success could be signaled later through fileOpenError/fileOpened() signals
	 */
	bool openFile(const QString &filePath);
	bool closeFile();

	bool play();
	bool pause();
	bool togglePlayPaused();
	bool seek(double seconds, bool accurate);
	bool step(int frameOffset);
	bool stop();
	bool setActiveAudioStream(int audioStreamIndex);

	void increaseVolume(double amount = 3.0);
	void decreaseVolume(double amount = 3.0);
	void setVolume(double volume);          // value from 0.0 to 100.0 (inclusive)

	void setMuted(bool mute);

signals:
	void backendInitialized(PlayerBackend *playerBackend);
	void backendFinalized(PlayerBackend *playerBackend);

	void fileOpenError(const QString &filePath, const QString &reason);
	void fileOpened(const QString &filePath);
	void fileClosed();

	void playbacqCritical(const QString &errorMessage = QString());
	void playing();
	void positionChanged(double seconds);
	void lengthChanged(double seconds);
	void framesPerSecondChanged(double fps);
	void playbackRateChanged(double rate);
	void paused();
	void stopped();
	void textStreamsChanged(const QStringList &textStreams);
	void activeAudioStreamChanged(int audioStreamIndex);
	void audioStreamsChanged(const QStringList &audioStreams);

	void volumeChanged(double volume);
	void muteChanged(bool muted);

	void doubleClicked(const QPoint &point);
	void rightClicked(const QPoint &point);
	void leftClicked(const QPoint &point);
	void wheelUp();
	void wheelDown();

private:
	VideoPlayer();
	virtual ~VideoPlayer();

	/**
	 * @brief initializeBackend - attempts to initialize the backend, making it the active backend.
	 * @param backend
	 * @param widgetParent
	 * @return true if backend is the active backend after the call; false if there was already another backend initialized
	 */
	virtual bool backendInitialize(PlayerBackend *backend, QWidget *widgetParent);

	/**
	 * @brief finalizeBackend - finalizes the active backend, leaving no active backend.
	 * @param backend
	 * returns??? the previously initialized backend (or NULL if there was none)
	 */
	virtual void backendFinalize(PlayerBackend *backend);

	PlayerBackend * backendLoad(const QString &pluginPath);

	void backendAdd(PlayerBackend *backend);

	bool backendInitializePrivate(PlayerBackend *backend);

	static double logarithmicVolume(double percentage);

	void resetState();

	// functions used by the backends to inform changes in state:

	void notifyVolume(double volume);
	void notifyMute(bool muted);

	void notifyPosition(double position);              // value in seconds
	void notifyLength(double length);          // value in seconds

	void notifyState(VideoPlayer::State state);
	void notifyErrorState(const QString &errorMessage = QString());

	void notifyFramesPerSecond(double framesPerSecond);
	void notifyTextStreams(const QStringList &textStreams);
	void notifyAudioStreams(const QStringList &audioStreams, int activeAudioStream);

private slots:
	void seekToSavedPosition();

	/** is the videoWidget() gets destroyed before the player, we finalize the player */
	void onVideoWidgetDestroyed();

	/** called if the player fails to set the state to Playing after opening the file */
	void onOpenFileTimeout(const QString &reason = QString());

private:
	QMap<QString, PlayerBackend *> m_backends;
	PlayerBackend *m_activeBackend;
	QWidget *m_widgetParent;

	bool m_applicationClosingDown;

	State m_state;

	VideoWidget *m_videoWidget;

	QString m_filePath;

	double m_position;
	double m_savedPosition;
	double m_length;
	double m_framesPerSecond;
	double m_playbackRate;
	double m_minPositionDelta;
	QStringList m_textStreams;
	int m_activeAudioStream;
	QStringList m_audioStreams;

	bool m_muted;
	double m_volume;
	double m_backendVolume;

	QTimer *m_openFileTimer;

	friend class PlayerBackend;
};

VideoPlayer::State
VideoPlayer::state() const
{
	return m_state;
}

bool
VideoPlayer::isInitialized() const
{
	return m_state >= VideoPlayer::Initialized;
}

PlayerBackend *
VideoPlayer::activeBackend() const
{
	return m_activeBackend;
}

PlayerBackend *
VideoPlayer::backend(const QString &backendName) const
{
	return m_backends.contains(backendName) ? m_backends[backendName] : 0;
}

bool
VideoPlayer::isActiveBackendDummy() const
{
	return activeBackendName() == dummyBackendName();
}

VideoWidget *
VideoPlayer::videoWidget()
{
	return m_videoWidget;
}

const QString &
VideoPlayer::filePath() const
{
	return m_filePath;
}

bool
VideoPlayer::isPlaying() const
{
	return m_state == VideoPlayer::Playing;
}

bool
VideoPlayer::isPaused() const
{
	return m_state == VideoPlayer::Paused;
}

double
VideoPlayer::position() const
{
	return m_state <= VideoPlayer::Opening ? -1.0 : (m_state == VideoPlayer::Ready ? 0.0 : m_position);
}

double
VideoPlayer::length() const
{
	return m_state <= VideoPlayer::Opening ? -1.0 : m_length;
}

double
VideoPlayer::framesPerSecond() const
{
	return m_state <= VideoPlayer::Opening ? -1.0 : m_framesPerSecond;
}

double
VideoPlayer::playbackRate() const
{
	return m_state != VideoPlayer::Playing ? .0 : m_playbackRate;
}


bool
VideoPlayer::isStopped() const
{
	return m_state == VideoPlayer::Ready;
}

double
VideoPlayer::volume() const
{
	return m_volume;
}

bool
VideoPlayer::isMuted() const
{
	return m_muted;
}

int
VideoPlayer::activeAudioStream() const
{
	return m_state <= VideoPlayer::Opening ? -1 : m_activeAudioStream;
}
}
#endif
