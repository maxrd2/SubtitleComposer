#ifndef MPVBACKEND_H
#define MPVBACKEND_H

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

#include "videoplayer/playerbackend.h"

#include <mpv/client.h>

#include <QWidget>
#include <QString>

namespace SubtitleComposer {
class MPVProcess;

class MPVBackend : public PlayerBackend
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID PlayerBackend_iid)
	Q_INTERFACES(SubtitleComposer::PlayerBackend)

public:
	MPVBackend();
	virtual ~MPVBackend();

	QWidget * newConfigWidget(QWidget *parent) override;

protected:
	bool initialize(VideoWidget *videoWidgetParent) override;
	void finalize() override;
	void _finalize();
	bool reconfigure() override;

	void waitState(VideoPlayer::State state);
	void waitState(VideoPlayer::State state1, VideoPlayer::State state2);

	bool openFile(const QString &filePath, bool &playingAfterCall) override;
	void closeFile() override;

	bool play() override;
	bool pause() override;
	bool seek(double seconds, bool accurate) override;
	bool step(int frameOffset) override;
	bool stop() override;

	void playbackRate(double newRate) override;

	bool setActiveAudioStream(int audioStream) override;

	bool setVolume(double volume) override;

	bool mpvInit();
	void mpvExit();

signals:
	void mpvEvents();

protected slots:
	void onMPVEvents();

private:
	void mpvEventHandle(mpv_event *event);
	static void wakeup(void *ctx);

	void updateTextData(const mpv_event_property *prop);
	void updateAudioData(const mpv_event_property *prop);
	void updateVideoData();

	void setSCConfig(SCConfig *scConfig) override;

protected:
	mpv_handle *m_mpv;
	bool m_initialized;
	QString m_currentFilePath;
};
}

#endif
