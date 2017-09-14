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

#include "mpvbackend.h"
#include "mpvconfigwidget.h"

#include "../scconfigdummy.h"


#include <KLocalizedString>

#include <QDebug>

#include <locale>

#include <KMessageBox>

using namespace SubtitleComposer;
using namespace mpv;
using namespace mpv::qt;

MPVBackend::MPVBackend()
	: PlayerBackend(),
	m_mpv(NULL),
	m_initialized(false)
{
	m_name = QStringLiteral("MPV");
}

MPVBackend::~MPVBackend()
{
	if(isInitialized())
		_finalize();
}

/*virtual*/ void
MPVBackend::setSCConfig(SCConfig *scConfig)
{
	scConfigGlobalSet(scConfig);
}

bool
MPVBackend::initialize(VideoWidget *videoWidget)
{
	videoWidget->setVideoLayer(new QWidget());
	return true;
}

void
MPVBackend::finalize()
{
	_finalize();
}

void
MPVBackend::_finalize()
{
	mpvExit();
}

QWidget *
MPVBackend::newConfigWidget(QWidget *parent)
{
	return new MPVConfigWidget(parent);
}

bool
MPVBackend::mpvInit()
{
	// FIXME: libmpv requires LC_NUMERIC category to be set to "C".. is there some nicer way to do this?
	std::setlocale(LC_NUMERIC, "C");

	if(m_mpv)
		mpv_detach_destroy(m_mpv);

	m_mpv = mpv_create();

	if(!m_mpv)
		return false;

	reconfigure();

	// window id
	int64_t winId = player()->videoWidget()->videoLayer()->winId();
	mpv_set_option(m_mpv, "wid", MPV_FORMAT_INT64, &winId);

	// no OSD
	mpv_set_option_string(m_mpv, "osd-level", "0");

	// Disable subtitles
	mpv_set_option_string(m_mpv, "sid", "no");

	// Disable default bindings
	mpv_set_option_string(m_mpv, "input-default-bindings", "no");
	// Disable keyboard input on the X11 window
	mpv_set_option_string(m_mpv, "input-vo-keyboard", "no");
	// Disable mouse input and let us handle mouse hiding
	mpv_set_option_string(m_mpv, "input-cursor", "no");
	mpv_set_option_string(m_mpv, "cursor-autohide", "no");

	// Receive property change events with MPV_EVENT_PROPERTY_CHANGE
	mpv_observe_property(m_mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
	mpv_observe_property(m_mpv, 0, "speed", MPV_FORMAT_DOUBLE);
	mpv_observe_property(m_mpv, 0, "pause", MPV_FORMAT_FLAG);
	mpv_observe_property(m_mpv, 0, "length", MPV_FORMAT_DOUBLE);
	mpv_observe_property(m_mpv, 0, "track-list", MPV_FORMAT_NODE);

	// Request log messages with level "info" or higher.
	// They are received as MPV_EVENT_LOG_MESSAGE.
	mpv_request_log_messages(m_mpv, "info");

	// From this point on, the wakeup function will be called. The callback
	// can come from any thread, so we use the QueuedConnection mechanism to
	// relay the wakeup in a thread-safe way.
	connect(this, SIGNAL(mpvEvents()), this, SLOT(onMPVEvents()), Qt::QueuedConnection);
	mpv_set_wakeup_callback(m_mpv, wakeup, this);

	m_initialized = mpv_initialize(m_mpv) >= 0;
	return m_initialized;
}

void
MPVBackend::mpvExit()
{
	if(m_mpv) {
		mpv_terminate_destroy(m_mpv);
		m_mpv = NULL;
	}
	m_initialized = false;
}


void
MPVBackend::mpvEventHandle(mpv_event *event)
{
	switch(event->event_id) {
	case MPV_EVENT_PROPERTY_CHANGE: {
		mpv_event_property *prop = (mpv_event_property *)event->data;
		if(strcmp(prop->name, "time-pos") == 0) {
			if(prop->format == MPV_FORMAT_DOUBLE) {
				double time = *(double *)prop->data;
				if(!player()->isPlaying() && !player()->isPaused())
					setPlayerState(VideoPlayer::Playing);
				setPlayerPosition(time);
			} else if(prop->format == MPV_FORMAT_NONE) {
				// property is unavailable, probably means that playback was stopped.
				setPlayerState(VideoPlayer::Ready);
			}
		} else if(strcmp(prop->name, "pause") == 0) {
			if(prop->format == MPV_FORMAT_FLAG) {
				int paused = *(int *)prop->data;
				if(paused && !player()->isPaused()) {
					setPlayerState(VideoPlayer::Paused);
				} else if(!paused && player()->isPaused()) {
					setPlayerState(VideoPlayer::Playing);
				}
			}
		} else if(strcmp(prop->name, "track-list") == 0) {
			updateAudioData(prop);
			updateTextData(prop);
		} else if(strcmp(prop->name, "speed") == 0) {
			if(prop->format == MPV_FORMAT_DOUBLE) {
				double rate = *(double *)prop->data;
				playbackRateNotify(rate);
			}
		}
		break;
	}
	case MPV_EVENT_VIDEO_RECONFIG:
		updateVideoData();
		break;

	case MPV_EVENT_LOG_MESSAGE: {
		struct mpv_event_log_message *msg = (struct mpv_event_log_message *)event->data;
		qDebug() << "[MPV:" << msg->prefix << "] " << msg->level << ": " << msg->text;
		if(msg->log_level == MPV_LOG_LEVEL_ERROR && strcmp(msg->prefix, "cplayer") == 0 && player()->state() == VideoPlayer::Opening)
			setPlayerErrorState(msg->text);
		break;
	}
	case MPV_EVENT_SHUTDOWN: {
		mpv_terminate_destroy(m_mpv);
		m_mpv = NULL;
		setPlayerState(VideoPlayer::Ready);
		break;
	}
	default:
		// Ignore uninteresting or unknown events.
		break;
	}
}

void
MPVBackend::updateTextData(const mpv_event_property *prop)
{
	QStringList textStreams;
	if(prop->format == MPV_FORMAT_NODE) {
		const mpv_node *node = (mpv_node *)prop->data;
		if(node->format == MPV_FORMAT_NODE_ARRAY) {
			for(int i = 0; i < node->u.list->num; i++) {
				const mpv_node &val = node->u.list->values[i];
				if(val.format != MPV_FORMAT_NODE_MAP)
					continue;

				const QMap<QString, QVariant> &map = mpv::qt::node_to_variant(&val).toMap();

				if(map[QStringLiteral("type")].toString() != QStringLiteral("sub")
				|| map[QStringLiteral("codec")].toString() != QStringLiteral("mov_text")
				|| map[QStringLiteral("external")].toBool() == true)
					continue;

				const int &id = map[QStringLiteral("id")].toInt();
				const QString &lang = map[QStringLiteral("lang")].toString();
				const QString &title = map[QStringLiteral("title")].toString();

				QString textStreamName = i18n("Text Stream #%1", id);
				if(!lang.isEmpty() && lang != QStringLiteral("und"))
					textStreamName += QStringLiteral(": ") + lang;
				if(!title.isEmpty())
					textStreamName += QStringLiteral(": ") + title;

				textStreams << textStreamName;
			}
		}
	}
	setPlayerTextStreams(textStreams);
}

void
MPVBackend::updateAudioData(const mpv_event_property *prop)
{
	QStringList audioStreams;
	if(prop->format == MPV_FORMAT_NODE) {
		const mpv_node *node = (mpv_node *)prop->data;
		if(node->format == MPV_FORMAT_NODE_ARRAY) {
			for(int i = 0; i < node->u.list->num; i++) {
				const mpv_node &val = node->u.list->values[i];
				if(val.format != MPV_FORMAT_NODE_MAP)
					continue;

				const QMap<QString, QVariant> &map = mpv::qt::node_to_variant(&val).toMap();

				if(map[QStringLiteral("type")].toString() != QStringLiteral("audio"))
					continue;

				const int &id = map[QStringLiteral("id")].toInt();
				const QString &lang = map[QStringLiteral("lang")].toString();
				const QString &title = map[QStringLiteral("title")].toString();
				const QString &codec = map[QStringLiteral("codec")].toString();

				QString audioStreamName = i18n("Audio Stream #%1", id);
				if(!lang.isEmpty() && lang != QStringLiteral("und"))
					audioStreamName += QStringLiteral(": ") + lang;
				if(!title.isEmpty())
					audioStreamName += QStringLiteral(": ") + title;
				if(!codec.isEmpty())
					audioStreamName += QStringLiteral(" [") + codec + QStringLiteral("]");

				audioStreams << audioStreamName;
			}
		}
	}
	setPlayerAudioStreams(audioStreams, audioStreams.isEmpty() ? -1 : 0);
}

void
MPVBackend::updateVideoData()
{
	// Retrieve the new video size.
	int64_t w, h;
	double dar, fps, length;
	if(mpv_get_property(m_mpv, "dwidth", MPV_FORMAT_INT64, &w) >= 0
			&& mpv_get_property(m_mpv, "dheight", MPV_FORMAT_INT64, &h) >= 0
			&& mpv_get_property(m_mpv, "video-aspect", MPV_FORMAT_DOUBLE, &dar)
			&& w > 0 && h > 0) {
		player()->videoWidget()->setVideoResolution(w, h, dar);
	}
	if(mpv_get_property(m_mpv, "estimated-vf-fps", MPV_FORMAT_DOUBLE, &fps) >= 0 && fps > 0)
		setPlayerFramesPerSecond(fps);
	else if(mpv_get_property(m_mpv, "container-fps", MPV_FORMAT_DOUBLE, &fps) >= 0 && fps > 0)
		setPlayerFramesPerSecond(fps);
	if(mpv_get_property(m_mpv, "duration", MPV_FORMAT_DOUBLE, &length) >= 0 && length > 0)
		setPlayerLength(length);
}


// This slot is invoked by wakeup() (through the mpv_events signal).
void
MPVBackend::onMPVEvents()
{
	// Process all events, until the event queue is empty.
	while(m_mpv) {
		mpv_event *event = mpv_wait_event(m_mpv, 0);
		if(event->event_id == MPV_EVENT_NONE)
			break;
		mpvEventHandle(event);
	}
}

/*static*/ void
MPVBackend::wakeup(void *ctx)
{
	// This callback is invoked from any mpv thread (but possibly also
	// recursively from a thread that is calling the mpv API). Just notify
	// the Qt GUI thread to wake up (so that it can process events with
	// mpv_wait_event()), and return as quickly as possible.
	MPVBackend *me = (MPVBackend *)ctx;
	emit me->mpvEvents();
}

bool
MPVBackend::openFile(const QString &filePath, bool &playingAfterCall)
{
	playingAfterCall = true;

	if(!m_mpv && !mpvInit())
		return false;

	QByteArray filename = filePath.toUtf8();
	m_currentFilePath = filePath;
	const char *args[] = { "loadfile", filename.constData(), NULL };
	mpv_command(m_mpv, args);

	if(player()->activeAudioStream() >= 0 && player()->audioStreams().count() > 1)
		mpv_set_option_string(m_mpv, "aid", QString::number(player()->activeAudioStream()).toUtf8().constData());

	return true;
}

void
MPVBackend::closeFile()
{}

bool
MPVBackend::stop()
{
	const char *args[] = { "stop", NULL };
	mpv_command_async(m_mpv, 0, args);
	return true;
}

bool
MPVBackend::play()
{
	if(player()->isStopped()) {
		QByteArray filename = m_currentFilePath.toUtf8();
		const char *args[] = { "loadfile", filename.constData(), NULL };
		mpv_command(m_mpv, args);

		if(player()->activeAudioStream() >= 0 && player()->audioStreams().count() > 1)
			mpv_set_option_string(m_mpv, "aid", QString::number(player()->activeAudioStream()).toUtf8().constData());
	} else {
		const char *args[] = { "cycle", "pause", NULL };
		mpv_command_async(m_mpv, 0, args);
	}
	return true;
}

bool
MPVBackend::pause()
{
	const char *args[] = { "cycle", "pause", NULL };
	mpv_command_async(m_mpv, 0, args);
	return true;
}

bool
MPVBackend::seek(double seconds, bool accurate)
{
	QByteArray strVal = QByteArray::number(seconds);
	if(accurate) {
		const char *args[] = { "seek", strVal.constData(), "absolute", "exact", NULL };
		mpv_command_async(m_mpv, 0, args);
	} else {
		const char *args[] = { "seek", strVal.constData(), "absolute", "keyframes", NULL };
		mpv_command_async(m_mpv, 0, args);
	}
	return true;
}

/*virtual*/ void
MPVBackend::playbackRate(double newRate)
{
	if(newRate > 1.) // without frame dropping we might go out of sync
		mpv_set_option_string(m_mpv, "framedrop", "vo");
	else
		mpv_set_option_string(m_mpv, "framedrop", SCConfig::mpvFrameDropping() ? "vo" : "no");
	mpv_set_option(m_mpv, "speed", MPV_FORMAT_DOUBLE, &newRate);
}

bool
MPVBackend::setActiveAudioStream(int audioStream)
{
	QByteArray strVal = QByteArray::number(audioStream);
	const char *args[] = { "aid", strVal.constData(), NULL };
	mpv_command_async(m_mpv, 0, args);
	return true;
}

bool
MPVBackend::setVolume(double volume)
{
	QByteArray strVal = QByteArray::number(volume);
	const char *args[] = { "set", "volume", strVal.constData(), NULL };
	mpv_command_async(m_mpv, 0, args);
	return true;
}

void
MPVBackend::waitState(VideoPlayer::State state)
{
	while(m_initialized && m_mpv && player()->state() != state) {
		mpv_wait_async_requests(m_mpv);
		QApplication::instance()->processEvents();
	}
}

/*virtual*/ bool
MPVBackend::reconfigure()
{
	if(!m_mpv)
		return false;

	if(SCConfig::mpvVideoOutputEnabled()) {
#if MPV_CLIENT_API_VERSION >= MPV_MAKE_VERSION(1, 21)
		if(SCConfig::mpvVideoOutput() == QStringLiteral("opengl-hq")) {
			mpv_set_option_string(m_mpv, "vo", "opengl");
			mpv_set_option_string(m_mpv, "profile", "opengl-hq");
		} else {
			mpv_set_option_string(m_mpv, "vo", SCConfig::mpvVideoOutput().toUtf8().constData());
		}
#else
		mpv_set_option_string(m_mpv, "vo", SCConfig::mpvVideoOutput().toUtf8().constData());
#endif
	}

	mpv_set_option_string(m_mpv, "hwdec", SCConfig::mpvHwDecodeEnabled() ? SCConfig::mpvHwDecode().toUtf8().constData() : "no");

	if(SCConfig::mpvAudioOutputEnabled())
		mpv_set_option_string(m_mpv, "ao", SCConfig::mpvAudioOutput().toUtf8().constData());

	mpv_set_option_string(m_mpv, "audio-channels", SCConfig::mpvAudioChannelsEnabled() ? QString::number(SCConfig::mpvAudioChannels()).toUtf8().constData() : "auto");

	mpv_set_option_string(m_mpv, "framedrop", SCConfig::mpvFrameDropping() ? "vo" : "no");

	if(SCConfig::mpvAutoSyncEnabled())
		mpv_set_option_string(m_mpv, "autosync", QString::number(SCConfig::mpvAutoSyncFactor()).toUtf8().constData());

	if(SCConfig::mpvCacheEnabled()) {
		mpv_set_option_string(m_mpv, "cache", QString::number(SCConfig::mpvCacheSize()).toUtf8().constData());
//		mpv_set_option_string(m_mpv, "cache-min", "99");
//		mpv_set_option_string(m_mpv, "cache-seek-min", "99");
	} else {
		mpv_set_option_string(m_mpv, "cache", "auto");
	}

	if(SCConfig::mpvVolumeNormalization())
		mpv_set_option_string(m_mpv, "drc", "1:0.25");

	if(SCConfig::mpvVolumeAmplificationEnabled()) {
#if MPV_CLIENT_API_VERSION >= MPV_MAKE_VERSION(1, 22)
		mpv_set_option_string(m_mpv, "volume-max", QString::number(SCConfig::mpvVolumeAmplification()).toUtf8().constData());
#else
		mpv_set_option_string(m_mpv, "softvol", "yes");
		mpv_set_option_string(m_mpv, "softvol-max", QString::number(SCConfig::mpvVolumeAmplification()).toUtf8().constData());
	} else {
		mpv_set_option_string(m_mpv, "softvol", "no");
#endif
	}

	// restart playing
	if(m_initialized && (player()->isPlaying() || player()->isPaused())) {
		bool wasPaused = player()->isPaused();
		double oldPosition;
		mpv_get_property(m_mpv, "time-pos", MPV_FORMAT_DOUBLE, &oldPosition);

		stop();
		waitState(VideoPlayer::Ready);
		play();
		waitState(VideoPlayer::Playing);
		seek(oldPosition, true);
		if(wasPaused)
			pause();
	}

	return true;
}
