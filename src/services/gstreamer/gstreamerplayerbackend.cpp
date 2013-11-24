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
#include <gst/interfaces/xoverlay.h>

#define MESSAGE_UPDATE_AUDIO_DATA 0
#define MESSAGE_UPDATE_VIDEO_DATA 1

using namespace SubtitleComposer;

GStreamerPlayerBackend::GStreamerPlayerBackend(Player *player) :
	PlayerBackend(player, "GStreamer", new GStreamerConfig()),
	m_playbin(NULL),
	m_playbinBus(NULL),
	m_playbinTimer(new QTimer(this)),
	m_lengthInformed(false)
{
	connect(m_playbinTimer, SIGNAL(timeout()), this, SLOT(onPlaybinTimerTimeout()));
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
GStreamerPlayerBackend::setupVideoSink(bool finalizing)
{
	GstElement *videosink;
	g_object_get(m_playbin, "video-sink", &videosink, NULL);
	if(videosink && GST_IS_X_OVERLAY(videosink)) {
		if(finalizing)
			gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(videosink), 0);
		else {
			gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(videosink), player()->videoWidget()->videoLayer()->winId());
			gst_x_overlay_expose(GST_X_OVERLAY(videosink));
		}
	}
}

bool
GStreamerPlayerBackend::openFile(const QString &filePath, bool &playingAfterCall)
{
	playingAfterCall = true;
	m_lengthInformed = false;
	m_usingPlaybin2 = config()->experimentalFeatures();

	qDebug() << "using playbin2:" << m_usingPlaybin2;

	m_playbin = GST_PIPELINE(gst_element_factory_make(m_usingPlaybin2 ? "playbin2" : "playbin", "playbin"));
	GstElement *audiosink = GStreamer::createElement(config()->audioSink() + QString(" alsasink osssink gconfaudiosink artsdsink autoaudiosink"), "audiosink");
	GstElement *videosink = GStreamer::createElement(config()->videoSink() + QString(" xvimagesink ximagesink gconfvideosink autovideosink"), "videosink");
	GstElement *textsink = m_usingPlaybin2 ? gst_element_factory_make("fakesink", "textsink") : 0;

	if(!m_playbin || !audiosink || !videosink || (m_usingPlaybin2 && !textsink)) {
		if(audiosink)
			gst_object_unref(GST_OBJECT(audiosink));
		if(videosink)
			gst_object_unref(GST_OBJECT(videosink));
		if(textsink)
			gst_object_unref(GST_OBJECT(textsink));
		if(m_playbin)
			gst_object_unref(GST_OBJECT(m_playbin));
		m_playbin = 0;
		return false;
	}

	KUrl fileUrl;
	fileUrl.setProtocol("file");
	fileUrl.setPath(filePath);

	g_object_set(G_OBJECT(m_playbin), "uri", fileUrl.url().toUtf8().constData(), NULL);
	g_object_set(G_OBJECT(m_playbin), "suburi", 0, NULL);
	// the volume is adjusted when file playback starts and it's best if it's initially at 0
	g_object_set(G_OBJECT(m_playbin), "volume", (gdouble)0.0, NULL);
	if(m_usingPlaybin2) {
//      g_signal_connect( m_playbin, "audio-changed", G_CALLBACK( audioChanged ), this );
//      g_signal_connect( m_playbin, "video-changed", G_CALLBACK( videoChanged ), this );
	} else {
		// NOTE hiding embedded subtitles works unreliably with old playbin ATM.
		// (it makes playback stutter or halt). We worked around that by making
		// the font as smalls as possible instead of hiding them.
		g_object_set(G_OBJECT(m_playbin), "subtitle-font-desc", "Sans 0", NULL);
	}

	g_object_set(G_OBJECT(m_playbin), "audio-sink", audiosink, NULL);
	g_object_set(G_OBJECT(m_playbin), "video-sink", videosink, NULL);
	if(m_usingPlaybin2)
		g_object_set(G_OBJECT(m_playbin), "text-sink", textsink, NULL);

	m_playbinBus = gst_pipeline_get_bus(GST_PIPELINE(m_playbin));
	m_playbinTimer->start(20);

	setupVideoSink(false);
	GStreamer::setElementState(GST_ELEMENT(m_playbin), GST_STATE_PLAYING, 0 /*don't block waiting */);

	return true;
}

void
GStreamerPlayerBackend::closeFile()
{
	if(m_playbin) {
		m_playbinTimer->stop();
		GStreamer::setElementState(GST_ELEMENT(m_playbin), GST_STATE_NULL, 60000 /*"infinity" wait */);
		setupVideoSink(true);
		GStreamer::freePipeline(&m_playbin, &m_playbinBus);
	}
}

bool
GStreamerPlayerBackend::play()
{
	setupVideoSink(false);
	GStreamer::setElementState(GST_ELEMENT(m_playbin), GST_STATE_PLAYING, 0 /*don't block waiting */);

	return true;
}

bool
GStreamerPlayerBackend::pause()
{
	GStreamer::setElementState(GST_ELEMENT(m_playbin), GST_STATE_PAUSED, 0 /*don't block waiting */);

	return true;
}

bool
GStreamerPlayerBackend::seek(double seconds, bool accurate)
{
	gst_element_seek_simple(GST_ELEMENT(m_playbin),
							GST_FORMAT_TIME, // time in nanoseconds
							(GstSeekFlags)(GST_SEEK_FLAG_FLUSH | (accurate ? GST_SEEK_FLAG_ACCURATE : GST_SEEK_FLAG_KEY_UNIT)),
							(gint64)(seconds * GST_SECOND)
							);

	return true;
}

bool
GStreamerPlayerBackend::stop()
{
	GStreamer::setElementState(GST_ELEMENT(m_playbin), GST_STATE_READY, 0 /* don't block waiting */);

	return true;
}

bool
GStreamerPlayerBackend::setActiveAudioStream(int audioStream)
{
	g_object_set(G_OBJECT(m_playbin), "current-audio", (gint)audioStream, NULL);

	return true;
}

bool
GStreamerPlayerBackend::setVolume(double volume)
{
	g_object_set(G_OBJECT(m_playbin), "volume", (gdouble)(volume * 0.01), NULL);

	return true;
}

void
GStreamerPlayerBackend::onPlaybinTimerTimeout()
{
	if(!isInitialized() || !m_playbin || !m_playbinBus)
		return;

	gint64 time;
	GstFormat fmt = GST_FORMAT_TIME;
	if(!m_lengthInformed && gst_element_query_duration(GST_ELEMENT(m_playbin), &fmt, &time) && GST_CLOCK_TIME_IS_VALID(time)) {
		setPlayerLength((double)time / GST_SECOND);
		m_lengthInformed = true;
	}
	if(gst_element_query_position(GST_ELEMENT(m_playbin), &fmt, &time))
		setPlayerPosition(((double)time / GST_SECOND));

	GstMessage *msg;
	while(m_playbin && m_playbinBus && (msg = gst_bus_pop(m_playbinBus))) {
		GstObject *src = GST_MESSAGE_SRC(msg);

		// we are only interested in error messages or messages directed to the playbin
		if(GST_MESSAGE_TYPE(msg) != GST_MESSAGE_ERROR && src != GST_OBJECT(m_playbin)) {
			gst_message_unref(msg);
			continue;
		}

		GStreamer::inspectMessage(msg);

		switch(GST_MESSAGE_TYPE(msg)) {
//		case GST_MESSAGE_APPLICATION: {
//			gint type = g_value_get_int( gst_structure_get_value(gst_message_get_structure( msg ), "type"));
//			if(type == MESSAGE_UPDATE_AUDIO_DATA)
//				updateAudioData();
//			else if(type == MESSAGE_UPDATE_VIDEO_DATA)
//				updateVideoData();
//			break;
//		}

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
			GStreamer::setElementState(GST_ELEMENT(m_playbin), GST_STATE_PAUSED, 0 /*don't block waiting */);
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

//void
//GStreamerPlayerBackend::audioChanged(GstElement *playbin2, gpointer /*userData*/)
//{
//	qDebug() << "AUDIO CHANGED";
//
//	GstStructure *structure = gst_structure_new("app_message", "type", G_TYPE_INT, MESSAGE_UPDATE_AUDIO_DATA, 0);
//	GstMessage *msg = gst_message_new_application(GST_OBJECT(playbin2), structure);
//	gst_element_post_message(GST_ELEMENT(playbin2), msg);
//}
//
//void
//GStreamerPlayerBackend::videoChanged(GstElement *playbin2, gpointer /*userData*/)
//{
//	qDebug() << "VIDEO CHANGED";
//
//	GstStructure *structure = gst_structure_new("app_message", "type", G_TYPE_INT, MESSAGE_UPDATE_VIDEO_DATA, 0);
//	GstMessage *msg = gst_message_new_application(GST_OBJECT(playbin2), structure);
//	gst_element_post_message(GST_ELEMENT(playbin2), msg);
//}

GList *
GStreamerPlayerBackend::streamInfoForType(const char *type /* "AUDIO", "TEXT", "SUBPICTURE" or "VIDEO" */)
{
	if(m_playbin == NULL)
		return NULL;

	GValueArray *infoArray = NULL;
	g_object_get(G_OBJECT(m_playbin), "stream-info-value-array", &infoArray, NULL);
	if(infoArray == NULL)
		return NULL;

	GList *ret = NULL;
	for(guint idx = 0; idx < infoArray->n_values; ++idx) {
		GObject *info = (GObject *)g_value_get_object(g_value_array_get_nth(infoArray, idx));
		if(info) {
			gint typeID = -1;
			g_object_get(G_OBJECT(info), "type", &typeID, NULL);
			GParamSpec *pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(info), "type");
			GEnumValue *val = g_enum_get_value(G_PARAM_SPEC_ENUM(pspec)->enum_class, typeID);
			if(val) {
				if(g_ascii_strcasecmp(val->value_nick, type) == 0 || g_ascii_strcasecmp(val->value_name, type) == 0)
					ret = g_list_prepend(ret, g_object_ref(info));
			}
		}
	}
	g_value_array_free(infoArray);

	return g_list_reverse(ret);
}

void
GStreamerPlayerBackend::updateAudioData()
{
	QStringList audioStreams;
	gint activeAudioStream;

	if(m_usingPlaybin2) {
		gint audioStreamsCount;
		g_object_get(m_playbin, "n-audio", &audioStreamsCount, "current-audio", &activeAudioStream, NULL);
		if(activeAudioStream < 0)
			activeAudioStream = audioStreamsCount ? 0 : -1;

		for(int audioStream = 0; audioStream < audioStreamsCount; ++audioStream) {
			QString audioStreamName;

			GstTagList *tags = NULL;
			g_signal_emit_by_name(G_OBJECT(m_playbin), "get-audio-tags", audioStream, &tags);
			if(tags) {
				// GStreamer::inspectTags( tags );

				gchar *value;
				if(gst_tag_list_get_string(tags, GST_TAG_LANGUAGE_CODE, &value)) {
					audioStreamName = LanguageCode::nameFromIso3(value);
					g_free(value);
				}

				if(gst_tag_list_get_string(tags, GST_TAG_CODEC, &value)) {
					if(!audioStreamName.isEmpty())
						audioStreamName += " / ";
					audioStreamName += QString(value);
					g_free(value);
				} else {
					if(gst_tag_list_get_string(tags, GST_TAG_AUDIO_CODEC, &value)) {
						if(!audioStreamName.isEmpty())
							audioStreamName += " / ";
						audioStreamName += QString(value);
						g_free(value);
					}
				}
			}

			if(audioStreamName.isEmpty())
				audioStreamName = i18n("Audio Stream #%1", audioStream + 1);

			audioStreams << audioStreamName;
		}
	} else {
		GList *list = streamInfoForType("AUDIO");
		if(list == NULL)
			return;

		gint audioStream = 1;
		for(GList *l = list; l != NULL; l = l->next, ++audioStream) {
			gchar *languageCode = NULL;
			gchar *codec = NULL;
			g_object_get(l->data, GST_TAG_CODEC, &codec, GST_TAG_LANGUAGE_CODE, &languageCode, NULL);

			QString audioStreamName;
			if(languageCode) {
				audioStreamName = LanguageCode::nameFromIso3(languageCode);
				g_free(languageCode);
			}
			if(codec) {
				if(!audioStreamName.isEmpty())
					audioStreamName += " / ";
				audioStreamName += QString(codec);
				g_free(codec);
			} else {
				g_object_get(l->data, GST_TAG_AUDIO_CODEC, &codec, NULL);
				if(!audioStreamName.isEmpty())
					audioStreamName += " / ";
				audioStreamName += QString(codec);
				g_free(codec);
			}

			if(audioStreamName.isEmpty())
				audioStreamName = i18n("Audio Stream #%1", audioStream);
			audioStreams << audioStreamName;
		}

		g_list_foreach(list, (GFunc)g_object_unref, NULL);     // krazy:exclude=c++/foreach
		g_list_free(list);

		// NOTE this value is incorrectly reported by GStreamer. It seems to report the
		// last value set on the property but when video restarts playing (after a stop)
		// the real audio stream playing is always the first stream found (BTW, the order
		// of the streams found can also change after a stop).
		activeAudioStream = audioStreams.isEmpty() ? -1 : 0;
	}

	setPlayerAudioStreams(audioStreams, activeAudioStream);
}

void
GStreamerPlayerBackend::updateVideoData()
{
	GstElement *videosink;
	g_object_get(m_playbin, "video-sink", &videosink, NULL);

	GstPad *videopad = gst_element_get_static_pad(GST_ELEMENT(videosink), "sink");
	if(!videopad)
		return;

	GstCaps *caps = gst_pad_get_negotiated_caps(videopad);
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
