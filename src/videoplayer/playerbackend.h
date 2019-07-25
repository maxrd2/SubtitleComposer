#ifndef PLAYERBACKEND_H
#define PLAYERBACKEND_H

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

#include "videoplayer.h"

#include <QObject>
#include <QString>
#include <QWidget>

class SCConfig;

#define PlayerBackend_iid "org.kde.SubtitleComposer.PlayerBackend"

class KCoreConfigSkeleton;

namespace SubtitleComposer {
class PlayerBackend : public QObject
{
	Q_OBJECT

	friend class VideoPlayer;
	friend class ConfigDialog;

public:
	inline const QString & name() const { return m_name; }

protected:
	explicit PlayerBackend();

	virtual QWidget * newConfigWidget(QWidget *parent) = 0;
	virtual KCoreConfigSkeleton * config() const = 0;

	/**
	 * @brief init - Perform any required initialization
	 * @param videoWidget - widget in which video must be rendered
	 * @return true on success
	 */
	virtual bool init(QWidget *videoWidget) = 0;

	/**
	 * @brief cleanup - Cleanup anything that's in use or have been initalized
	 */
	virtual void cleanup() = 0;

	/*
	 * IMPORTANT: methods that are described as synchronous must emit state
	 * change before returning - e.g. openFile() should emit stateChanged signal
	 * before returning, other signals like resolutionChanged, positionChanged
	 * can probably be emitted at later time
	 */

	/**
	 * @brief openFile - open video file, this function must be synchronous
	 * @param path - full path to video file
	 * @return false if error occurred
	 */
	virtual bool openFile(const QString &path) = 0;

	/**
	 * @brief closeFile - close current video file - this function must be synchronous
	 * @return false if error occurred
	 */
	virtual bool closeFile() = 0;

	/**
	 * @brief play - start playing - this function must be synchronous
	 * @return false if error occurred
	 */
	virtual bool play() = 0;

	/**
	 * @brief pause - pause playing - this function must be synchronous
	 * @return false if error occurred
	 */
	virtual bool pause() = 0;

	/**
	 * @brief seek - jump to some play time - this function should be async
	 * @param seconds - play position from video start in seconds
	 * @return false if error occurred
	 */
	virtual bool seek(double seconds) = 0;

	/**
	 * @brief step - skip amount of frames backwars/forwards - this function should be async
	 * @param frameOffset - count of frames to step
	 * @return false if error occurred
	 */
	virtual bool step(int frameOffset) = 0;

	/**
	 * @brief stop - stop playing - this function must be synchronous
	 * @return false if error occurred
	 */
	virtual bool stop() = 0;

	/**
	 * @brief playbackRate - change playback rate - this function can be async
	 * @param newRate - new play speed multiplier
	 * @return false if error occurred
	 */
	virtual bool playbackRate(double newRate) = 0;

	/**
	 * @brief selectAudioStream - change active audio stream - this function can be async
	 * @param streamIndex - audio stream index as notified by textStreamsChanged()
	 * @return false if error occurred
	 */
	virtual bool selectAudioStream(int streamIndex) = 0;

	/**
	 * @brief setVolume - change audio volume - this function can be async
	 * @param volume - value from 0. - 100.
	 * @return false if error occurred
	 */
	virtual bool setVolume(double volume) = 0;

signals:
	// notify application of state changes
	void resolutionChanged(int width, int height, double aspectRatio);
	void fpsChanged(double framesPerSecond);

	void stateChanged(VideoPlayer::State state);
	void errorOccured(const QString &errorMessage);

	void positionChanged(double position); // play position in seconds
	void lengthChanged(double length); // media length in seconds

	void speedChanged(double speedMultiplier);

	void volumeChanged(double volume); // audio volume 0. - 100.
	void muteChanged(bool muted);

	void textStreamsChanged(const QStringList &textStreams);
	void audioStreamsChanged(const QStringList &audioStreams, int activeAudioStream);

protected:
	QString m_name;
};
}

Q_DECLARE_INTERFACE(SubtitleComposer::PlayerBackend, PlayerBackend_iid)

#endif
