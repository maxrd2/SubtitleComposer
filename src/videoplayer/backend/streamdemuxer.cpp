/*
    SPDX-FileCopyrightText: 2003 Fabrice Bellard
    SPDX-FileCopyrightText: 2020 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "streamdemuxer.h"

#include <QMutex>
#include <QWaitCondition>

#include "videoplayer/backend/ffplayer.h"
#include "videoplayer/backend/packetqueue.h"
#include "videoplayer/backend/videostate.h"
#include "videoplayer/videoplayer.h"

extern "C" {
#include "libavutil/time.h"
#include "libavformat/avformat.h"
}


using namespace SubtitleComposer;

static void
print_error(const char *filename, int err)
{
	char errbuf[128];
	const char *errbuf_ptr = errbuf;

	if(av_strerror(err, errbuf, sizeof(errbuf)) < 0)
		errbuf_ptr = strerror(AVUNERROR(err));
	av_log(nullptr, AV_LOG_ERROR, "%s: %s\n", filename, errbuf_ptr);
}


static bool
isRealTime(AVFormatContext *s)
{
	if(!strcmp(s->iformat->name, "rtp") || !strcmp(s->iformat->name, "rtsp") || !strcmp(s->iformat->name, "sdp"))
		return 1;
#if LIBAVFORMAT_VERSION_MAJOR < 58
	const char *url = s->filename;
#else
	const char *url = s->url;
#endif
	if(s->pb && (!strncmp(url, "rtp:", 4) || !strncmp(url, "udp:", 4)))
		return 1;
	return 0;
}

StreamDemuxer::StreamDemuxer(VideoState *vs, QObject *parent)
	: QThread(parent),
	  m_vs(vs)
{
}

VideoState *
StreamDemuxer::open(const char *filename)
{
	VideoState *vs = new VideoState();
	if(!vs)
		return nullptr;
	vs->lastVideoStream = vs->vidStreamIdx = -1;
	vs->lastAudioStream = vs->audStreamIdx = -1;
	vs->lastSubtitleStream = vs->subStreamIdx = -1;
	vs->filename = filename;

	if(vs->vidFQ.init(&vs->vidPQ, VIDEO_PICTURE_QUEUE_SIZE, 1) < 0)
		goto fail;
	if(vs->subFQ.init(&vs->subPQ, SUBPICTURE_QUEUE_SIZE, 0) < 0)
		goto fail;
	if(vs->vidPQ.init() < 0 || vs->audPQ.init() < 0 || vs->subPQ.init() < 0)
		goto fail;

	vs->continueReadThread = new QWaitCondition();

	vs->vidClk.init(&vs->vidPQ);
	vs->audClk.init(&vs->audPQ);
	vs->extClk.init(nullptr);

	vs->demuxer = new StreamDemuxer(vs);
	vs->demuxer->start();
	return vs;

fail:
	close(vs);
	return nullptr;
}

void
StreamDemuxer::close(VideoState *vs)
{
	// XXX: use a special url_shutdown call to abort parse cleanly
	vs->abortRequested = true;
	vs->demuxer->wait();

	// close each stream
	if(vs->audStreamIdx >= 0)
		vs->demuxer->componentClose(vs->audStreamIdx);
	if(vs->vidStreamIdx >= 0)
		vs->demuxer->componentClose(vs->vidStreamIdx);
	if(vs->subStreamIdx >= 0)
		vs->demuxer->componentClose(vs->subStreamIdx);

	delete vs->demuxer;

	avformat_close_input(&vs->fmtContext);

	vs->vidPQ.destroy();
	vs->audPQ.destroy();
	vs->subPQ.destroy();

	vs->vidFQ.destory();
	vs->subFQ.destory();
	delete vs->continueReadThread;
#ifdef VIDEO_SUBTITLE
	sws_freeContext(vs->subConvertCtx);
#endif
	delete vs;
}

void
StreamDemuxer::componentClose(int streamIndex)
{
	AVFormatContext *ic = m_vs->fmtContext;
	AVCodecParameters *codecPar;

	if(streamIndex < 0 || streamIndex >= (int)ic->nb_streams)
		return;
	codecPar = ic->streams[streamIndex]->codecpar;

	switch(codecPar->codec_type) {
	case AVMEDIA_TYPE_AUDIO:
		m_vs->audDec.abort();
		m_vs->audDec.destroy();

#ifdef AUDIO_VISUALIZATION
		if(m_vs->rdft) {
			av_rdft_end(m_vs->rdft);
			av_freep(&m_vs->rdft_data);
			m_vs->rdft = nullptr;
			m_vs->rdft_bits = 0;
		}
#endif
		break;
	case AVMEDIA_TYPE_VIDEO:
		m_vs->vidDec.abort();
		m_vs->vidDec.destroy();
		break;
	case AVMEDIA_TYPE_SUBTITLE:
		m_vs->subDec.abort();
		m_vs->subDec.destroy();
		break;
	default:
		break;
	}

	ic->streams[streamIndex]->discard = AVDISCARD_ALL;
	switch(codecPar->codec_type) {
	case AVMEDIA_TYPE_AUDIO:
		m_vs->audStream = nullptr;
		m_vs->audStreamIdx = -1;
		break;
	case AVMEDIA_TYPE_VIDEO:
		m_vs->vidStream = nullptr;
		m_vs->vidStreamIdx = -1;
		break;
	case AVMEDIA_TYPE_SUBTITLE:
		m_vs->subStream = nullptr;
		m_vs->subStreamIdx = -1;
		break;
	default:
		break;
	}
}

void
StreamDemuxer::pauseToggle()
{
	if(m_vs->paused) {
		m_vs->frameTimer += av_gettime_relative() / double(AV_TIME_BASE) - m_vs->vidClk.lastUpdated();
		if(m_vs->readPauseReturn != AVERROR(ENOSYS))
			m_vs->vidClk.pause(0);
		m_vs->vidClk.set(m_vs->vidClk.get(), m_vs->vidClk.serial());
		m_vs->audDec.play();
	} else {
		m_vs->audDec.pause();
	}
	m_vs->extClk.set(m_vs->extClk.get(), m_vs->extClk.serial());
	m_vs->paused = !m_vs->paused;
	m_vs->audClk.pause(m_vs->paused);
	m_vs->vidClk.pause(m_vs->paused);
	m_vs->extClk.pause(m_vs->paused);
}

void
StreamDemuxer::seek(qint64 time)
{
	m_vs->seekFlags &= ~AVSEEK_FLAG_BYTE;
	if(m_vs->seek_by_bytes) {
		m_vs->seekFlags |= AVSEEK_FLAG_BYTE;
		m_vs->seekPos = double(time) * (m_vs->fmtContext->bit_rate ? double(m_vs->fmtContext->bit_rate) / 8. : 180000.);
	} else {
		m_vs->seekPos = time;
	}
	m_vs->seekReq = true;
	m_vs->audDec.flush();
	m_vs->continueReadThread->wakeOne();
}

void
StreamDemuxer::stepFrame()
{
	if(m_vs->paused)
		pauseToggle();
	m_vs->step = 1;
}

bool
StreamDemuxer::abortRequested()
{
	return m_vs->abortRequested;
}

// open a given stream. Return 0 if OK
int
StreamDemuxer::componentOpen(int streamIndex)
{
	AVFormatContext *ic = m_vs->fmtContext;
	AVCodecContext *avCtx;
	const AVCodec *codec;
	AVDictionary *opts = nullptr;
	AVDictionaryEntry *t = nullptr;
	int sampleRate, nbChannels;
	int64_t channelLayout;
	int ret = 0;
	int stream_lowres = m_vs->lowres;

	if(streamIndex < 0 || streamIndex >= (int)ic->nb_streams)
		return -1;

	avCtx = avcodec_alloc_context3(nullptr);
	if(!avCtx)
		return AVERROR(ENOMEM);

	ret = avcodec_parameters_to_context(avCtx, ic->streams[streamIndex]->codecpar);
	if(ret < 0)
		goto fail;
	avCtx->pkt_timebase = ic->streams[streamIndex]->time_base;

	codec = avcodec_find_decoder(avCtx->codec_id);

	switch(avCtx->codec_type) {
	case AVMEDIA_TYPE_AUDIO   :
		m_vs->lastAudioStream = streamIndex;
		break;
	case AVMEDIA_TYPE_SUBTITLE:
		m_vs->lastSubtitleStream = streamIndex;
		break;
	case AVMEDIA_TYPE_VIDEO   :
		m_vs->lastVideoStream = streamIndex;
		break;
	default:
		break;
	}
	if(!codec) {
		av_log(nullptr, AV_LOG_WARNING, "No decoder could be found for codec %s\n", avcodec_get_name(avCtx->codec_id));
		ret = AVERROR(EINVAL);
		goto fail;
	}

	avCtx->codec_id = codec->id;
	if(stream_lowres > codec->max_lowres) {
		av_log(avCtx, AV_LOG_WARNING, "The maximum value for lowres supported by the decoder is %d\n",
			   codec->max_lowres);
		stream_lowres = codec->max_lowres;
	}
	avCtx->lowres = stream_lowres;

	if(m_vs->fast)
		avCtx->flags2 |= AV_CODEC_FLAG2_FAST;

	if(!av_dict_get(opts, "threads", nullptr, 0))
		av_dict_set(&opts, "threads", "auto", 0);
	if(stream_lowres)
		av_dict_set_int(&opts, "lowres", stream_lowres, 0);
	if((ret = avcodec_open2(avCtx, codec, &opts)) < 0) {
		char e[AV_ERROR_MAX_STRING_SIZE];
		av_log(nullptr, AV_LOG_ERROR, "Failed opening codec err:%d - %s.\n", ret, av_make_error_string(e, sizeof(e), ret));
		goto fail;
	}
	if((t = av_dict_get(opts, "", nullptr, AV_DICT_IGNORE_SUFFIX))) {
		av_log(nullptr, AV_LOG_ERROR, "Option %s not found.\n", t->key);
		ret = AVERROR_OPTION_NOT_FOUND;
		goto fail;
	}

	m_vs->eof = false;
	ic->streams[streamIndex]->discard = AVDISCARD_DEFAULT;
	switch(avCtx->codec_type) {
	case AVMEDIA_TYPE_AUDIO:
		sampleRate = avCtx->sample_rate;
		nbChannels = avCtx->channels;
		channelLayout = avCtx->channel_layout;

		// prepare audio output
		if(!m_vs->audDec.open(channelLayout, nbChannels, sampleRate))
			goto fail;

		m_vs->audStreamIdx = streamIndex;
		m_vs->audStream = ic->streams[streamIndex];

		m_vs->audDec.init(avCtx, &m_vs->audPQ, nullptr, m_vs->continueReadThread);
		if((m_vs->fmtContext->iformat->flags & (AVFMT_NOBINSEARCH | AVFMT_NOGENSEARCH | AVFMT_NO_BYTE_SEEK)) &&
		   !m_vs->fmtContext->iformat->read_seek) {
			m_vs->audDec.startPts(m_vs->audStream->start_time, m_vs->audStream->time_base);
		}
		m_vs->audDec.start();
		m_vs->audDec.pause();
		break;
	case AVMEDIA_TYPE_VIDEO:
		m_vs->vidStreamIdx = streamIndex;
		m_vs->vidStream = ic->streams[streamIndex];

		m_vs->vidDec.init(avCtx, &m_vs->vidPQ, &m_vs->vidFQ, m_vs->continueReadThread);
		m_vs->vidDec.start();
		m_vs->queueAttachmentsReq = true;
		break;
	case AVMEDIA_TYPE_SUBTITLE:
		m_vs->subStreamIdx = streamIndex;
		m_vs->subStream = ic->streams[streamIndex];

		m_vs->subDec.init(avCtx, &m_vs->subPQ, &m_vs->subFQ, m_vs->continueReadThread);
		m_vs->subDec.start();
		break;
	default:
		break;
	}
	goto out;

fail:
	avcodec_free_context(&avCtx);
out:
	av_dict_free(&opts);

	return ret;
}

void
StreamDemuxer::cycleStream(int codecType)
{
	int startIndex, oldIndex;
	if(codecType == AVMEDIA_TYPE_VIDEO) {
		startIndex = m_vs->lastVideoStream;
		oldIndex = m_vs->vidStreamIdx;
	} else if(codecType == AVMEDIA_TYPE_AUDIO) {
		startIndex = m_vs->lastAudioStream;
		oldIndex = m_vs->audStreamIdx;
	} else {
		startIndex = m_vs->lastSubtitleStream;
		oldIndex = m_vs->subStreamIdx;
	}
	int streamIndex = startIndex;

	AVProgram *p = nullptr;
	AVFormatContext *ic = m_vs->fmtContext;
	int nbStreams = m_vs->fmtContext->nb_streams;
	if(codecType != AVMEDIA_TYPE_VIDEO && m_vs->vidStreamIdx != -1) {
		p = av_find_program_from_stream(ic, nullptr, m_vs->vidStreamIdx);
		if(p) {
			nbStreams = p->nb_stream_indexes;
			for(startIndex = 0; startIndex < nbStreams; startIndex++)
				if((int)p->stream_index[startIndex] == streamIndex)
					break;
			if(startIndex == nbStreams)
				startIndex = -1;
			streamIndex = startIndex;
		}
	}

	for(;;) {
		if(++streamIndex >= nbStreams) {
			if(codecType == AVMEDIA_TYPE_SUBTITLE) {
				streamIndex = -1;
				m_vs->lastSubtitleStream = -1;
				goto the_end;
			}
			if(startIndex == -1)
				return;
			streamIndex = 0;
		}
		if(streamIndex == startIndex)
			return;
		AVStream *st = m_vs->fmtContext->streams[p ? p->stream_index[streamIndex] : streamIndex];
		if(st->codecpar->codec_type == codecType) {
			/* check that parameters are OK */
			switch(codecType) {
			case AVMEDIA_TYPE_AUDIO:
				if(st->codecpar->sample_rate != 0 &&
				   st->codecpar->channels != 0)
					goto the_end;
				break;
			case AVMEDIA_TYPE_VIDEO:
			case AVMEDIA_TYPE_SUBTITLE:
				goto the_end;
			default:
				break;
			}
		}
	}
the_end:
	if(p && streamIndex != -1)
		streamIndex = p->stream_index[streamIndex];
	av_log(nullptr, AV_LOG_INFO, "Switch %s stream from #%d to #%d\n",
		   av_get_media_type_string((AVMediaType)codecType),
		   oldIndex, streamIndex);

	componentClose(oldIndex);
	componentOpen(streamIndex);
}

int
StreamDemuxer::relativeStreamIndex(int codecType, int absoluteIndex)
{
	int idx = 0;
	for(int i = 0; i < int(m_vs->fmtContext->nb_streams); i++) {
		if(m_vs->fmtContext->streams[i]->codecpar->codec_type != codecType)
			continue;
		if(i == absoluteIndex)
			return idx;
		idx++;
	}
	return -1;
}

int
StreamDemuxer::absoluteStreamIndex(int codecType, int relativeIndex)
{
	int idx = 0;
	for(int i = 0; i < int(m_vs->fmtContext->nb_streams); i++) {
		if(m_vs->fmtContext->streams[i]->codecpar->codec_type != codecType)
			continue;
		if(idx == relativeIndex)
			return i;
		idx++;
	}
	return -1;
}

void
StreamDemuxer::selectStream(int codecType, int streamIndex)
{
	int oldIndex;
	if(codecType == AVMEDIA_TYPE_VIDEO)
		oldIndex = m_vs->vidStreamIdx;
	else if(codecType == AVMEDIA_TYPE_AUDIO)
		oldIndex = m_vs->audStreamIdx;
	else
		oldIndex = m_vs->subStreamIdx;

	if(streamIndex < 0)
		streamIndex = -1;

	av_log(nullptr, AV_LOG_INFO, "Switch %s stream from #%d to #%d\n",
		   av_get_media_type_string((AVMediaType)codecType), oldIndex, streamIndex);

	componentClose(oldIndex);
	componentOpen(streamIndex);
}

void
StreamDemuxer::run()
{
	AVFormatContext *ic = nullptr;
	int err, i;
	int strIndex[AVMEDIA_TYPE_NB];
	QMutex wait_mutex;
	AVPacket *pkt = nullptr;

	memset(strIndex, -1, sizeof(strIndex));
	m_vs->eof = false;

	ic = avformat_alloc_context();
	if(!ic) {
		av_log(nullptr, AV_LOG_FATAL, "Could not allocate context.\n");
		goto cleanup;
	}
	ic->interrupt_callback.opaque = m_vs;
	ic->interrupt_callback.callback = [](void *ctx)->int {
		VideoState *is = (VideoState *)ctx;
		return is->abortRequested;
	};
	err = avformat_open_input(&ic, m_vs->filename.toUtf8(), nullptr, nullptr/*&format_opts*/);
	if(err < 0) {
		print_error(m_vs->filename.toUtf8(), err);
		goto cleanup;
	}
	m_vs->fmtContext = ic;

	if(m_vs->genpts)
		ic->flags |= AVFMT_FLAG_GENPTS;

	av_format_inject_global_side_data(ic);

	{ // find_stream_info
		const int origNbStreams = ic->nb_streams;
		AVDictionary **opts = (AVDictionary **)av_mallocz_array(origNbStreams, sizeof(*opts));
		if(!opts) {
			av_log(nullptr, AV_LOG_ERROR, "Could not alloc memory for stream options.\n");
			goto cleanup;
		}

		err = avformat_find_stream_info(ic, opts);

		for(i = 0; i < origNbStreams; i++)
			av_dict_free(&opts[i]);
		av_freep(&opts);

		if(err < 0) {
			av_log(nullptr, AV_LOG_WARNING, "%s: could not find codec parameters\n", m_vs->filename.toUtf8().data());
			goto cleanup;
		}
	}

	if(ic->pb)
		ic->pb->eof_reached = 0; // FIXME hack, ffplay maybe should not use avio_feof() to test for the end

	if(m_vs->seek_by_bytes < 0)
		m_vs->seek_by_bytes = !!(ic->iformat->flags & AVFMT_TS_DISCONT) && strcmp("ogg", ic->iformat->name);

	m_vs->maxFrameDuration = (ic->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0;

	// seeking requested, execute it
	if(m_vs->start_time != AV_NOPTS_VALUE) {
		int64_t timestamp = m_vs->start_time;
		// add the stream start time
		if(ic->start_time != AV_NOPTS_VALUE)
			timestamp += ic->start_time;
		if(avformat_seek_file(ic, -1, INT64_MIN, timestamp, INT64_MAX, 0) < 0)
			av_log(nullptr, AV_LOG_WARNING, "%s: could not seek to position %0.3f\n", m_vs->filename.toUtf8().data(), double(timestamp) / AV_TIME_BASE);
	}

	m_vs->realTime = isRealTime(ic);

	av_dump_format(ic, 0, m_vs->filename.toUtf8(), 0);

	for(i = 0; i < (int)ic->nb_streams; i++)
		ic->streams[i]->discard = AVDISCARD_ALL;

	strIndex[AVMEDIA_TYPE_VIDEO] = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, strIndex[AVMEDIA_TYPE_VIDEO], -1, nullptr, 0);
	strIndex[AVMEDIA_TYPE_AUDIO] = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, strIndex[AVMEDIA_TYPE_AUDIO], strIndex[AVMEDIA_TYPE_VIDEO], nullptr, 0);

	// open the streams
	if(strIndex[AVMEDIA_TYPE_AUDIO] >= 0)
		componentOpen(strIndex[AVMEDIA_TYPE_AUDIO]);

	{
		const int vidMode = strIndex[AVMEDIA_TYPE_VIDEO] >= 0 ? componentOpen(strIndex[AVMEDIA_TYPE_VIDEO]) : -1;
		m_vs->showMode = vidMode >= 0 ? SHOW_MODE_VIDEO :
#ifdef AUDIO_VISUALIZATION
								SHOW_MODE_RDFT
#else
								SHOW_MODE_NB
#endif
								;
	}

	if(strIndex[AVMEDIA_TYPE_SUBTITLE] >= 0)
		componentOpen(strIndex[AVMEDIA_TYPE_SUBTITLE]);

	if(m_vs->vidStreamIdx < 0 && m_vs->audStreamIdx < 0) {
		av_log(nullptr, AV_LOG_FATAL, "Failed to open file '%s'\n", m_vs->filename.toUtf8().data());
		goto cleanup;
	}

	if(m_vs->infinite_buffer < 0 && m_vs->realTime)
		m_vs->infinite_buffer = 1;

	m_vs->notifyLoaded();

	for(;;) {
		if(m_vs->abortRequested)
			break;
		if(m_vs->paused != m_vs->lastPaused) {
			m_vs->lastPaused = m_vs->paused;
			if(m_vs->paused)
				m_vs->readPauseReturn = av_read_pause(ic);
			else
				av_read_play(ic);
		}
		if(m_vs->seekReq) {
			const int64_t seekTarget = m_vs->seekPos;
			// seeks are inaccurate so seek to previous keyframe and then retrive/decode frames until is->seek_pos
			m_vs->seekDecoder = seekTarget / double(AV_TIME_BASE);
			if(av_seek_frame(m_vs->fmtContext, -1, seekTarget, m_vs->seekFlags | AVSEEK_FLAG_BACKWARD) < 0) {
				m_vs->seekDecoder = 0.;
				av_log(nullptr, AV_LOG_ERROR, "%s: error while seeking\n",
#if LIBAVFORMAT_VERSION_MAJOR < 58
					   m_vs->fmtContext->filename
#else
					   m_vs->fmtContext->url
#endif
					   );
			} else {
				if(m_vs->audStreamIdx >= 0) {
					m_vs->audPQ.flush();
					m_vs->audPQ.putFlushPacket();
				}
				if(m_vs->subStreamIdx >= 0) {
					m_vs->subPQ.flush();
					m_vs->subPQ.putFlushPacket();
				}
				if(m_vs->vidStreamIdx >= 0) {
					m_vs->vidPQ.flush();
					m_vs->vidPQ.putFlushPacket();
				}
				if(m_vs->seekFlags & AVSEEK_FLAG_BYTE) {
					m_vs->extClk.set(NAN, 0);
				} else {
					m_vs->extClk.set(seekTarget / (double)AV_TIME_BASE, 0);
				}
				m_vs->vidClk.set(NAN, 0);
				m_vs->audClk.set(NAN, 0);
			}
			m_vs->seekReq = false;
			m_vs->queueAttachmentsReq = true;
			m_vs->eof = false;
			if(m_vs->paused)
				stepFrame();
		}
		if(m_vs->queueAttachmentsReq) {
			if(m_vs->vidStream && m_vs->vidStream->disposition & AV_DISPOSITION_ATTACHED_PIC) {
				AVPacket *copy = av_packet_alloc();
				if(av_packet_ref(copy, &m_vs->vidStream->attached_pic) < 0) {
					av_packet_free(&copy);
					goto cleanup;
				}
				m_vs->vidPQ.put(&copy);
				m_vs->vidPQ.putNullPacket(m_vs->vidStreamIdx);
			}
			m_vs->queueAttachmentsReq = false;
		}

		// wait 10 ms if queues are full
		if(m_vs->infinite_buffer < 1 && m_vs->streamsHaveEnoughPackets()) {
			wait_mutex.lock();
			m_vs->continueReadThread->wait(&wait_mutex, 10);
			wait_mutex.unlock();
			continue;
		}
		// pause when EOF reached
		if(!m_vs->paused && m_vs->reachedEOF()) {
			m_vs->step = 0;
			pauseToggle();
			m_vs->notifyState();
		}
		if(!pkt)
			pkt = av_packet_alloc();
		if(int ret = av_read_frame(ic, pkt) < 0) {
			if((ret == AVERROR_EOF || avio_feof(ic->pb)) && !m_vs->eof) {
				if(m_vs->vidStreamIdx >= 0)
					m_vs->vidPQ.putNullPacket(m_vs->vidStreamIdx);
				if(m_vs->audStreamIdx >= 0)
					m_vs->audPQ.putNullPacket(m_vs->audStreamIdx);
				if(m_vs->subStreamIdx >= 0)
					m_vs->subPQ.putNullPacket(m_vs->subStreamIdx);
				m_vs->eof = true;
			}
			if(ic->pb && ic->pb->error)
				break;
			wait_mutex.lock();
			m_vs->continueReadThread->wait(&wait_mutex, 10);
			wait_mutex.unlock();
			continue;
		} else {
			m_vs->eof = false;
		}
		if(pkt->stream_index == m_vs->audStreamIdx) {
			m_vs->audPQ.put(&pkt);
		} else if(pkt->stream_index == m_vs->vidStreamIdx && m_vs->vidStream && !(m_vs->vidStream->disposition & AV_DISPOSITION_ATTACHED_PIC)) {
			m_vs->vidPQ.put(&pkt);
		} else if(pkt->stream_index == m_vs->subStreamIdx) {
			m_vs->subPQ.put(&pkt);
		} else {
			av_packet_unref(pkt);
		}
	}
	m_vs->notifyState();

cleanup:
	if(ic && !m_vs->fmtContext)
		avformat_close_input(&ic);
}
