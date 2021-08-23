/*
    SPDX-FileCopyrightText: 2003 Fabrice Bellard
    SPDX-FileCopyrightText: 2020 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FFPLAYER_H
#define FFPLAYER_H

#include "videoplayer/backend/decoder.h"
#include "videoplayer/backend/audiodecoder.h"
#include "videoplayer/backend/videodecoder.h"
#include "videoplayer/backend/subtitledecoder.h"
#include "videoplayer/backend/framequeue.h"
#include "videoplayer/backend/packetqueue.h"
#include "videoplayer/backend/clock.h"
#include "videoplayer/backend/streamdemuxer.h"
#include "videoplayer/backend/videostate.h"

#include <QtGlobal>
#include <QObject>
#include <QTimer>

extern "C" {
#include "libavformat/avformat.h"
}


QT_FORWARD_DECLARE_CLASS(QThread)
QT_FORWARD_DECLARE_CLASS(QWaitCondition)

namespace SubtitleComposer {
class VideoPlayer;
class GLRenderer;

class FFPlayer : public QObject {
	Q_OBJECT

public:
	FFPlayer(QObject *parent=nullptr);
	virtual ~FFPlayer();

	static uint8_t *flushPkt();

	bool open(const char *filename);
	void close();

	inline double position() { return m_vs == nullptr ? 0.0 : m_vs->position(); }

	void pauseToggle();
	bool paused();
	void stepFrame(int frameCnt);

	void seek(double seconds);

	bool muted() { return m_muted; }
	void setMuted(bool mute);
	double volume() { return m_volume; }
	void setVolume(double volume);

	void setSpeed(double speed);

	quint32 videoWidth();
	quint32 videoHeight();
	qreal videoSAR();
	qreal videoFPS();

	int activeVideoStream();
	int activeAudioStream();
	void activeAudioStream(int streamIndex);
	int activeSubtitleStream();

	enum State { Stopped, Playing, Paused };
	Q_ENUM(FFPlayer::State)

	inline GLRenderer * renderer() const { return m_renderer; }

signals:
	void mediaLoaded();
	void stateChanged(FFPlayer::State state);

	void positionChanged(double pos);
	void durationChanged(double duration);
	void speedChanged(double speed);

	void volumeChanged(double volume);
	void muteChanged(bool muted);

	void videoStreamsChanged(const QStringList &streams);
	void audioStreamsChanged(const QStringList &streams);
	void subtitleStreamsChanged(const QStringList &streams);

private:
	bool m_muted;
	double m_volume;

	QTimer m_positionTimer;
	qint32 m_postitionLast;
	VideoState *m_vs;
	GLRenderer *m_renderer;
};
}

Q_DECLARE_METATYPE(SubtitleComposer::FFPlayer::State)

#endif // FFPLAYER_H
