#ifndef GSTREAMERPLAYERBACKEND2_H
#define GSTREAMERPLAYERBACKEND2_H

/***************************************************************************
 *   Copyright (C) 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gstreamerconfig.h"
#include "../playerbackend.h"

#include <QtGui/QWidget>
#include <QtCore/QString>

#include <gst/gst.h>

class QTimer;

namespace SubtitleComposer {
class GStreamerPlayerBackend : public PlayerBackend
{
	Q_OBJECT

public:
	GStreamerPlayerBackend(Player *player);
	virtual ~GStreamerPlayerBackend();

	const GStreamerConfig * config() { return static_cast<const GStreamerConfig *const>(PlayerBackend::config()); }

	virtual AppConfigGroupWidget * newAppConfigGroupWidget(QWidget *parent);

protected:
	virtual VideoWidget * initialize(QWidget *videoWidgetParent);
	virtual void finalize();

	virtual bool openFile(const QString &filePath, bool &playingAfterCall);
	virtual void closeFile();

	virtual bool play();
	virtual bool pause();
	virtual bool seek(double seconds, bool accurate);
	virtual bool stop();

	virtual bool setActiveAudioStream(int audioStream);

	virtual bool setVolume(double volume);

protected slots:
	void onPlaybinTimerTimeout();

private:
	void setupVideoOverlay();

//	static void audioChanged(GstElement *playbin2, gpointer userData);
//	static void videoChanged(GstElement *playbin2, gpointer userData);

	void updateAudioData();
	void updateVideoData();

private:
	GstPipeline *m_pipeline;
	GstBus *m_pipelineBus;
	QTimer *m_pipelineTimer;
	bool m_lengthInformed;
};
}

#endif
