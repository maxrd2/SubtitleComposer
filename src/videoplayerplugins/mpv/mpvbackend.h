#ifndef MPVBACKEND_H
#define MPVBACKEND_H

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

#include "videoplayer/playerbackend.h"

#include <mpv/qthelper.hpp>

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

	QWidget * newConfigWidget(QWidget *parent) Q_DECL_OVERRIDE;

protected:
	bool initialize(VideoWidget *videoWidgetParent) Q_DECL_OVERRIDE;
	void finalize() Q_DECL_OVERRIDE;
	void _finalize();
	bool reconfigure() Q_DECL_OVERRIDE;

	void waitState(VideoPlayer::State state);

	bool openFile(const QString &filePath, bool &playingAfterCall) Q_DECL_OVERRIDE;
	void closeFile() Q_DECL_OVERRIDE;

	bool play() Q_DECL_OVERRIDE;
	bool pause() Q_DECL_OVERRIDE;
	bool seek(double seconds, bool accurate) Q_DECL_OVERRIDE;
	bool step(int frameOffset) Q_DECL_OVERRIDE;
	bool stop() Q_DECL_OVERRIDE;

	void playbackRate(double newRate) Q_DECL_OVERRIDE;

	bool setActiveAudioStream(int audioStream) Q_DECL_OVERRIDE;

	bool setVolume(double volume) Q_DECL_OVERRIDE;

	bool mpvInit();
	void mpvExit();

signals:
	void mpvEvents();

protected slots:
	void onMPVEvents();

protected:
	void setupProcessArgs(const QString &filePath);

	void mpvEventHandle(mpv_event *event);

	static void wakeup(void *ctx);

private:
	void updateTextData(const mpv_event_property *prop);
	void updateAudioData(const mpv_event_property *prop);
	void updateVideoData();

	void setSCConfig(SCConfig *scConfig) Q_DECL_OVERRIDE;

protected:
	mpv_handle *m_mpv;
	bool m_initialized;
	QString m_currentFilePath;
};
}

#endif
