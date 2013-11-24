#ifndef GSTREAMERDECODERBACKEND_H
#define GSTREAMERDECODERBACKEND_H

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
#include "../decoderbackend.h"
#include "../wavewriter.h"

#include <QtGui/QWidget>
#include <QtCore/QString>

#include <gst/gst.h>

class QTimer;

namespace SubtitleComposer {
class GStreamerDecoderBackend : public DecoderBackend
{
	Q_OBJECT

public:
	GStreamerDecoderBackend(Decoder *decoder);
	virtual ~GStreamerDecoderBackend();

	const GStreamerConfig * config() { return static_cast<const GStreamerConfig *const>(DecoderBackend::config()); }

	virtual AppConfigGroupWidget * newAppConfigGroupWidget(QWidget *parent);

protected:
	virtual QWidget * initialize(QWidget *videoWidgetParent);
	virtual void finalize();

	virtual bool openFile(const QString &filePath);
	virtual void closeFile();

	virtual bool decode(int audioStream, const QString &outputPath, const WaveFormat &outputFormat);
	virtual bool stop();

private:
	static void dataHandoff(GstElement *fakesrc, GstBuffer *buffer, GstPad *pad, gpointer userData);
	static void decodebinPadAdded(GstElement *decodebin, GstPad *pad, gpointer userData);
	static void decodebinNoMorePads(GstElement *decodebin, gpointer userData);

private slots:
	void onInfoTimerTimeout();
	void onDecodingTimerTimeout();

private:
	GstPipeline *m_infoPipeline;
	GstBus *m_infoBus;
	QTimer *m_infoTimer;

	GstPipeline *m_decodingPipeline;
	GstBus *m_decodingBus;
	QTimer *m_decodingTimer;

	QString m_decodingStreamName;
	WaveFormat m_decodingStreamFormat;
	bool m_lengthInformed;
	WaveWriter m_waveWriter;
};
}

#endif
