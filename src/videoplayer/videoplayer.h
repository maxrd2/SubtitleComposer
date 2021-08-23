/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PLAYER_H
#define PLAYER_H

#include "videowidget.h"

#include "videoplayer/backend/ffplayer.h"
#include "videoplayer/subtitletextoverlay.h"

#include <QPainterPath>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QWidget>

namespace SubtitleComposer {

class VideoPlayer : public QObject
{
	Q_OBJECT

public:
	enum State {
		Initialized,
		Opening,
		Stopped,
		Playing,
		Paused
	};

	static VideoPlayer * instance();

	bool init(QWidget *videoContainer);

	inline State state() const { return m_state; }
	inline VideoWidget * videoWidget() { return m_videoWidget; }
	inline const QString & filePath() const { return m_filePath; }

	inline bool isPlaying() const { return m_state == Playing; }
	inline bool isPaused() const { return m_state == Paused; }
	inline bool isStopped() const { return m_state <= Stopped; }
	inline double position() const { return m_player->position(); }
	inline double duration() const { return m_duration; }
	inline double fps() const { return m_fps; }
	inline double playSpeed() const { return m_playSpeed; }
	inline double volume() const { return m_volume; }
	inline bool isMuted() const { return m_muted; }
	inline int selectedAudioStream() const { return m_activeAudioStream; }

	void playSpeed(double newRate);

	inline const QStringList & textStreams() const { return m_textStreams; }
	inline const QStringList & audioStreams() const { return m_audioStreams; }

	inline SubtitleTextOverlay & subtitleOverlay() { return m_subOverlay; }

	bool playOnLoad();

public slots:
	bool openFile(const QString &filePath);
	void closeFile();

	void play();
	void pause();
	void togglePlayPaused();
	bool seek(double seconds);
	bool step(int frameOffset);
	bool stop();
	bool selectAudioStream(int streamIndex);

	void increaseVolume(double amount = 3.0);
	void decreaseVolume(double amount = 3.0);
	void setVolume(double volume); // [0.0 - 100.0]
	void setMuted(bool mute);

signals:
	void fileOpenError(const QString &filePath, const QString &reason);
	void fileOpened(const QString &filePath);
	void fileClosed();

	void playbackError(const QString &errorMessage = QString());
	void playing();
	void positionChanged(double seconds);
	void durationChanged(double seconds);
	void fpsChanged(double fps);
	void playSpeedChanged(double rate);
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

	void reset();
	void setupNotifications();

private:
	FFPlayer *m_player;
	VideoWidget *m_videoWidget;
	SubtitleTextOverlay m_subOverlay;
	QString m_filePath;

	State m_state;
	double m_duration;
	double m_fps;
	double m_playSpeed;
	QStringList m_textStreams;
	int m_activeAudioStream;
	QStringList m_audioStreams;

	bool m_muted;
	double m_volume;
};
}
#endif
