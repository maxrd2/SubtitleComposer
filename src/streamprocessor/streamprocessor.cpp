/**
 * Copyright (C) 2010-2016 Mladen Milinkovic <max@smoothware.net>
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

#include "streamprocessor.h"
#include "gstreamer.h"

#include <QTimer>
#include <QDebug>
#include <QThread>

#define MESSAGE_INFO_INIT_DISPOSE 1

using namespace SubtitleComposer;

StreamProcessor::StreamProcessor()
	: m_opened(false),
	  m_audioReady(false),
	  m_textReady(false),
	  m_decodingPipeline(NULL),
	  m_decodingBus(NULL),
	  m_decodingTimer(new QTimer(this))
{
	connect(m_decodingTimer, &QTimer::timeout, this, &StreamProcessor::decoderMessageProc);

	GStreamer::init();
}

StreamProcessor::~StreamProcessor()
{
	close();

	GStreamer::deinit();
}

bool
StreamProcessor::open(const QString &filename)
{
	if(m_opened)
		close();

	m_filename = filename;
	m_audioStreamIndex = -1;
	m_textStreamIndex = -1;
	m_streamLen = m_streamPos = 0;

	m_decodingPipeline = GST_PIPELINE(gst_pipeline_new("streamprocessor_pipeline"));
	GstElement *filesrc = gst_element_factory_make("filesrc", "filesrc");
	GstElement *decodebin = gst_element_factory_make("decodebin", "decodebin");

	if(!m_decodingPipeline || !filesrc || !decodebin) {
		if(filesrc)
			gst_object_unref(GST_OBJECT(filesrc));
		if(decodebin)
			gst_object_unref(GST_OBJECT(decodebin));
		if(m_decodingPipeline)
			gst_object_unref(GST_OBJECT(m_decodingPipeline));
		m_decodingPipeline = NULL;
		return false;
	}

	gchar *uri = g_strdup(m_filename.toLocal8Bit());
	g_object_set(G_OBJECT(filesrc), "location", uri, NULL);
	g_free(uri);

	g_signal_connect(decodebin, "pad-added", G_CALLBACK(onPadAdded), this);
	g_signal_connect(decodebin, "autoplug-continue", G_CALLBACK(onPadCheck), this);

	gst_bin_add_many(GST_BIN(m_decodingPipeline), filesrc, decodebin, NULL);

	if(gst_element_link(filesrc, decodebin)) {
		m_opened = true;
		return true;
	}

	close();
	return false;
}

void
StreamProcessor::close()
{
	if(m_decodingPipeline) {
		m_decodingTimer->stop();
		GStreamer::setElementState(GST_ELEMENT(m_decodingPipeline), GST_STATE_NULL, 60000);
		GStreamer::freePipeline(&m_decodingPipeline, &m_decodingBus);
	}

	m_opened = false;
	m_audioReady = false;
	m_textReady = false;
}

bool
StreamProcessor::initAudio(const int streamIndex, const WaveFormat &waveFormat)
{
	if(!m_opened)
		return false;

	m_audioStreamCurrent = -1;
	m_audioStreamIndex = streamIndex;
	m_audioStreamFormat = waveFormat;
	m_audioReady = false;

	GstElement *audioconvert = gst_element_factory_make("audioconvert", "audioconvert");
	GstElement *audioresample = gst_element_factory_make("audioresample", "audioresample");
	GstElement *fakesink = gst_element_factory_make("fakesink", "fakesink");

	if(!audioresample || !audioconvert || !fakesink) {
		if(audioresample)
			gst_object_unref(GST_OBJECT(audioresample));
		if(audioconvert)
			gst_object_unref(GST_OBJECT(audioconvert));
		if(fakesink)
			gst_object_unref(GST_OBJECT(fakesink));
		return false;
	}

	g_object_set(G_OBJECT(fakesink), "signal-handoffs", TRUE, NULL);
	g_signal_connect(fakesink, "handoff", G_CALLBACK(onAudioDataReady), this);

	gst_bin_add_many(GST_BIN(m_decodingPipeline), audioresample, audioconvert, fakesink, NULL);

	GstCaps *outputFilter = GStreamer::audioCapsFromFormat(m_audioStreamFormat);
	if(gst_element_link(audioresample, audioconvert)
			&& GST_PAD_LINK_SUCCESSFUL(GStreamer::link(GST_BIN(m_decodingPipeline), "audioconvert", "fakesink", outputFilter))) {
		m_decodingBus = gst_pipeline_get_bus(GST_PIPELINE(m_decodingPipeline));
		m_audioReady = true;
	}

	return m_audioReady;
}

bool
StreamProcessor::initText(const int /*streamIndex*/)
{
	if(!m_opened)
		return false;

	m_textStreamCurrent = -1;

	// FIXME: implement text stream demux
	return false;
}

bool
StreamProcessor::start()
{
	if(!m_opened || !(m_audioReady || m_textReady))
		return false;

	// do not start twice
	GstState valCurrent, valPending;
	if(gst_element_get_state(GST_ELEMENT(m_decodingPipeline), &valCurrent, &valPending, 0) != GST_STATE_CHANGE_SUCCESS
			|| valCurrent == GST_STATE_PLAYING || valPending == GST_STATE_PLAYING)
		return false;

	m_decodingTimer->start(20);
	GStreamer::setElementState(GST_ELEMENT(m_decodingPipeline), GST_STATE_PLAYING, 0);

	return true;
}

/*static*/ void
StreamProcessor::onAudioDataReady(GstElement */*fakesink*/, GstBuffer *buffer, GstPad */*pad*/, gpointer userData)
{
	StreamProcessor *me = reinterpret_cast<StreamProcessor *>(userData);
	GstMapInfo map;

	while(!me->m_streamLen) {
		gint64 time;
		if(gst_element_query_duration(GST_ELEMENT(me->m_decodingPipeline), GST_FORMAT_TIME, &time) && GST_CLOCK_TIME_IS_VALID(time) && time) {
			me->m_streamLen = time / GST_MSECOND;
			emit me->streamProgress(me->m_streamPos, me->m_streamLen);
		}
		QThread::yieldCurrentThread();
	}

	gst_buffer_map(buffer, &map, GST_MAP_READ);
	emit me->audioDataAvailable(map.data, map.size, &me->m_audioStreamFormat);
	gst_buffer_unmap(buffer, &map);
}

/*static*/ void
StreamProcessor::onPadAdded(GstElement */*decodebin*/, GstPad *pad, gpointer userData)
{
	StreamProcessor *me = reinterpret_cast<StreamProcessor *>(userData);

	if(!me->m_decodingPipeline || gst_pad_get_direction(pad) != GST_PAD_SRC)
		return;

	GstCaps *caps = gst_pad_get_current_caps(pad);
	const GstStructure *capsStruct = gst_caps_get_structure(caps, 0);
	const gchar *mimeType = gst_structure_get_name(capsStruct);

	if(strncmp(mimeType, "audio/", 6) == 0) {
		if(++(me->m_audioStreamCurrent) == me->m_audioStreamIndex) {
			// fill in missing values in WaveFormat from source values
			if(me->m_audioStreamFormat.channels() == 0)
				me->m_audioStreamFormat.setChannels(g_value_get_int(gst_structure_get_value(capsStruct, "channels")));
			if(me->m_audioStreamFormat.bitsPerSample() == 0)
				me->m_audioStreamFormat.setBitsPerSample(g_value_get_int(gst_structure_get_value(capsStruct, "width")));
			if(me->m_audioStreamFormat.sampleRate() == 0)
				me->m_audioStreamFormat.setSampleRate(g_value_get_int(gst_structure_get_value(capsStruct, "rate")));
			gst_caps_unref(caps);

			// link decodebin to audioresample
			const gchar *padName = gst_pad_get_name(pad);
			if(GST_PAD_LINK_FAILED(GStreamer::link(GST_BIN(me->m_decodingPipeline), "decodebin", padName, "audioresample", "sink")))
				qCritical() << "Failed to connect decodebin pad" << padName;
			qDebug() << "Selected audio stream #" << me->m_audioStreamCurrent << " [" << padName << "] " << gst_caps_to_string(caps);
		}
	}
}

/*static*/ gboolean
StreamProcessor::onPadCheck(GstElement */*decodebin*/, GstPad *pad, GstCaps *caps, gpointer userData)
{
	StreamProcessor *me = reinterpret_cast<StreamProcessor *>(userData);

	if(!me->m_decodingPipeline || gst_pad_get_direction(pad) != GST_PAD_SRC)
		return TRUE;

	const GstStructure *capsStruct = gst_caps_get_structure(caps, 0);
	const gchar *mimeType = gst_structure_get_name(capsStruct);
	if(strcmp(mimeType, "video/quicktime") == 0
	|| strcmp(mimeType, "video/x-matroska") == 0
	|| strcmp(mimeType, "video/ogg") == 0
	|| strcmp(mimeType, "video/x-msvideo") == 0) {
#if defined(VERBOSE) || !defined(NDEBUG)
	GStreamer::inspectCaps(caps, QStringLiteral("Container stream"));
#endif
		return TRUE;
	} else if(strncmp(mimeType, "audio/", 6) == 0 && me->m_audioStreamIndex >= 0) {
#if defined(VERBOSE) || !defined(NDEBUG)
	GStreamer::inspectCaps(caps, QStringLiteral("Probing stream"));
#endif
		return TRUE;
	} else if(strncmp(mimeType, "text/", 6) == 0 && me->m_textStreamIndex >= 0) {
#if defined(VERBOSE) || !defined(NDEBUG)
	GStreamer::inspectCaps(caps, QStringLiteral("Probing stream"));
#endif
		return TRUE;
	}

	// we don't want to decode unused streams as they will unnecessarily hog the cpu/gpu
#if defined(VERBOSE) || !defined(NDEBUG)
	GStreamer::inspectCaps(caps, QStringLiteral("Ignoring stream"));
#endif
	return FALSE;
}

void
StreamProcessor::decoderMessageProc()
{
	if(!m_decodingBus || !m_decodingPipeline)
		return;

	gint64 time;
	if(gst_element_query_position(GST_ELEMENT(m_decodingPipeline), GST_FORMAT_TIME, &time)) {
		if(m_streamPos != (quint64)time / GST_MSECOND) {
			m_streamPos = time / GST_MSECOND;
			if(m_streamLen)
				emit streamProgress(m_streamPos, m_streamLen);
		}
	}

	GstMessage *msg;
	while(m_decodingBus && m_decodingPipeline && (msg = gst_bus_pop(m_decodingBus))) {
		GstObject *src = GST_MESSAGE_SRC(msg);

#if defined(VERBOSE) || !defined(NDEBUG)
		GStreamer::inspectMessage(msg);
#endif

		if(src == GST_OBJECT(m_decodingPipeline)) {
			switch(GST_MESSAGE_TYPE(msg)) {
			case GST_MESSAGE_STATE_CHANGED: {
				GstState old, current, target;
				gst_message_parse_state_changed(msg, &old, &current, &target);
				if(old > GST_STATE_PAUSED && current <= GST_STATE_PAUSED) {
					emit streamFinished();
					close();
				}
				break;
			}

			case GST_MESSAGE_DURATION: {
				GstFormat format;
				gst_message_parse_duration(msg, &format, &time);
				m_streamLen = time / GST_MSECOND;
				break;
			}

			case GST_MESSAGE_EOS: {
				emit streamFinished();
				close();
				break;
			}

			case GST_MESSAGE_ERROR: {
				gchar *debug = NULL;
				GError *error = NULL;
				gst_message_parse_error(msg, &error, &debug);
				emit streamError(error->code, QString::fromUtf8(error->message), QString::fromUtf8(debug));
				g_error_free(error);
				g_free(debug);
				break;
			}

			default:
				break;
			}
		}

		gst_message_unref(msg);
	}
}
