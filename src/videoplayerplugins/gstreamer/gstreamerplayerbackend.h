#ifndef GSTREAMERPLAYERBACKEND2_H
#define GSTREAMERPLAYERBACKEND2_H

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

#include "../../videoplayer/playerbackend.h"

#include <QWidget>
#include <QString>

#include <gst/gst.h>

QT_FORWARD_DECLARE_CLASS(QTimer)

namespace SubtitleComposer {
class GStreamerPlayerBackend : public PlayerBackend
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID PlayerBackend_iid)
	Q_INTERFACES(SubtitleComposer::PlayerBackend)

public:
	GStreamerPlayerBackend();
	virtual ~GStreamerPlayerBackend();

	virtual QWidget * newConfigWidget(QWidget *parent);

protected:
	virtual bool initialize(VideoWidget *videoWidget);
	virtual void finalize();
	virtual bool reconfigure();

	virtual bool doesVolumeCorrection() const { return true; }

	static GstElement * createAudioSink();
	static GstElement * createVideoSink();

	virtual bool openFile(const QString &filePath, bool &playingAfterCall);
	virtual void closeFile();

	virtual bool play();
	virtual bool pause();
	virtual bool seek(double seconds, bool accurate);
	virtual bool stop();

	virtual void playbackRate(double newRate);

	virtual bool setActiveAudioStream(int audioStream);

	virtual bool setVolume(double volume);

	bool eventFilter(QObject *obj, QEvent *event);

protected slots:
	void onPlaybinTimerTimeout();

private:
	void setupVideoOverlay();

	void updateTextData();
	void updateAudioData();
	void updateVideoData();

	virtual void setSCConfig(SCConfig *scConfig);

private:
	GstPipeline *m_pipeline;
	GstBus *m_pipelineBus;
	QTimer *m_pipelineTimer;
	bool m_lengthInformed;
	gdouble m_playbackRate;
	gdouble m_volume;
	gboolean m_muted;
};
}

#endif
