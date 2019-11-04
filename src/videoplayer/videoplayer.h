#ifndef PLAYER_H
#define PLAYER_H

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

	template <class C, class T> friend class PluginHelper;
	friend class PlayerBackend;

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
	 * @param videoContainer
	 * @param backendName
	 * @return false if there was already an initialized backend or none could be initialized; true otherwise
	 */
	bool init(QWidget *videoContainer, const QString &backendName = QString());

	/**
	 * @brief reinitialize - finalizes the active backend and attempts to initialize the one defined by
	 *  prefBackendName (or the active one if is not valid a valid name); if that fails, attempts to
	 *  initialize any other backend.
	 * @param backendName
	 * @return false if there was no initialized backend or none could be initialized; true otherwise
	 */
	bool switchBackend(const QString &backendName = QString());

	/**
	 * @brief finalize - finalizes the active backend
	 */
	void cleanup();

	inline const PlayerBackend *backend() const { return m_backend; }
	inline const QMap<QString, PlayerBackend *> & plugins() const { return m_plugins; }

	inline State state() const { return m_state; }
	inline bool isInitialized() const { return m_state >= VideoPlayer::Initialized; }

	inline VideoWidget * videoWidget() { return m_videoWidget; }

	inline const QString & filePath() const { return m_filePath; }


	inline bool isPlaying() const { return m_state == VideoPlayer::Playing; }
	inline bool isPaused() const { return m_state == VideoPlayer::Paused; }
	inline double position() const { return m_state <= VideoPlayer::Opening ? -1.0 : (m_state == VideoPlayer::Ready ? 0.0 : m_position); }
	inline double length() const { return m_state <= VideoPlayer::Opening ? -1.0 : m_length; }
	inline double framesPerSecond() const { return m_state <= VideoPlayer::Opening ? -1.0 : m_fps; }
	inline double playbackRate() const { return m_state != VideoPlayer::Playing ? .0 : m_playbackRate; }
	inline bool isStopped() const { return m_state == VideoPlayer::Ready; }
	inline double volume() const { return m_volume; }
	inline bool isMuted() const { return m_muted; }
	inline int selectedAudioStream() const { return m_state <= VideoPlayer::Opening ? -1 : m_activeAudioStream; }

	void playbackRate(double newRate);

	inline const QStringList & textStreams() const { return m_textStreams; }
	inline const QStringList & audioStreams() const { return m_audioStreams; }

	bool playOnLoad();

public slots:
	bool openFile(const QString &filePath);
	bool closeFile();

	bool play();
	bool pause();
	bool togglePlayPaused();
	bool seek(double seconds);
	bool step(int frameOffset);
	bool stop();
	bool selectAudioStream(int audioStreamIndex);

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

	void playbackError(const QString &errorMessage = QString());
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

	bool backendInit(PlayerBackend *backend);
	void backendCleanup();
	void backendRestart();

	PlayerBackend * backendLoad(const QString &pluginPath);

	void resetState();

private slots:
	// PlayerPlugins update player state with these
	void changeResolution(int width, int height, double aspectRatio);
	void changeFPS(double fps);
	void updateTextStreams(const QStringList &textStreams);
	void updateAudioStreams(const QStringList &audioStreams, int selectedAudioStream);
	void onError(const QString &message = QString());

	void changeState(VideoPlayer::State state);
	void changePosition(double position);
	void changeLength(double length);
	void changePlaySpeed(double playbackRate);
	void changeVolume(double volume);
	void changeMute(bool muted);

private:
	QMap<QString, PlayerBackend *> m_plugins;
	PlayerBackend *m_backend;
	QWidget *m_videoContainer; // layered widget containing video and subtitle overlay

	State m_state;

	VideoWidget *m_videoWidget;

	QString m_filePath;

	double m_position;
	double m_length;
	double m_fps;
	double m_playbackRate;
	double m_minPositionDelta;
	QStringList m_textStreams;
	int m_activeAudioStream;
	QStringList m_audioStreams;

	bool m_muted;
	double m_volume;
	double m_backendVolume;
};
}
#endif
