/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2019 Mladen Milinkovic <max@smoothware.net>
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

#include "xineplayerbackend.h"
#include "xinevideolayerwidget.h"
#include "xineconfigwidget.h"
#include "xineconfig.h"

#include <QEventLoop>
#include <QEvent>
#include <QDir>
#include <QFile>
#include <QApplication>
#include <QDebug>
#include <QUrl>

#include <KLocalizedString>

#include <xine/xineutils.h>
#include <X11/Xlib.h>

using namespace SubtitleComposer;

#define EVENT_PLAYBACK_FINISHED (QEvent::User + 1)
#define EVENT_CHANNELS_CHANGED (QEvent::User + 2)
#define EVENT_FRAME_FORMAT_CHANGED (QEvent::User + 3)

#define UPDATE_INTERVAL 50

XinePlayerBackend::XinePlayerBackend()
	: PlayerBackend(),
	  m_state(STOPPED),
	  m_connection(nullptr),
	  m_xineEngine(nullptr),
	  m_audioDriver(nullptr),
	  m_videoDriver(nullptr),
	  m_xineStream(nullptr),
	  m_eventQueue(nullptr),
	  m_videoLayer(nullptr),
	  m_updatePosition(true),
	  m_softwareMixer(false),
	  m_streamIsSeekable(false)
{
	m_name = QStringLiteral("Xine");
	connect(&m_timesTimer, &QTimer::timeout, this, &XinePlayerBackend::updatePosition);
	//connect(XineConfig::self(), &XineConfig::configChanged, this, &XinePlayerBackend::reconfigure);
}

XinePlayerBackend::~XinePlayerBackend()
{
	cleanup();
}

bool
XinePlayerBackend::init(QWidget *videoWidget)
{
	if(!m_videoLayer) {
		m_videoLayer = new XineVideoLayerWidget(videoWidget);
		connect(m_videoLayer, &XineVideoLayerWidget::geometryChanged, this, &XinePlayerBackend::onVideoLayerGeometryChanged);
	} else {
		m_videoLayer->setParent(videoWidget);
	}

	if(!initializeXine(m_videoLayer->winId())) {
		finalizeXine();
		qCritical() << "xine initialization failed!";
		return false;
	}

	m_videoLayer->setVideoDriver(m_videoDriver);

	return true;
}

void
XinePlayerBackend::cleanup()
{
	m_timesTimer.stop();

	finalizeXine();
}

QWidget *
XinePlayerBackend::newConfigWidget(QWidget *parent)
{
	return new XineConfigWidget(parent);
}

KCoreConfigSkeleton *
XinePlayerBackend::config() const
{
	return XineConfig::self();
}

bool
XinePlayerBackend::openFile(const QString &filePath)
{
	// the volume is adjusted when file playback starts and it's best if it's initially at 0
	xine_set_param(m_xineStream, m_softwareMixer ? XINE_PARAM_AUDIO_AMP_LEVEL : XINE_PARAM_AUDIO_VOLUME, 0);

	m_streamIsSeekable = false;

	QUrl fileUrl;
	fileUrl.setScheme("file");
	fileUrl.setPath(filePath);

	if(!xine_open(m_xineStream, fileUrl.url().toLocal8Bit()))
		return false;

	// no subtitles
	xine_set_param(m_xineStream, XINE_PARAM_SPU_CHANNEL, -1);

	if(!xine_play(m_xineStream, 0, 0))
		return false;

	setState(PLAYING);

	// this methods do nothing if the information is not available
	updateVideoData();
	updateAudioData();
	updatePosition();

	m_timesTimer.start(UPDATE_INTERVAL);

	return true;
}

bool
XinePlayerBackend::closeFile()
{
	return true;
}

bool
XinePlayerBackend::play()
{
	if(xine_get_status(m_xineStream) != XINE_STATUS_PLAY)
		xine_play(m_xineStream, 0, 0); // was stopped
	else
		xine_set_param(m_xineStream, XINE_PARAM_SPEED, XINE_SPEED_NORMAL); // was paused

	setState(PLAYING);

	m_timesTimer.start(UPDATE_INTERVAL);

	return true;
}

bool
XinePlayerBackend::pause()
{
	m_timesTimer.stop();

	xine_set_param(m_xineStream, XINE_PARAM_SPEED, XINE_SPEED_PAUSE);
	setState(PAUSED);

	return true;
}

bool
XinePlayerBackend::seek(double seconds)
{
	if(m_streamIsSeekable) {
		int targetTime = (int)(seconds * 1000 + 0.5);

		if(m_state == PAUSED)
			xine_set_param(m_xineStream, m_softwareMixer ? XINE_PARAM_AUDIO_AMP_MUTE : XINE_PARAM_AUDIO_MUTE, 1);

		xine_play(m_xineStream, 0, targetTime);

		{ // seek to frame
			int time;

			if(!xine_get_pos_length(m_xineStream, 0, &time, 0))
				return true;

			m_updatePosition = false;

			xine_set_param(m_xineStream, XINE_PARAM_SPEED, XINE_SPEED_FAST_4);

			while(targetTime - time > 200 /*|| time - targetTime > 100 || */) {
				QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

				if(!xine_get_pos_length(m_xineStream, 0, &time, 0))
					break;
			}

			if(m_state != PAUSED)
				xine_set_param(m_xineStream, XINE_PARAM_SPEED, XINE_SPEED_NORMAL);

			m_updatePosition = true;
		}

		if(m_state == PAUSED) {
			xine_set_param(m_xineStream, XINE_PARAM_SPEED, XINE_SPEED_PAUSE);
			xine_set_param(m_xineStream, m_softwareMixer ? XINE_PARAM_AUDIO_AMP_MUTE : XINE_PARAM_AUDIO_MUTE, 0);
		}
	}

	return true;
}

bool
XinePlayerBackend::stop()
{
	m_timesTimer.stop();

	xine_stop(m_xineStream);
	setState(STOPPED);

	return true;
}

bool
XinePlayerBackend::selectAudioStream(int streamIndex)
{
	xine_set_param(m_xineStream, XINE_PARAM_AUDIO_CHANNEL_LOGICAL, streamIndex);

	return true;
}

bool
XinePlayerBackend::setVolume(double volume)
{
	xine_set_param(m_xineStream, m_softwareMixer ? XINE_PARAM_AUDIO_AMP_LEVEL : XINE_PARAM_AUDIO_VOLUME, (int)(volume + 0.5));

	return true;
}

void
XinePlayerBackend::updateVideoData()
{
	if(!m_xineStream)
		return;

	if(xine_get_stream_info(m_xineStream, XINE_STREAM_INFO_HAS_VIDEO)) {
		int fps = xine_get_stream_info(m_xineStream, XINE_STREAM_INFO_FRAME_DURATION);
		if(fps > 0) {
			emit fpsChanged(90000.0 / fps);

			// tweak prebuffer so we can be sure to show only a single frame
			// qDebug() << "PREBUFFER " << xine_get_param(m_xineStream, XINE_PARAM_METRONOM_PREBUFFER);
			xine_set_param(m_xineStream, XINE_PARAM_METRONOM_PREBUFFER, fps);
			// qDebug() << "PREBUFFER " << xine_get_param(m_xineStream, XINE_PARAM_METRONOM_PREBUFFER);
		}

		int width = xine_get_stream_info(m_xineStream, XINE_STREAM_INFO_VIDEO_WIDTH);
		int height = xine_get_stream_info(m_xineStream, XINE_STREAM_INFO_VIDEO_HEIGHT);
		double dar = xine_get_stream_info(m_xineStream, XINE_STREAM_INFO_VIDEO_RATIO) / 10000.0;
		emit resolutionChanged(width, height, dar);
	} else {
		emit resolutionChanged(0, 0, 0.);
	}

	m_streamIsSeekable = xine_get_stream_info(m_xineStream, XINE_STREAM_INFO_SEEKABLE);

	onVideoLayerGeometryChanged();
}

void
XinePlayerBackend::updateAudioData()
{
	if(!m_xineStream)
		return;

	QStringList audioStreams;

	int channels = xine_get_stream_info(m_xineStream, XINE_STREAM_INFO_MAX_AUDIO_CHANNEL);
	for(int index = 0; index < channels; ++index) {
		QString audioStreamName = i18n("Audio Stream #%1", index + 1);
		char lang[128];
		if(xine_get_audio_lang(m_xineStream, index, lang))
			audioStreamName += QStringLiteral(" - ") + lang;
		audioStreams << audioStreamName;
	}

	emit audioStreamsChanged(audioStreams, audioStreams.isEmpty() ? -1 : 0);
}

void
XinePlayerBackend::updateLengthData()
{
	if(!m_xineStream)
		return;

	int time, length;
	if(xine_get_pos_length(m_xineStream, 0, &time, &length))
		emit lengthChanged(length / 1000.0);
}

void
XinePlayerBackend::updatePosition()
{
	if(!m_xineStream || !m_updatePosition)
		return;

	// some streams make xine sometimes report spurious position data during playback
	// to compensate this we check if the received position is not too far away from the
	// previously received one (not further than 200ms).

	static int prevTime;

	static int time, length;
	if(xine_get_pos_length(m_xineStream, 0, &time, &length)) {
		if(time < prevTime + 200 || time < prevTime)
			emit positionChanged(time / 1000.0);

		prevTime = time;
	}
}

void
XinePlayerBackend::onVideoLayerGeometryChanged()
{
	const QSize size = m_videoLayer->size();
	const QPoint globalPos = m_videoLayer->mapToGlobal(QPoint(0, 0));
	m_videoLayerGeometry = QRect(globalPos.x(), globalPos.y(), size.width(), size.height());
}

void
XinePlayerBackend::xineEventListener(void *p, const xine_event_t *event)
{
	if(p == nullptr)
		return;

	XinePlayerBackend *xinePlayer = (XinePlayerBackend *)p;

	switch(event->type) {
	case XINE_EVENT_UI_PLAYBACK_FINISHED:
		QApplication::postEvent(xinePlayer, new QEvent((QEvent::Type)EVENT_PLAYBACK_FINISHED));
		break;

	case XINE_EVENT_UI_CHANNELS_CHANGED:
		QApplication::postEvent(xinePlayer, new QEvent((QEvent::Type)EVENT_CHANNELS_CHANGED));
		break;

	case XINE_EVENT_FRAME_FORMAT_CHANGE:
		QApplication::postEvent(xinePlayer, new QEvent((QEvent::Type)EVENT_FRAME_FORMAT_CHANGED));
		break;

	default:
		break;
	}
}

void
XinePlayerBackend::customEvent(QEvent *event)
{
	switch((int)event->type()) {
	case EVENT_PLAYBACK_FINISHED:
		stop();
		break;

	case EVENT_CHANNELS_CHANGED:
		updateAudioData();
		updateLengthData();
		break;

	case EVENT_FRAME_FORMAT_CHANGED:
		// updateLengthData();
		updateVideoData();
		break;

	default:
		break;
	}
}

bool
XinePlayerBackend::initializeXine(WId winId)
{
	int screen_nbr = 0;
	m_connection = xcb_connect(nullptr, &screen_nbr);

	if(!m_connection) {
		qDebug() << "Failed to connect to X-Server!";
		return false;
	}

	if(!(m_xineEngine = xine_new())) {
		qDebug() << "Couldn't init xine Engine!";
		return false;
	}

	QString configFilePath(QDir::homePath() + "/.xine/config");
	if(QFile::exists(configFilePath))
		xine_config_load(m_xineEngine, QFile::encodeName(configFilePath));

	xine_init(m_xineEngine);

	m_softwareMixer = (bool)xine_config_register_bool(m_xineEngine, "audio.mixer_software", 1, nullptr, nullptr, 10, &XinePlayerBackend::audioMixerMethodChangedCallback, this);

	xcb_screen_iterator_t screen_it = xcb_setup_roots_iterator(xcb_get_setup(m_connection));
	while(screen_it.rem > 1 && screen_nbr > 0) {
		xcb_screen_next(&screen_it);
		--screen_nbr;
	}
	m_x11Visual.connection = m_connection;
	m_x11Visual.screen = screen_it.data;
	m_x11Visual.window = winId;
//  m_x11Visual.dest_size_cb = &XinePlayerBackend::destSizeCallback;
	m_x11Visual.frame_output_cb = &XinePlayerBackend::frameOutputCallback;
	m_x11Visual.user_data = (void *)this;

	QStringList videoDriverNames = QStringLiteral("xv xvmc opengl xxmc sdl xshm fb XDirectFB DirectFB aa caca auto").split(' ');
	if(XineConfig::videoEnabled())
		videoDriverNames.prepend(XineConfig::video());
	foreach(QString videoDriver, videoDriverNames) {
		if(videoDriver.isEmpty())
			continue;

		{
			// NOTE: magical fix follows... make sure all Qt<->X communication is done, else xine_open_video_driver will crash
			static Display* display = 0;
			if(!display)
				display = XOpenDisplay(0);
			Q_ASSERT(display);
			XSync(display, false);
		}

		m_videoDriver = xine_open_video_driver(m_xineEngine, videoDriver.toLatin1(),
			XINE_VISUAL_TYPE_XCB,
			(void *)&(m_x11Visual)
		);

		if(m_videoDriver)
			break;
	}

	if(!m_videoDriver) {
		qDebug() << "All video drivers failed to initialize!";
		return false;
	}

	QStringList audioDriverNames = QStringLiteral("alsa oss jack pulseaudio esd auto").split(' ');
	if(XineConfig::audioEnabled())
		audioDriverNames.prepend(XineConfig::audio());
	foreach(QString audioDriver, audioDriverNames) {
		if(!audioDriver.isEmpty() && (m_audioDriver = xine_open_audio_driver(m_xineEngine, audioDriver.toLatin1(), nullptr)) != nullptr)
			break;
	}

	if(!m_audioDriver) {
		qDebug() << "All audio drivers failed to initialize!";
		return false;
	}

	m_xineStream = xine_stream_new(m_xineEngine, m_audioDriver, m_videoDriver);

	if(!m_xineStream) {
		qDebug() << "Couldn't create a new xine stream!";
		return false;
	}

	m_eventQueue = xine_event_new_queue(m_xineStream);
	xine_event_create_listener_thread(m_eventQueue, &XinePlayerBackend::xineEventListener, (void *)this);

	return true;
}

void
XinePlayerBackend::finalizeXine()
{
	if(m_xineStream)
		xine_close(m_xineStream);

	if(m_eventQueue) {
		xine_event_dispose_queue(m_eventQueue);
		m_eventQueue = nullptr;
	}

	if(m_xineStream) {
		xine_dispose(m_xineStream);
		m_xineStream = nullptr;
	}

	if(m_audioDriver) {
		xine_close_audio_driver(m_xineEngine, m_audioDriver);
		m_audioDriver = nullptr;
	}

	if(m_videoDriver) {
		xine_close_video_driver(m_xineEngine, m_videoDriver);
		m_videoDriver = nullptr;
	}

	if(m_xineEngine) {
		xine_exit(m_xineEngine);
		m_xineEngine = nullptr;
	}

	if(m_connection) {
		xcb_disconnect(m_connection);
		m_connection = nullptr;
	}

	m_state = STOPPED;
}

void
XinePlayerBackend::destSizeCallback(void *p, int /*video_width */, int /*video_height */, double /*video_aspect */, int *dest_width, int *dest_height, double *dest_aspect)
{
	if(p == nullptr)
		return;

	XinePlayerBackend *xinePlayer = (XinePlayerBackend *)p;
	*dest_width = xinePlayer->m_videoLayerGeometry.width();
	*dest_height = xinePlayer->m_videoLayerGeometry.height();
	*dest_aspect = 1.0;
}

void
XinePlayerBackend::frameOutputCallback(void *p, int /*video_width */, int /*video_height */, double /*video_aspect */, int *dest_x, int *dest_y, int *dest_width, int *dest_height, double *dest_aspect, int *win_x, int *win_y)
{
	if(p == nullptr)
		return;

	XinePlayerBackend *xinePlayer = (XinePlayerBackend *)p;

	*dest_x = 0;
	*dest_y = 0;
	*dest_width = xinePlayer->m_videoLayerGeometry.width();
	*dest_height = xinePlayer->m_videoLayerGeometry.height();
	*dest_aspect = 1.0;
	*win_x = xinePlayer->m_videoLayerGeometry.x();
	*win_y = xinePlayer->m_videoLayerGeometry.y();
}

void
XinePlayerBackend::audioMixerMethodChangedCallback(void *p, xine_cfg_entry_t *entry)
{
	if(p == nullptr)
		return;

	XinePlayerBackend *xinePlayer = (XinePlayerBackend *)p;
	xinePlayer->m_softwareMixer = (bool)entry->num_value;
}

void
XinePlayerBackend::setState(PlayState state)
{
	static VideoPlayer::State vpState[] = { VideoPlayer::Ready, VideoPlayer::Paused, VideoPlayer::Playing };
	if(m_state == state)
		return;
	m_state = state;
	emit stateChanged(vpState[state]);
}
