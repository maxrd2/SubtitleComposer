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

#include "gstreamer.h"
#include "gstreamerplayerbackend.h"
#include "gstreamerconfigwidget.h"
#include "../../common/languagecode.h"

#include <QtCore/QTimer>

#include <KDebug>
#include <KGlobal>
#include <KLocale>
#include <KUrl>

#include <gst/gst.h>
#include <gst/video/videooverlay.h>

#define MESSAGE_UPDATE_AUDIO_DATA 0
#define MESSAGE_UPDATE_VIDEO_DATA 1

using namespace SubtitleComposer;

#ifndef __GST_PLAY_ENUM_H__
/**
 * GstPlayFlags:
 * @GST_PLAY_FLAG_VIDEO: Enable rendering of the video stream
 * @GST_PLAY_FLAG_AUDIO: Enable rendering of the audio stream
 * @GST_PLAY_FLAG_TEXT: Enable rendering of subtitles
 * @GST_PLAY_FLAG_VIS: Enable rendering of visualisations when there is
 *       no video stream.
 * @GST_PLAY_FLAG_SOFT_VOLUME: Use software volume
 * @GST_PLAY_FLAG_NATIVE_AUDIO: only allow native audio formats, this omits
 *   configuration of audioconvert and audioresample.
 * @GST_PLAY_FLAG_NATIVE_VIDEO: only allow native video formats, this omits
 *   configuration of videoconvert and videoscale.
 * @GST_PLAY_FLAG_DOWNLOAD: enable progressice download buffering for selected
 *   formats.
 * @GST_PLAY_FLAG_BUFFERING: enable buffering of the demuxed or parsed data.
 * @GST_PLAY_FLAG_DEINTERLACE: deinterlace raw video (if native not forced).
 * @GST_PLAY_FLAG_FORCE_FILTERS: force audio/video filters to be applied if
 *   set.
 *
 * Extra flags to configure the behaviour of the sinks.
 */
typedef enum {
  GST_PLAY_FLAG_VIDEO         = (1 << 0),
  GST_PLAY_FLAG_AUDIO         = (1 << 1),
  GST_PLAY_FLAG_TEXT          = (1 << 2),
  GST_PLAY_FLAG_VIS           = (1 << 3),
  GST_PLAY_FLAG_SOFT_VOLUME   = (1 << 4),
  GST_PLAY_FLAG_NATIVE_AUDIO  = (1 << 5),
  GST_PLAY_FLAG_NATIVE_VIDEO  = (1 << 6),
  GST_PLAY_FLAG_DOWNLOAD      = (1 << 7),
  GST_PLAY_FLAG_BUFFERING     = (1 << 8),
  GST_PLAY_FLAG_DEINTERLACE   = (1 << 9),
  GST_PLAY_FLAG_SOFT_COLORBALANCE = (1 << 10),
  GST_PLAY_FLAG_FORCE_FILTERS = (1 << 11),
} GstPlayFlags;
#endif /* __GST_PLAY_ENUM_H__ */

GStreamerPlayerBackend::GStreamerPlayerBackend(Player *player) :
	PlayerBackend(player, "GStreamer", new GStreamerConfig()),
	m_pipeline(NULL),
	m_pipelineBus(NULL),
	m_pipelineTimer(new QTimer(this)),
	m_lengthInformed(false)
{
	connect(m_pipelineTimer, SIGNAL(timeout()), this, SLOT(onPlaybinTimerTimeout()));
}

GStreamerPlayerBackend::~GStreamerPlayerBackend()
{
	if(isInitialized())
		GStreamer::deinit();
}

VideoWidget *
GStreamerPlayerBackend::initialize(QWidget *videoWidgetParent)
{
	if(!GStreamer::init())
		return 0;

	return new VideoWidget(videoWidgetParent);
}

void
GStreamerPlayerBackend::finalize()
{
	return GStreamer::deinit();
}

SubtitleComposer::AppConfigGroupWidget *
GStreamerPlayerBackend::newAppConfigGroupWidget(QWidget *parent)
{
	return new GStreamerConfigWidget(parent);
}


void
GStreamerPlayerBackend::setupVideoOverlay()
{
	gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(m_pipeline), player()->videoWidget()->videoLayer()->winId());
	gst_video_overlay_expose(GST_VIDEO_OVERLAY(m_pipeline));
}

bool
GStreamerPlayerBackend::openFile(const QString &filePath, bool &playingAfterCall)
{
	playingAfterCall = true;
	m_lengthInformed = false;

	m_pipeline = GST_PIPELINE(gst_element_factory_make("playbin", "playbin"));
	GstElement *audiosink = GStreamer::createElement(config()->audioSink() + QString(" pulsesink alsasink osssink gconfaudiosink artsdsink autoaudiosink"), "audiosink");
	GstElement *videosink = GStreamer::createElement(config()->videoSink() + QString(" xvimagesink ximagesink gconfvideosink autovideosink"), "videosink");

	if(!m_pipeline || !audiosink || !videosink) {
		if(audiosink)
			gst_object_unref(GST_OBJECT(audiosink));
		if(videosink)
			gst_object_unref(GST_OBJECT(videosink));
		if(m_pipeline)
			gst_object_unref(GST_OBJECT(m_pipeline));
		m_pipeline = 0;
		return false;
	}

	KUrl fileUrl;
	fileUrl.setProtocol("file");
	fileUrl.setPath(filePath);

	g_object_set(G_OBJECT(m_pipeline), "uri", fileUrl.url().toUtf8().constData(), NULL);
	g_object_set(G_OBJECT(m_pipeline), "suburi", 0, NULL);

	// disable embedded subtitles
	gint flags = 0;
	g_object_get(G_OBJECT(m_pipeline), "flags", &flags, NULL);
	g_object_set(G_OBJECT(m_pipeline), "flags", flags & ~GST_PLAY_FLAG_TEXT, NULL);

	// the volume is adjusted when file playback starts and it's best if it's initially at 0
	g_object_set(G_OBJECT(m_pipeline), "volume", (gdouble)0.0, NULL);

	g_object_set(G_OBJECT(m_pipeline), "audio-sink", audiosink, NULL);
	g_object_set(G_OBJECT(m_pipeline), "video-sink", videosink, NULL);

	m_pipelineBus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
	m_pipelineTimer->start(20);

	setupVideoOverlay();

	GStreamer::setElementState(GST_ELEMENT(m_pipeline), GST_STATE_PLAYING, 0);

	return true;
}

void
GStreamerPlayerBackend::closeFile()
{
	if(m_pipeline) {
		m_pipelineTimer->stop();
		GStreamer::setElementState(GST_ELEMENT(m_pipeline), GST_STATE_NULL, 60000 /*"infinity" wait */);
		GStreamer::freePipeline(&m_pipeline, &m_pipelineBus);
	}
}

bool
GStreamerPlayerBackend::play()
{
	setupVideoOverlay();
	GStreamer::setElementState(GST_ELEMENT(m_pipeline), GST_STATE_PLAYING, 0);

	return true;
}

bool
GStreamerPlayerBackend::pause()
{
	GStreamer::setElementState(GST_ELEMENT(m_pipeline), GST_STATE_PAUSED, 0);

	return true;
}

bool
GStreamerPlayerBackend::seek(double seconds, bool accurate)
{
	gst_element_seek_simple(GST_ELEMENT(m_pipeline),
		GST_FORMAT_TIME, // time in nanoseconds
		(GstSeekFlags)(GST_SEEK_FLAG_FLUSH | (accurate ? GST_SEEK_FLAG_ACCURATE : GST_SEEK_FLAG_KEY_UNIT)),
		(gint64)(seconds * GST_SECOND));

	return true;
}

bool
GStreamerPlayerBackend::stop()
{
	GStreamer::setElementState(GST_ELEMENT(m_pipeline), GST_STATE_READY, 0);

	return true;
}

bool
GStreamerPlayerBackend::setActiveAudioStream(int audioStream)
{
	g_object_set(G_OBJECT(m_pipeline), "current-audio", (gint)audioStream, NULL);

	return true;
}

bool
GStreamerPlayerBackend::setVolume(double volume)
{
	g_object_set(G_OBJECT(m_pipeline), "volume", (gdouble)(volume * 0.01), NULL);

	return true;
}

void
GStreamerPlayerBackend::onPlaybinTimerTimeout()
{
	if(!isInitialized() || !m_pipeline || !m_pipelineBus)
		return;

	gint64 time;
	if(!m_lengthInformed && gst_element_query_duration(GST_ELEMENT(m_pipeline), GST_FORMAT_TIME, &time) && GST_CLOCK_TIME_IS_VALID(time)) {
		setPlayerLength((double)time / GST_SECOND);
		m_lengthInformed = true;
	}
	if(gst_element_query_position(GST_ELEMENT(m_pipeline), GST_FORMAT_TIME, &time))
		setPlayerPosition(((double)time / GST_SECOND));

	GstMessage *msg;
	while(m_pipeline && m_pipelineBus && (msg = gst_bus_pop(m_pipelineBus))) {
		GstObject *src = GST_MESSAGE_SRC(msg);

		// we are only interested in error messages or messages directed to the playbin
		if(GST_MESSAGE_TYPE(msg) != GST_MESSAGE_ERROR && src != GST_OBJECT(m_pipeline)) {
			gst_message_unref(msg);
			continue;
		}

		GStreamer::inspectMessage(msg);

		switch(GST_MESSAGE_TYPE(msg)) {
		case GST_MESSAGE_STATE_CHANGED: {
			GstState old, current, target;
			gst_message_parse_state_changed(msg, &old, &current, &target);

			if(current == GST_STATE_PAUSED)
				setPlayerState(Player::Paused);
			else if(current == GST_STATE_PLAYING)
				setPlayerState(Player::Playing);
			else if(current == GST_STATE_READY)
				setPlayerState(Player::Ready);

			if(old == GST_STATE_READY) {
				updateAudioData();
				updateVideoData();
			}
			break;
		}

		case GST_MESSAGE_EOS: {
			setPlayerState(Player::Ready);
			seek(0, true);
			GStreamer::setElementState(GST_ELEMENT(m_pipeline), GST_STATE_PAUSED, 0);
			break;
		}

		case GST_MESSAGE_ERROR: {
			gchar *debug = NULL;
			GError *error = NULL;
			gst_message_parse_error(msg, &error, &debug);
			// setPlayerErrorState(QString(error->message));
			setPlayerErrorState(QString(debug));
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

void
GStreamerPlayerBackend::updateAudioData()
{
	QStringList audioStreams;
	gint activeAudioStream;

	gint n;
	g_object_get(m_pipeline, "n-audio", &n, NULL);
	for(gint i = 0; i < n; i++) {
		QString audioStreamName;
		GstTagList *tags = NULL;
		guint rate;
		gchar *str;
		g_signal_emit_by_name(m_pipeline, "get-audio-tags", i, &tags);
		if(tags) {
			audioStreamName = i18n("Audio Stream #%1", i);
			if(gst_tag_list_get_string(tags, GST_TAG_LANGUAGE_CODE, &str)) {
				audioStreamName += ": " + LanguageCode::nameFromIso(str);
				g_free(str);
			}
			if(gst_tag_list_get_string(tags, GST_TAG_AUDIO_CODEC, &str)) {
				audioStreamName += QString(" [") + str + "]";
				g_free(str);
			}
			if(gst_tag_list_get_uint(tags, GST_TAG_BITRATE, &rate)) {
				audioStreamName += " " + QString::number(rate / 1000) + "kbps";
			}
			gst_tag_list_free(tags);

			audioStreams << audioStreamName;
		}
	}

	g_object_get(m_pipeline, "current-audio", &activeAudioStream, NULL);

	setPlayerAudioStreams(audioStreams, activeAudioStream);
}

void
GStreamerPlayerBackend::updateVideoData()
{
	GstElement *videosink;
	g_object_get(m_pipeline, "video-sink", &videosink, NULL);

	GstPad *videopad = gst_element_get_static_pad(GST_ELEMENT(videosink), "sink");
	if(!videopad)
		return;

	GstCaps *caps = gst_pad_get_current_caps(videopad);
	if(!caps)
		return;

	const GstStructure *capsStruct = gst_caps_get_structure(caps, 0);
	if(!capsStruct)
		return;

	gint width = 0, height = 0;
	gst_structure_get_int(capsStruct, "width", &width);
	gst_structure_get_int(capsStruct, "height", &height);

	double dar = 0.0;
	const GValue *par;
	if((par = gst_structure_get_value(capsStruct, "pixel-aspect-ratio"))) {
		dar = (double)gst_value_get_fraction_numerator(par) / gst_value_get_fraction_denominator(par);
		dar = dar * width / height;
	}

	player()->videoWidget()->setVideoResolution(width, height, dar);

	const GValue *fps;
	if((fps = gst_structure_get_value(capsStruct, "framerate"))) {
		int num = gst_value_get_fraction_numerator(fps);
		int den = gst_value_get_fraction_denominator(fps);
		setPlayerFramesPerSecond((double)num / den);
	}

	gst_caps_unref(caps);
	gst_object_unref(videopad);
}

#include "gstreamerplayerbackend.moc"
