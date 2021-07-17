/*
 * Copyright (c) 2003 Fabrice Bellard
 * Copyright (c) 2020 Mladen Milinkovic <max@smoothware.net>
 *
 * Portions of this code are based on FFmpeg
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

#include "ffplayer.h"

#include <QDebug>
#include <QThread>
#include <QWidget>
#include <QEvent>
#include <QWaitCondition>
#include <QMutex>

#include <cinttypes>
#include <cmath>
#include <climits>
#include <csignal>
#include <cstdint>

#include "videoplayer/backend/decoder.h"
#include "videoplayer/backend/framequeue.h"
#include "videoplayer/backend/glrenderer.h"
#include "videoplayer/backend/packetqueue.h"
#include "videoplayer/backend/renderthread.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

using namespace SubtitleComposer;

FFPlayer::FFPlayer(QObject *parent)
	: QObject(parent),
	  m_muted(false),
	  m_volume(1.0),
	  m_vs(nullptr),
	  m_renderer(new GLRenderer(nullptr))
{
	connect(m_renderer, &QObject::destroyed, this, [&](){
		close();
		m_renderer = nullptr;
	});

	qRegisterMetaType<FFPlayer::State>("FFPlayer::State");
	connect(&m_positionTimer, &QTimer::timeout, this, [this](){
		const double pf = position();
		const qint32 p = pf * 1000.;
		if(m_postitionLast != p) {
			m_postitionLast = p;
			emit positionChanged(pf);
		}
	});

	av_log_set_flags(AV_LOG_SKIP_REPEATED);
#ifndef NDEBUG
	av_log_set_level(AV_LOG_VERBOSE);
#else
	av_log_set_level(AV_LOG_INFO);
#endif
#ifdef FFMPEG_QT_LOGGING
	av_log_set_callback(av_log_callback);
#endif // FFMPEG_QT_LOGGING

	// register all codecs, demux and protocols
	avformat_network_init();
}

FFPlayer::~FFPlayer()
{
	if(m_renderer)
		m_renderer->deleteLater();

	close();
	avformat_network_deinit();
	av_log(nullptr, AV_LOG_QUIET, "%s", "");
}

uint8_t *
FFPlayer::flushPkt()
{
	static uint8_t pktData;
	return &pktData;
}

void
FFPlayer::pauseToggle()
{
	m_vs->demuxer->pauseToggle();
	m_vs->step = 0;
	m_vs->notifyState();
}

bool
FFPlayer::paused()
{
	return m_vs->paused || m_vs->step;
}

void
FFPlayer::seek(double seconds)
{
	m_vs->demuxer->seek(seconds * double(AV_TIME_BASE));
}

void
FFPlayer::stepFrame(int frameCnt)
{
	const AVStream *st = m_vs->vidStream;
	if(!st)
		return;

	if(frameCnt >= 0) {
		while(frameCnt--)
			m_vs->demuxer->stepFrame();
	} else {
		if(!m_vs->paused)
			m_vs->demuxer->pauseToggle();

		// TODO: FIXME: backward stepping is broken
		double seek_seconds = m_vs->vidClk.pts(); // maxrd2: was m_vs->extclk.pts
		if(std::isnan(seek_seconds))
			return; // maxrd2: was seek_seconds = m_vs->extclk.pts;
		seek_seconds += double(frameCnt - 1) / av_q2d(st->r_frame_rate);
		seek(seek_seconds);

		m_vs->forceRefresh = true;
		m_vs->demuxer->stepFrame();
	}
	m_vs->notifyState();
}

//#define FFMPEG_QT_LOGGING
#ifdef FFMPEG_QT_LOGGING
static void
av_log_callback(void *ptr, int severity, const char *format, va_list args)
{
	if(severity > av_log_get_level())
		return;
	static QMap<int, QtMsgType> type {
		{ AV_LOG_TRACE, QtDebugMsg },
		{ AV_LOG_DEBUG, QtDebugMsg },
		{ AV_LOG_VERBOSE, QtInfoMsg },
		{ AV_LOG_INFO, QtInfoMsg },
		{ AV_LOG_WARNING, QtWarningMsg },
		{ AV_LOG_ERROR, QtCriticalMsg },
		{ AV_LOG_FATAL, QtFatalMsg },
		{ AV_LOG_PANIC, QtFatalMsg },
	};
	if(!type.contains(severity))
		return;
	static char buf[8192];
	static int bufpos = 0;
	static int bufprt = 0;
	static int pfx = 1;
	if(pfx) {
		// flush
		if(bufprt < bufpos)
			qt_message_output(type[severity], QMessageLogContext(), QString::fromUtf8(buf + bufprt));
		bufpos = bufprt = 0;
	}
	int ls = sizeof(buf) - bufpos;
	const int ll = av_log_format_line2(ptr, severity, format, args, buf + bufpos, ls, &pfx);
	if(ll <= 0)
		return;
	if(ll < ls)
		ls = ll;
	bufpos += ls - 1;
	int bufend = bufprt;
	for(;;) {
		while(bufend <= bufpos && buf[bufend] != '\n' && buf[bufend] != '\r' && buf[bufend])
			bufend++;
		if(bufend > bufpos || (buf[bufend] != '\n' && buf[bufend] != '\r'))
			break;
		buf[bufend++] = 0;
		qt_message_output(type[severity], QMessageLogContext(), QString::fromUtf8(buf + bufprt));
		bufprt = bufend;
	}
	if(bufprt == bufpos)
		bufpos = bufprt = 0;
}
#endif // FFMPEG_QT_LOGGING

bool
FFPlayer::open(const char *filename)
{
	close();

	m_vs = StreamDemuxer::open(filename);
	if(!m_vs) {
		av_log(nullptr, AV_LOG_FATAL, "Failed to initialize VideoState!\n");
		close();
		return false;
	}
	m_vs->player = this;
	m_vs->glRenderer = m_renderer;

	// start event loop
	m_vs->renderThread = new RenderThread(m_vs);
	m_vs->renderThread->start();

	m_postitionLast = -1;
	m_positionTimer.start(100);

	return true;
}

void
FFPlayer::close()
{
	if(m_vs) {
		m_positionTimer.stop();

		if(m_vs->renderThread) {
			m_vs->renderThread->requestInterruption();
			m_vs->renderThread->wait();
			delete m_vs->renderThread;
			m_vs->renderThread = nullptr;
		}
		StreamDemuxer::close(m_vs);
		m_vs = nullptr;
	}
}

quint32
FFPlayer::videoWidth()
{
	return m_vs->vidStream->codecpar->width;
}

quint32
FFPlayer::videoHeight()
{
	return m_vs->vidStream->codecpar->height;
}

qreal
FFPlayer::videoSAR()
{
	AVRational aspectRatio = av_guess_sample_aspect_ratio(m_vs->fmtContext, m_vs->vidStream, nullptr);
	if(av_cmp_q(aspectRatio, av_make_q(0, 1)) <= 0)
		aspectRatio = av_make_q(1, 1);

	const int picWidth = m_vs->vidStream->codecpar->width;
	const int picHeight = m_vs->vidStream->codecpar->height;
	aspectRatio = av_mul_q(aspectRatio, av_make_q(picWidth, picHeight));

	// XXX: we suppose the screen has a 1.0 pixel ratio
	return av_q2d(aspectRatio);
}

qreal
FFPlayer::videoFPS()
{
	if(!m_vs->vidStream)
		return 0.;
	return av_q2d(m_vs->vidStream->r_frame_rate);
}

int
FFPlayer::activeVideoStream()
{
	int idx = 0;
	for(int i = 0; i < int(m_vs->fmtContext->nb_streams); i++) {
		const AVStream *stream = m_vs->fmtContext->streams[i];
		if(stream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO)
			continue;
		if(i == m_vs->vidStreamIdx)
			return idx;
		idx++;
	}
	return -1;
}

int
FFPlayer::activeAudioStream()
{
	if(m_vs->audStreamIdx < 0)
		return -1;
	return m_vs->demuxer->relativeStreamIndex(AVMEDIA_TYPE_AUDIO, m_vs->audStreamIdx);
}

void
FFPlayer::activeAudioStream(int streamIndex)
{
	streamIndex = streamIndex < 0 ? -1 : m_vs->demuxer->absoluteStreamIndex(AVMEDIA_TYPE_AUDIO, streamIndex);
	m_vs->demuxer->selectStream(AVMEDIA_TYPE_AUDIO, streamIndex);
}

int
FFPlayer::activeSubtitleStream()
{
	int idx = 0;
	for(int i = 0; i < int(m_vs->fmtContext->nb_streams); i++) {
		const AVStream *st = m_vs->fmtContext->streams[i];
		if(st->codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE)
			continue;
		const AVCodecDescriptor *desc = avcodec_descriptor_get(st->codecpar->codec_id);
		if(!desc || !(desc->props & AV_CODEC_PROP_TEXT_SUB))
			continue;
		if(i == m_vs->subStreamIdx)
			return idx;
		idx++;
	}
	return -1;
}

void
FFPlayer::setMuted(bool mute)
{
	if(m_muted == mute)
		return;
	m_muted = mute;

	if(m_vs)
		m_vs->audDec.setListenerGain(m_muted ? 0. : m_volume);
}

void
FFPlayer::setVolume(double volume)
{
	volume = qMax(0., volume);
	if(m_volume == volume)
		return;
	m_volume = volume;

	if(m_vs)
		m_vs->audDec.setListenerGain(m_volume);
}

void
FFPlayer::setSpeed(double speed)
{
	m_vs->audDec.setPitch(speed);
}
