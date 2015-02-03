/***************************************************************************
 *   Copyright (C) 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)  *
 *   Copyright (C) 2010-2015 Mladen Milinkovic <max@smoothware.net>        *
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

#include "mpvbackend.h"
#include "mpvconfigwidget.h"
#include "../../common/qxtsignalwaiter.h"

#include <KDebug>
#include <KLocale>
#include <KStandardDirs>
#include <locale>

using namespace SubtitleComposer;
using namespace mpv;
using namespace mpv::qt;

MPVBackend::MPVBackend(Player *player) :
	PlayerBackend(player, "MPV", new MPVConfig()),
	m_mpv(NULL)
{
}

MPVBackend::~MPVBackend()
{
	if(isInitialized())
		_finalize();
}

VideoWidget *
MPVBackend::initialize(QWidget *videoWidgetParent)
{
	return new VideoWidget(videoWidgetParent);
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

SubtitleComposer::AppConfigGroupWidget *
MPVBackend::newAppConfigGroupWidget(QWidget *parent)
{
	return new MPVConfigWidget(parent);
}

bool
MPVBackend::mpvInit()
{
	// TODO: libmpv requires LC_NUMERIC category to be set to "C".. is there some nicer way to do this?
	std::setlocale(LC_NUMERIC, "C");

	if(m_mpv)
		mpv_detach_destroy(m_mpv);

	m_mpv = mpv_create();

	if(!m_mpv)
		return false;

	if(config()->hasVideoOutput())
		mpv_set_option_string(m_mpv, "vo", config()->videoOutput().toUtf8().constData());

	mpv_set_option_string(m_mpv, "hwdec", config()->hasHwDecode() ? config()->hwDecode().toUtf8().constData() : "no");

	if(config()->hasAudioOutput())
		mpv_set_option_string(m_mpv, "ao", config()->audioOutput().toUtf8().constData());

	if(config()->hasAudioChannels())
		mpv_set_option_string(m_mpv, "audio-channels", QString::number(config()->audioChannels()).toUtf8().constData());

	if(config()->frameDropping())
		mpv_set_option_string(m_mpv, "framedrop", "vo");

	if(config()->hasAutoSyncFactor())
		mpv_set_option_string(m_mpv, "autosync", QString::number(config()->autoSyncFactor()).toUtf8().constData());

	if(config()->hasCacheSize()) {
		mpv_set_option_string(m_mpv, "cache", QString::number(config()->cacheSize()).toUtf8().constData());
//		mpv_set_option_string(m_mpv, "cache-min", "99");
//		mpv_set_option_string(m_mpv, "cache-seek-min", "99");
	}

	// window id
	int64_t winId = player()->videoWidget()->videoLayer()->winId();
	mpv_set_option(m_mpv, "wid", MPV_FORMAT_INT64, &winId);

	// no OSD
	mpv_set_option_string(m_mpv, "osd-level", "0");

	if(config()->volumeNormalization())
		mpv_set_option_string(m_mpv, "drc", "1:0.25");

	mpv_set_option_string(m_mpv, "softvol", "yes");
	if(config()->hasVolumeAmplification())
		mpv_set_option_string(m_mpv, "softvol-max", QString::number(config()->volumeAmplification()).toUtf8().constData());

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

	return mpv_initialize(m_mpv) >= 0;
}

void
MPVBackend::mpvExit()
{
	if(m_mpv) {
		mpv_terminate_destroy(m_mpv);
		m_mpv = NULL;
	}
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
					setPlayerState(Player::Playing);
				setPlayerPosition(time);
			} else if(prop->format == MPV_FORMAT_NONE) {
				// property is unavailable, probably means that playback was stopped.
				setPlayerState(Player::Ready);
			}
		} else if(strcmp(prop->name, "pause") == 0) {
			if(prop->format == MPV_FORMAT_FLAG) {
				int paused = *(int *)prop->data;
				if(paused && !player()->isPaused()) {
					setPlayerState(Player::Paused);
				} else if(!paused && player()->isPaused()) {
					setPlayerState(Player::Playing);
				}
			}
		} else if(strcmp(prop->name, "length") == 0) {
			if(prop->format == MPV_FORMAT_DOUBLE) {
				double duration = *(double *)prop->data;
				setPlayerLength(duration);
			}
		} else if(strcmp(prop->name, "track-list") == 0) {
			QStringList audioStreams;
			if(prop->format == MPV_FORMAT_NODE) {
				mpv_node *node = (mpv_node *)prop->data;
				if(node->format == MPV_FORMAT_NODE_ARRAY) {
					for(int i = 0; i < node->u.list->num; i++) {
						mpv_node &val = node->u.list->values[i];
						if(val.format == MPV_FORMAT_NODE_MAP) {
							QMap<QString, QVariant> map = mpv::qt::node_to_variant(&val).toMap();
							if(map["type"].toString() != "audio")
								continue;

							QString audioStreamName;
							if(map.contains("lang") && !map["lang"].toString().isEmpty())
								audioStreamName = map["lang"].toString();
							if(map.contains("title") && !map["title"].toString().isEmpty()) {
								if(!audioStreamName.isEmpty())
									audioStreamName += " / ";
								audioStreamName += map["title"].toString();
							}
							if(audioStreamName.isEmpty())
								audioStreamName = i18n("Stream #%1", map["id"].toLongLong());
							if(map.contains("codec") && !map["codec"].toString().isEmpty()) {
								audioStreamName += " [";
								audioStreamName += map["codec"].toString();
								audioStreamName += "]";
							}
							audioStreams << audioStreamName;
						}
					}
				}
			}
			setPlayerAudioStreams(audioStreams, audioStreams.isEmpty() ? -1 : 0);
		}
		break;
	}
	case MPV_EVENT_VIDEO_RECONFIG: {
		// Retrieve the new video size.
		int64_t w, h;
		double dar, fps;
		if(mpv_get_property(m_mpv, "dwidth", MPV_FORMAT_INT64, &w) >= 0
				&& mpv_get_property(m_mpv, "dheight", MPV_FORMAT_INT64, &h) >= 0
				&& mpv_get_property(m_mpv, "video-aspect", MPV_FORMAT_DOUBLE, &dar)
				&& w > 0 && h > 0) {
			player()->videoWidget()->setVideoResolution(w, h, dar);
		}
		if(mpv_get_property(m_mpv, "fps", MPV_FORMAT_DOUBLE, &fps) >= 0 && fps > 0) {
			setPlayerFramesPerSecond(fps);
		}

		break;
	}
	case MPV_EVENT_LOG_MESSAGE: {
		struct mpv_event_log_message *msg = (struct mpv_event_log_message *)event->data;
		kDebug() << "[MPV:" << msg->prefix << "] " << msg->level << ": " << msg->text;
		break;
	}
	case MPV_EVENT_SHUTDOWN: {
		mpv_terminate_destroy(m_mpv);
		m_mpv = NULL;
		setPlayerState(Player::Ready);
		break;
	}
	default:
		// Ignore uninteresting or unknown events.
		break;
	}
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
	currentFilePath = filePath;
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
		QByteArray filename = currentFilePath.toUtf8();
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

#include "mpvbackend.moc"
