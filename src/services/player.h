#ifndef PLAYER_H
#define PLAYER_H

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "service.h"
#include "videowidget.h"

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QMap>
#include <QtGui/QWidget>

class QTimer;

namespace SubtitleComposer {
class PlayerBackend;

class Player : public Service
{
	Q_OBJECT

public:
	typedef enum {
		Closed = Service::Initialized,          // same as Initialized
		Opening,
		// Opened, same as >Opening
		Playing,
		Paused,
		Ready                                   // same as Stopped or Finished
	} State;

	virtual QString dummyBackendName() const { return "Dummy"; }

	inline PlayerBackend * backend(const QString &name) const { return (PlayerBackend *)Service::backend(name); }

	inline PlayerBackend * activeBackend() const { return (PlayerBackend *)Service::activeBackend(); }

	static Player * instance();

	inline VideoWidget * videoWidget();

	inline const QString & filePath() const;

	inline bool isPlaying() const;
	inline bool isPaused() const;
	inline double position() const;
	inline double length() const;
	inline double framesPerSecond() const;
	inline bool isStopped() const;

	inline double volume() const;
	inline bool isMuted() const;
	inline int activeAudioStream() const;
	const QStringList & audioStreams() const;

public slots:
	// return values of these functions don't imply that the operation was performed OK
	// but that it was allowed (a false return value means that nothing was attempted).
	bool openFile(const QString &filePath);
	bool closeFile();

	bool play();
	bool pause();
	bool togglePlayPaused();
	bool seek(double seconds, bool accurate);
	bool stop();
	bool setActiveAudioStream(int audioStreamIndex);

	void increaseVolume(double amount = 3.0);
	void decreaseVolume(double amount = 3.0);
	void setVolume(double volume);          // value from 0.0 to 100.0 (inclusive)

	void setMuted(bool mute);

signals:
	void fileOpenError(const QString &filePath);
	void fileOpened(const QString &filePath);
	void fileClosed();

	void playbackError(const QString &errorMessage = QString());
	void playing();
	void positionChanged(double seconds);
	void lengthChanged(double seconds);
	void framesPerSecondChanged(double fps);
	void paused();
	void stopped();
	void activeAudioStreamChanged(int audioStreamIndex);
	void audioStreamsChanged(const QStringList &audioStreams);

	void volumeChanged(double volume);
	void muteChanged(bool muted);

	void doubleClicked(const QPoint &point);
	void rightClicked(const QPoint &point);
	void leftClicked(const QPoint &point);
	void wheelUp();
	void wheelDown();

protected:
	/**
	 * @brief initializeBackend - attempts to initialize the backend, making it the active backend.
	 * @param backend
	 * @param widgetParent
	 * @return true if backend is the active backend after the call; false if there was already another backend initialized
	 */
	virtual bool initializeBackend(ServiceBackend *backend, QWidget *widgetParent);

	/**
	 * @brief finalizeBackend - finalizes the active backend, leaving no active backend.
	 * @param backend
	 * returns??? the previously initialized backend (or 0 if there was none).
	 */
	virtual void finalizeBackend(ServiceBackend *backend);

private:
	Player();
	virtual ~Player();

	static double logarithmicVolume(double percentage);

	void resetState();

	// functions used by the backends to inform changes in state:

	void setPosition(double position);              // value in seconds
	void setLength(double length);          // value in seconds

	void setState(Player::State state);
	void setErrorState(const QString &errorMessage = QString());

	void setFramesPerSecond(double framesPerSecond);
	void setAudioStreams(const QStringList &audioStreams, int activeAudioStream);

private slots:
	void seekToSavedPosition();

	/** is the videoWidget() gets destroyed before the player, we finalize the player */
	void onVideoWidgetDestroyed();

	/** called if the player fails to set the state to Playing after opening the file */
	void onOpenFileTimeout();

private:
	VideoWidget *m_videoWidget;

	QString m_filePath;

	double m_position;
	double m_savedPosition;
	double m_length;
	double m_framesPerSecond;
	double m_minPositionDelta;
	int m_activeAudioStream;
	QStringList m_audioStreams;

	bool m_muted;
	double m_volume;
	double m_backendVolume;

	QTimer *m_openFileTimer;

	friend class PlayerBackend;
};

VideoWidget *
Player::videoWidget()
{
	return m_videoWidget;
}

const QString &
Player::filePath() const
{
	return m_filePath;
}

bool
Player::isPlaying() const
{
	return m_state == Player::Playing;
}

bool
Player::isPaused() const
{
	return m_state == Player::Paused;
}

double
Player::position() const
{
	return m_state <= Player::Opening ? -1.0 : (m_state == Player::Ready ? 0.0 : m_position);
}

double
Player::length() const
{
	return m_state <= Player::Opening ? -1.0 : m_length;
}

double
Player::framesPerSecond() const
{
	return m_state <= Player::Opening ? -1.0 : m_framesPerSecond;
}

bool
Player::isStopped() const
{
	return m_state == Player::Ready;
}

double
Player::volume() const
{
	return m_volume;
}

bool
Player::isMuted() const
{
	return m_muted;
}

int
Player::activeAudioStream() const
{
	return m_state <= Player::Opening ? -1 : m_activeAudioStream;
}
}
#endif
