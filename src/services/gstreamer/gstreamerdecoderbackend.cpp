/***************************************************************************
 *   Copyright (C) 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)  *
 *   based on Kaffeine by Jürgen Kofler                                    *
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

#include "gstreamerdecoderbackend.h"
#include "gstreamerconfigwidget.h"
#include "gstreamer.h"

#include <QtCore/QTimer>

#include <KDebug>

using namespace SubtitleComposer;

#define MESSAGE_INFO_INIT_DISPOSE 1

GStreamerDecoderBackend::GStreamerDecoderBackend(Decoder *decoder) :
	DecoderBackend(decoder, "GStreamer", new GStreamerConfig()),
	m_infoPipeline(0),
	m_infoBus(0),
	m_infoTimer(new QTimer(this)),
	m_decodingPipeline(0),
	m_decodingBus(0),
	m_decodingTimer(new QTimer(this))
{
	connect(m_infoTimer, SIGNAL(timeout()), this, SLOT(onInfoTimerTimeout()));
	connect(m_decodingTimer, SIGNAL(timeout()), this, SLOT(onDecodingTimerTimeout()));
}

GStreamerDecoderBackend::~GStreamerDecoderBackend()
{
	if(isInitialized())
		GStreamer::deinit();
}

QWidget *
GStreamerDecoderBackend::initialize(QWidget * /*videoWidgetParent*/)
{
	return GStreamer::init() ? (QWidget *)0 : (QWidget *)1;
}

void
GStreamerDecoderBackend::finalize()
{
	GStreamer::deinit();
}

SubtitleComposer::AppConfigGroupWidget *
GStreamerDecoderBackend::newAppConfigGroupWidget(QWidget *parent)
{
	return new GStreamerConfigWidget(parent);
}

bool
GStreamerDecoderBackend::openFile(const QString &filePath)
{
	m_lengthInformed = false;

	m_infoPipeline = GST_PIPELINE(gst_pipeline_new("information_pipeline"));
	GstElement *filesrc = gst_element_factory_make("filesrc", "filesrc");
	GstElement *decodebin = gst_element_factory_make("decodebin", "decodebin");
	GstElement *audioconvert = gst_element_factory_make("audioconvert", "audioconvert");
	GstElement *fakesink = gst_element_factory_make("fakesink", "fakesink");

	if(!m_infoPipeline || !filesrc || !decodebin || !audioconvert || !fakesink) {
		if(filesrc)
			gst_object_unref(GST_OBJECT(filesrc));
		if(decodebin)
			gst_object_unref(GST_OBJECT(decodebin));
		if(audioconvert)
			gst_object_unref(GST_OBJECT(audioconvert));
		if(fakesink)
			gst_object_unref(GST_OBJECT(fakesink));
		if(m_infoPipeline)
			gst_object_unref(GST_OBJECT(m_infoPipeline));
		m_infoPipeline = 0;
		return false;
	}

	gchar *uri = g_strdup(filePath.toLocal8Bit());
	g_object_set(G_OBJECT(filesrc), "location", uri, NULL);
	g_free(uri);

	// listen for newly created pads
	g_signal_connect(decodebin, "pad-added", G_CALLBACK(decodebinPadAdded), this);
	g_signal_connect(decodebin, "no-more-pads", G_CALLBACK(decodebinNoMorePads), this);

	// First add the elements to the bin
	gst_bin_add_many(GST_BIN(m_infoPipeline), filesrc, decodebin, audioconvert, fakesink, NULL);

	// link the elements
	gboolean success = TRUE;
	success = success && GST_PAD_LINK_SUCCESSFUL(GStreamer::link(GST_BIN(m_infoPipeline), "filesrc", "decodebin"));
	success = success && GST_PAD_LINK_SUCCESSFUL(GStreamer::link(GST_BIN(m_infoPipeline), "audioconvert", "fakesink"));

	if(success) {
		m_infoBus = gst_pipeline_get_bus(GST_PIPELINE(m_infoPipeline));
		m_infoTimer->start(20);
		GStreamer::setElementState(GST_ELEMENT(m_infoPipeline), GST_STATE_PAUSED, 0 /*don't block waiting */);
	} else {
		gst_object_unref(GST_OBJECT(m_infoPipeline));
		m_infoPipeline = 0;
	}

	return success;
}

void
GStreamerDecoderBackend::closeFile()
{
	stop();
}

bool
GStreamerDecoderBackend::decode(int audioStream, const QString &outputPath, const WaveFormat &outputFormat)
{
	m_decodingPipeline = GST_PIPELINE(gst_pipeline_new("decoding_pipeline"));
	GstElement *filesrc = gst_element_factory_make("filesrc", "filesrc");
	GstElement *decodebin = gst_element_factory_make("decodebin", "decodebin");
	GstElement *audioresample = gst_element_factory_make("audioresample", "audioresample");
	GstElement *audioconvert = gst_element_factory_make("audioconvert", "audioconvert");
	GstElement *fakesink = gst_element_factory_make("fakesink", "fakesink");

	if(!m_decodingPipeline || !filesrc || !decodebin || !audioresample || !audioconvert || !fakesink) {
		if(filesrc)
			gst_object_unref(GST_OBJECT(filesrc));
		if(decodebin)
			gst_object_unref(GST_OBJECT(decodebin));
		if(audioresample)
			gst_object_unref(GST_OBJECT(audioresample));
		if(audioconvert)
			gst_object_unref(GST_OBJECT(audioconvert));
		if(fakesink)
			gst_object_unref(GST_OBJECT(fakesink));
		if(m_decodingPipeline)
			gst_object_unref(GST_OBJECT(m_decodingPipeline));
		m_decodingPipeline = 0;
		return false;
	}

	gchar *uri = g_strdup(decoder()->filePath().toLocal8Bit());
	g_object_set(G_OBJECT(filesrc), "location", uri, NULL);
	g_free(uri);

	g_object_set(G_OBJECT(fakesink), "signal-handoffs", TRUE, NULL);

	g_signal_connect(decodebin, "pad-added", G_CALLBACK(decodebinPadAdded), this);
	g_signal_connect(fakesink, "handoff", G_CALLBACK(dataHandoff), this);

	// add the elements to the bin
	gst_bin_add_many(GST_BIN(m_decodingPipeline), filesrc, decodebin, audioresample, audioconvert, fakesink, NULL);

	GstBin *bin = GST_BIN(m_decodingPipeline);

	gboolean success = TRUE;
	success = success && GST_PAD_LINK_SUCCESSFUL(GStreamer::link(bin, "filesrc", "decodebin"));
	// success = success && GST_PAD_LINK_SUCCESSFUL( GStreamer::link( bin, "decodebin", "audioresample" ) );

	if(success) {
		m_waveWriter.open(outputPath, outputFormat);
		m_decodingStreamName = decoder()->audioStreamName(audioStream);
		m_decodingStreamFormat = decoder()->audioStreamFormat(audioStream);
		m_decodingBus = gst_pipeline_get_bus(GST_PIPELINE(m_decodingPipeline));
		m_decodingTimer->start(20);
		GStreamer::setElementState(GST_ELEMENT(m_decodingPipeline), GST_STATE_PLAYING, 0 /*don't block waiting */);
	} else {
		gst_object_unref(GST_OBJECT(m_decodingPipeline));
		m_decodingPipeline = 0;
	}

	return success;
}

bool
GStreamerDecoderBackend::stop()
{
	m_waveWriter.close();

	if(m_infoPipeline) {
		m_infoTimer->stop();
		GStreamer::setElementState(GST_ELEMENT(m_infoPipeline), GST_STATE_NULL, 60000 /*"infinity" wait */);
		GStreamer::freePipeline(&m_infoPipeline, &m_infoBus);
	}

	if(m_decodingPipeline) {
		m_decodingTimer->stop();
		GStreamer::setElementState(GST_ELEMENT(m_decodingPipeline), GST_STATE_NULL, 60000 /*"infinity" wait */);
		GStreamer::freePipeline(&m_decodingPipeline, &m_decodingBus);
	}

	return true;
}

void
GStreamerDecoderBackend::dataHandoff(GstElement * /*fakesink */, GstBuffer *buffer, GstPad * /*pad */, gpointer userData)
{
	GStreamerDecoderBackend *backend = (GStreamerDecoderBackend *)userData;
	GstMapInfo map;

	gst_buffer_map(buffer, &map, GST_MAP_READ);
	backend->m_waveWriter.writeSamplesData(map.data, map.size, backend->m_waveWriter.outputFormat());
	gst_buffer_unmap(buffer, &map);
}

void
GStreamerDecoderBackend::decodebinPadAdded(GstElement *decodebin, GstPad *srcpad, gpointer userData)
{
	if(gst_pad_get_direction(srcpad) != GST_PAD_SRC)
		return;

	GStreamerDecoderBackend *backend = (GStreamerDecoderBackend *)userData;
	SubtitleComposer::Decoder *decoder = backend->decoder();

	gchar *name = gst_pad_get_name(srcpad);
	QString srcPadName = name;
	g_free(name);

	if(backend->m_infoPipeline && gst_bin_get_by_name(GST_BIN(backend->m_infoPipeline), "decodebin") == decodebin) {
		// we are gathering information about the stream

		GstElement *audioconvert = gst_bin_get_by_name(GST_BIN(backend->m_infoPipeline), "audioconvert");
		GstPad *sinkpad = gst_element_get_static_pad(GST_ELEMENT(audioconvert), "sink");
		if(gst_pad_is_linked(sinkpad)) {
			GstPad *peer = gst_pad_get_peer(sinkpad);
			gst_pad_unlink(peer, sinkpad);
			gst_object_unref(peer);
		}
		gboolean compatible = gst_pad_can_link(srcpad, sinkpad);
		gst_object_unref(sinkpad);
		gst_object_unref(audioconvert);

		if(decoder->audioStreamNames().contains(srcPadName))
			return;

		// GStreamer::inspectPad( srcpad, compatible ? "ADDED COMPATIBLE " : "ADDED NON COMPATIBLE " );

		if(!compatible)
			return;

		GstCaps *srccaps = gst_pad_get_pad_template_caps(srcpad);
		if(srccaps) {
			WaveFormat filterFormat = GStreamer::formatFromAudioCaps(srccaps);

			backend->appendDecoderAudioStream(srcPadName, filterFormat);

			if(!gst_pad_is_linked(sinkpad))
				gst_pad_link(srcpad, sinkpad);

			gst_caps_unref(srccaps);
		}
	} else {
		// we are starting playback

		GstBin *bin = GST_BIN(backend->m_decodingPipeline);

		if(backend->m_decodingStreamName == srcPadName) {
			// GstCaps* sourceFilter = GStreamer::audioCapsFromFormat( backend->m_decodingStreamFormat, false );
			GstCaps *outputFilter = GStreamer::audioCapsFromFormat(backend->m_waveWriter.outputFormat());

			gboolean success = TRUE;
			success = success && GST_PAD_LINK_SUCCESSFUL(GStreamer::link(bin, "decodebin", srcPadName.toAscii(), "audioresample", "sink" /*, sourceFilter */));
			success = success && GST_PAD_LINK_SUCCESSFUL(GStreamer::link(bin, "audioresample", "audioconvert"));
			success = success && GST_PAD_LINK_SUCCESSFUL(GStreamer::link(bin, "audioconvert", "fakesink", outputFilter));

			if(success) {
				qDebug() << "all linked ok, decoding should start now";
			} else {
				GError *error = g_error_new(g_quark_from_string("application"), 1, "failed to link pipeline elements");
				GstMessage *msg = gst_message_new_error(GST_OBJECT(backend->m_decodingPipeline), error, "failed to link pipeline elements");
				gst_element_post_message(GST_ELEMENT(backend->m_decodingPipeline), msg);
			}
		}
	}
}

void
GStreamerDecoderBackend::decodebinNoMorePads(GstElement * /*decodebin */, gpointer userData)
{
	GStreamerDecoderBackend *backend = (GStreamerDecoderBackend *)userData;

	GstStructure *structure = gst_structure_new("app_message", "type", G_TYPE_INT, MESSAGE_INFO_INIT_DISPOSE, 0);
	GstMessage *msg = gst_message_new_application(GST_OBJECT(backend->m_infoPipeline), structure);

	gst_element_post_message(GST_ELEMENT(backend->m_infoPipeline), msg);
}

void
GStreamerDecoderBackend::onInfoTimerTimeout()
{
	if(!isInitialized())
		return;

	GstMessage *msg;
	while(m_infoBus && m_infoPipeline && (msg = gst_bus_pop(m_infoBus))) {
		GstObject *src = GST_MESSAGE_SRC(msg);

		// we are only interested in error messages or messages directed to the info pipeline
		if(GST_MESSAGE_TYPE(msg) != GST_MESSAGE_ERROR && src != GST_OBJECT(m_infoPipeline)) {
			gst_message_unref(msg);
			continue;
		}

		GStreamer::inspectMessage(msg);

		switch(GST_MESSAGE_TYPE(msg)) {
		case GST_MESSAGE_APPLICATION: {
			gint type = g_value_get_int(gst_structure_get_value(gst_message_get_structure(msg), "type"));
			if(type == MESSAGE_INFO_INIT_DISPOSE) {
				GstStateChangeReturn ret = GStreamer::setElementState(GST_ELEMENT(m_infoPipeline), GST_STATE_NULL);
				if(ret != GST_STATE_CHANGE_FAILURE && ret != GST_STATE_CHANGE_ASYNC)
					GStreamer::freePipeline(&m_infoPipeline, &m_infoBus);
				setDecoderState(Decoder::Ready);
			}
			break;
		}

		case GST_MESSAGE_STATE_CHANGED: {
			GstState old, cur, pending;
			gst_message_parse_state_changed(msg, &old, &cur, &pending);
			if(cur == GST_STATE_NULL)
				GStreamer::freePipeline(&m_infoPipeline, &m_infoBus);
			break;
		}

		case GST_MESSAGE_DURATION: {
			GstFormat format;
			gint64 duration;
			gst_message_parse_duration(msg, &format, &duration);
			setDecoderLength((double)duration / GST_SECOND);
			break;
		}

		case GST_MESSAGE_ERROR: {
			GStreamer::setElementState(GST_ELEMENT(m_infoPipeline), GST_STATE_NULL, 60000);
			GStreamer::freePipeline(&m_infoPipeline, &m_infoBus);
			setDecoderErrorState();
			break;
		}

		default:
			break;
		}

		gst_message_unref(msg);
	}
}

void
GStreamerDecoderBackend::onDecodingTimerTimeout()
{
	if(!isInitialized() || !m_decodingBus || !m_decodingPipeline)
		return;

	// first we update the decoding position and file length (if it hasn't been informed yet)
	gint64 time;
	if(!m_lengthInformed && gst_element_query_duration(GST_ELEMENT(m_decodingPipeline), GST_FORMAT_TIME, &time) && GST_CLOCK_TIME_IS_VALID(time)) {
		setDecoderLength((double)time / GST_SECOND);
		m_lengthInformed = true;
	}
	if(gst_element_query_position(GST_ELEMENT(m_decodingPipeline), GST_FORMAT_TIME, &time))
		setDecoderPosition(((double)time / GST_SECOND));

	GstMessage *msg;
	while(m_decodingBus && m_decodingPipeline && (msg = gst_bus_pop(m_decodingBus))) {
		GstObject *src = GST_MESSAGE_SRC(msg);

		// we are only interested in error messages or messages directed to the decoding pipeline
		if(GST_MESSAGE_TYPE(msg) != GST_MESSAGE_ERROR && src != GST_OBJECT(m_decodingPipeline)) {
			gst_message_unref(msg);
			continue;
		}
		// GStreamer::inspectMessage( msg );

		switch(GST_MESSAGE_TYPE(msg)) {
		case GST_MESSAGE_STATE_CHANGED: {
			GstState old, current, target;
			gst_message_parse_state_changed(msg, &old, &current, &target);
			if(target == GST_STATE_PLAYING) {
				setDecoderState(Decoder::Decoding);
				GStreamer::setElementState(GST_ELEMENT(m_decodingPipeline), GST_STATE_PLAYING, 0);
			} else if(current == GST_STATE_READY || current == GST_STATE_NULL) {
				m_waveWriter.close();
				setDecoderState(Decoder::Ready);
			}
			break;
		}

		case GST_MESSAGE_DURATION: {
			GstFormat format;
			gint64 duration;
			gst_message_parse_duration(msg, &format, &duration);
			setDecoderLength((double)duration / GST_SECOND);
			break;
		}

		case GST_MESSAGE_EOS: {
			m_waveWriter.close();
			setDecoderState(Decoder::Ready);
			break;
		}

		case GST_MESSAGE_ERROR: {
			gchar *debug = NULL;
			GError *error = NULL;
			gst_message_parse_error(msg, &error, &debug);
			// setDecoderErrorState( QString( error->message ) );
			setDecoderErrorState(QString(debug));
			g_error_free(error);
			g_free(debug);
			break;
		}

		default:
			break;
		}

		gst_message_unref(msg);
	}
}

#include "gstreamerdecoderbackend.moc"
