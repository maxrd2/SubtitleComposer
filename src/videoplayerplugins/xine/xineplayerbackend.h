#ifndef XINEPLAYERBACKEND_H
#define XINEPLAYERBACKEND_H

/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2018 Mladen Milinkovic <max@smoothware.net>
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "videoplayer/playerbackend.h"

#include <QString>
#include <QRect>
#include <QTimer>
#include <QWidget>

#include <xine.h>
#include <xcb/xcb.h>

QT_FORWARD_DECLARE_CLASS(QEvent)

namespace SubtitleComposer {
class XinePlayerBackend : public PlayerBackend
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID PlayerBackend_iid)
	Q_INTERFACES(SubtitleComposer::PlayerBackend)

public:
	XinePlayerBackend();
	virtual ~XinePlayerBackend();

	QWidget * newConfigWidget(QWidget *parent) override;

protected:
	bool initialize(VideoWidget *videoWidget) override;
	void finalize() override;
	void _finalize();
	bool reconfigure() override;

	bool openFile(const QString &filePath, bool &playingAfterCall) override;
	void closeFile() override;

	bool play() override;
	bool pause() override;
	bool seek(double seconds, bool accurate) override;
	bool step(int /*frameOffset*/) override { return false; }
	bool stop() override;

	void playbackRate(double /*newRate*/) override {}

	bool setActiveAudioStream(int audioStream) override;

	bool setVolume(double volume) override;

protected:
	bool initializeXine(WId winId);
	void finalizeXine();

	void updateVideoData();
	void updateAudioData();
	void updateLengthData();

	static void xineEventListener(void *p, const xine_event_t *);
	void customEvent(QEvent *event) override;

	static void destSizeCallback(void *p, int video_width, int video_height, double video_aspect, int *dest_width, int *dest_height, double *dest_aspect);
	static void frameOutputCallback(void *p, int video_width, int video_height, double video_aspect, int *dest_x, int *dest_y, int *dest_width, int *dest_height, double *dest_aspect, int *win_x, int *win_y);
	static void audioMixerMethodChangedCallback(void *p, xine_cfg_entry_t *entry);

protected slots:
	void updatePosition();
	void onVideoLayerGeometryChanged();

private:
	void setSCConfig(SCConfig *scConfig) override;

private:
	xcb_connection_t * m_connection;
	xcb_visual_t m_x11Visual;

	xine_t *m_xineEngine;
	xine_audio_port_t *m_audioDriver;
	xine_video_port_t *m_videoDriver;
	xine_stream_t *m_xineStream;
	xine_event_queue_t *m_eventQueue;

	bool m_updatePosition;
	bool m_softwareMixer;

	QRect m_videoLayerGeometry;
	QTimer m_timesTimer;

	bool m_streamIsSeekable;
};
}

#endif
