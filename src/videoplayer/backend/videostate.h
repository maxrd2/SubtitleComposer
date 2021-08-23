/*
    SPDX-FileCopyrightText: 2003 Fabrice Bellard
    SPDX-FileCopyrightText: 2020 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VIDEOSTATE_H
#define VIDEOSTATE_H

#include <cmath>

#include "videoplayer/backend/videodecoder.h"
#include "videoplayer/backend/audiodecoder.h"
#include "videoplayer/backend/subtitledecoder.h"
#include "videoplayer/backend/framequeue.h"
#include "videoplayer/backend/packetqueue.h"
#include "videoplayer/backend/streamdemuxer.h"
#include "videoplayer/backend/clock.h"

#include <QString>
#include <QWaitCondition>

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avfft.h"
}


#define MAX_VOLUME 1.0

#define MAX_QUEUE_SIZE (15 * 1024 * 1024)
#define MIN_FRAMES 25
#define EXTERNAL_CLOCK_MIN_FRAMES 2
#define EXTERNAL_CLOCK_MAX_FRAMES 10

// no AV sync correction is done if below the minimum AV sync threshold
#define AV_SYNC_THRESHOLD_MIN 0.04
// AV sync correction is done if above the maximum AV sync threshold
#define AV_SYNC_THRESHOLD_MAX 0.1
// If a frame duration is longer than this, it will not be duplicated to compensate AV sync
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1

// external clock speed adjustment constants for realtime sources based on buffer fullness
#define EXTERNAL_CLOCK_SPEED_MIN  0.900
#define EXTERNAL_CLOCK_SPEED_MAX  1.010
#define EXTERNAL_CLOCK_SPEED_STEP 0.001

// polls for possible required screen refresh at least this often, should be less than 1/fps
#define REFRESH_RATE 0.01

#define CURSOR_HIDE_DELAY 1000000

#define USE_ONEPASS_SUBTITLE_RENDER 1

// TODO: support audio and subtitle rendering
#undef AUDIO_VISUALIZATION
#undef VIDEO_SUBTITLE

namespace SubtitleComposer {
class RenderThread;
class GLRenderer;

enum {
	AV_SYNC_AUDIO_MASTER,
	AV_SYNC_VIDEO_MASTER,
	AV_SYNC_EXTERNAL_CLOCK
};

enum ShowMode {
	SHOW_MODE_NONE = -1,
	SHOW_MODE_VIDEO = 0,
#ifdef AUDIO_VISUALIZATION
	SHOW_MODE_WAVES,
	SHOW_MODE_RDFT,
#endif
	SHOW_MODE_NB
};

class VideoState {
	friend class FFPlayer;
	friend class RenderThread;
	friend class AudioDecoder;
	friend class VideoDecoder;
	friend class SubtitleDecoder;
	friend class PacketQueue;
	friend class FrameQueue;
	friend class StreamDemuxer;

private:
	VideoState();

	inline bool streamHasEnoughPackets(AVStream *st, int streamId, PacketQueue *q) {
		return streamId < 0
			|| q->abortRequested()
			|| (st->disposition & AV_DISPOSITION_ATTACHED_PIC)
			|| (q->nbPackets() > MIN_FRAMES && (!q->duration() || av_q2d(st->time_base) * q->duration() > 1.0));
	}

	inline bool streamsHaveEnoughPackets() {
		return audPQ.size() + vidPQ.size() + subPQ.size() > MAX_QUEUE_SIZE
			|| (streamHasEnoughPackets(audStream, audStreamIdx, &audPQ)
				&& streamHasEnoughPackets(vidStream, vidStreamIdx, &vidPQ)
				&& streamHasEnoughPackets(subStream, subStreamIdx, &subPQ));
	}

	inline bool reachedEOF() {
		return (!audStream || (audDec.finished() == audPQ.serial()))
			&& (!vidStream || (vidDec.finished() == vidPQ.serial() && vidFQ.nbRemaining() == 0));
	}

	inline double position() {
		const double pos = masterTime();
		return std::isnan(pos) ? double(seekPos) / AV_TIME_BASE : pos;
	}

	int masterSyncType();
	Clock * masterClock();
	double masterTime();
	void checkExternalClockSpeed();

	void notifyLoaded();
	void notifySpeed();
	void notifyState();

private: // settings
	int seek_by_bytes = -1;
	int av_sync_type = AV_SYNC_AUDIO_MASTER;
	int64_t start_time = AV_NOPTS_VALUE;
	int fast = 0;
	int genpts = 0;
	int lowres = 0;
	int framedrop = -1;
	int infinite_buffer = -1;
	double rdftspeed = 0.02;
	int autorotate = 1;

private:
	bool abortRequested = false;
	bool paused = false;
	bool lastPaused = false;
	int step = 0;
	int readPauseReturn = 0;
	bool queueAttachmentsReq = false;
	bool seekReq = false;
	double seekDecoder = 0.;
	int seekFlags = 0;
	int64_t seekPos = 0;
	AVFormatContext *fmtContext = nullptr;
	bool realTime = false;

	FFPlayer *player = nullptr;
	StreamDemuxer *demuxer = nullptr;
	GLRenderer *glRenderer = nullptr;
	RenderThread *renderThread = nullptr;

	ShowMode showMode = SHOW_MODE_NONE;
	bool forceRefresh = true;

	Clock audClk;
	AudioDecoder audDec;
	int audStreamIdx = -1;
	AVStream *audStream = nullptr;
	PacketQueue audPQ;
	int frameDropsLate = 0;

#ifdef AUDIO_VISUALIZATION
	QVector<int16_t> sample_array;
	int sample_array_index = 0;
	int last_i_start = 0;
	RDFTContext *rdft = nullptr;
	int rdft_bits = 0;
	FFTSample *rdft_data = nullptr;
	double last_vis_time = 0.;
#endif

	Clock extClk;

	SubtitleDecoder subDec;
	int subStreamIdx = -1;
	AVStream *subStream = nullptr;
	PacketQueue subPQ;
	FrameQueue subFQ;
#ifdef VIDEO_SUBTITLE
	SwsContext *subConvertCtx = nullptr;
#endif

	Clock vidClk;
	VideoDecoder vidDec;
	double frameTimer = 0.;
	double frameLastReturnedTime = 0.;
	int vidStreamIdx = -1;
	AVStream *vidStream = nullptr;
	PacketQueue vidPQ;
	FrameQueue vidFQ;
	double maxFrameDuration = 0.; // maximum duration of a frame - above this, we consider the jump a timestamp discontinuity
	bool eof = false;

	QString filename;

	int lastVideoStream = -1;
	int lastAudioStream = -1;
	int lastSubtitleStream = -1;

	QWaitCondition *continueReadThread = nullptr;
};
}

#endif // VIDEOSTATE_H
