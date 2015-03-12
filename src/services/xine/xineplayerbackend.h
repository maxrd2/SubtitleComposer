#ifndef XINEPLAYERBACKEND_H
#define XINEPLAYERBACKEND_H

/**
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2015 Mladen Milinkovic <max@smoothware.net>
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
#include <config.h>
#endif

#include "../playerbackend.h"

#include <QtCore/QString>
#include <QtCore/QRect>
#include <QtCore/QTimer>
#include <QWidget>

#include <xine.h>
#ifdef HAVE_XCB
#include <xcb/xcb.h>
#else
typedef struct _XDisplay Display;
#endif

class QEvent;

namespace SubtitleComposer {
class XinePlayerBackend : public PlayerBackend
{
	Q_OBJECT

public:
	XinePlayerBackend(Player *player);
	virtual ~XinePlayerBackend();

	virtual QWidget * newConfigWidget(QWidget *parent);

protected:
	virtual VideoWidget * initialize(QWidget *videoWidgetParent);
	virtual void finalize();
	void _finalize();
	virtual bool reconfigure();

	virtual bool openFile(const QString &filePath, bool &playingAfterCall);
	virtual void closeFile();

	virtual bool play();
	virtual bool pause();
	virtual bool seek(double seconds, bool accurate);
	virtual bool stop();

	virtual bool setActiveAudioStream(int audioStream);

	virtual bool setVolume(double volume);

protected:
	bool initializeXine(WId winId);
	void finalizeXine();

	void updateVideoData();
	void updateAudioData();
	void updateLengthData();

	static void xineEventListener(void *p, const xine_event_t *);
	virtual void customEvent(QEvent *event);

	static void destSizeCallback(void *p, int video_width, int video_height, double video_aspect, int *dest_width, int *dest_height, double *dest_aspect);
	static void frameOutputCallback(void *p, int video_width, int video_height, double video_aspect, int *dest_x, int *dest_y, int *dest_width, int *dest_height, double *dest_aspect, int *win_x, int *win_y);
	static void audioMixerMethodChangedCallback(void *p, xine_cfg_entry_t *entry);

protected slots:
	void updatePosition();
	void onVideoLayerGeometryChanged();

private:
#ifdef HAVE_XCB
	xcb_connection_t * m_connection;
	xcb_visual_t m_x11Visual;
#else
	Display * m_connection;
	x11_visual_t m_x11Visual;
#endif

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
