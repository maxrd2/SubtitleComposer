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
#include <QtAV>

QT_FORWARD_DECLARE_CLASS(QTimer)

namespace SubtitleComposer {
class VideoPlayerSubtitleOverlay;
class SubtitleTextOverlay;

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
	void cleanup();

	inline State state() const { return m_state; }
	inline VideoWidget * videoWidget() { return m_videoWidget; }
	inline const QString & filePath() const { return m_filePath; }

	inline bool isPlaying() const { return m_state == Playing; }
	inline bool isPaused() const { return m_state == Paused; }
	inline bool isStopped() const { return m_state == Stopped; }
	inline double position() const { return m_position; }
	inline double duration() const { return m_duration; }
	inline double framesPerSecond() const { return m_fps; }
	inline double playbackRate() const { return m_playbackRate; }
	inline double volume() const { return m_volume; }
	inline bool isMuted() const { return m_muted; }
	inline int selectedAudioStream() const { return m_activeAudioStream; }

	void playbackRate(double newRate);

	inline const QStringList & textStreams() const { return m_textStreams; }
	inline const QStringList & audioStreams() const { return m_audioStreams; }

	SubtitleTextOverlay & subtitleOverlay();

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

	void reset();

private slots:
	// QtAV player state notifications
	void onMediaLoaded();
	void onResolutionChange();
	void onStateChange(QtAV::AVPlayer::State state);
	void onPositionChange(qint64 position);
	void onDurationChange(qint64 duration);
	void onSpeedChange(double speed);
	void onVolumeChange(double volume);
	void onMuteChange(bool muted);

private:
	QtAV::VideoOutput *m_renderer;
	QtAV::AVPlayer *m_player;

	State m_state;

	VideoWidget *m_videoWidget;

	QString m_filePath;

	double m_position;
	double m_duration;
	double m_fps;
	double m_playbackRate;
	double m_minPositionDelta;
	QStringList m_textStreams;
	int m_activeAudioStream;
	QStringList m_audioStreams;

	bool m_muted;
	double m_volume;

	VideoPlayerSubtitleOverlay *m_overlay;
};
}
#endif
