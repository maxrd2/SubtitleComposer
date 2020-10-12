/*
 * Copyright (c) 2003 Fabrice Bellard
 * Copyright (c) 2020 Mladen Milinkovic <max@smoothware.net>
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

#include "audiodecoder.h"

#include <QMap>

#include "videoplayer/backend/ffplayer.h"
#include "videoplayer/backend/framequeue.h"
#include "videoplayer/backend/videostate.h"
#include "videoplayer/backend/renderthread.h"

extern "C" {
#include "libavutil/time.h"
#include "libswresample/swresample.h"
}

#include <AL/al.h>
#include <AL/alc.h>

// maximum audio speed change to get correct sync
#define SAMPLE_CORRECTION_PERCENT_MAX 10

// we use about AUDIO_DIFF_AVG_NB A-V differences to make the average
#define AUDIO_DIFF_AVG_NB   20

// Minimum audio buffer size, in samples.
#define AUDIO_MIN_BUFFER_SIZE 512


using namespace SubtitleComposer;

AudioDecoder::AudioDecoder(VideoState *state, QObject *parent)
	: Decoder(parent),
	  m_vs(state),
	  m_swrCtx(nullptr),
	  m_audioBuf(nullptr),
	  m_bufSize(0),
	  m_audioBuf1(nullptr),
	  m_buf1Size(0),
	  m_alDev(nullptr),
	  m_alCtx(nullptr),
	  m_alSrc(0),
	  m_bufCnt(0),
	  m_bufFmt(0)
{
}

void
AudioDecoder::destroy()
{
	Decoder::destroy();
	swr_free(&m_swrCtx);

	av_freep(&m_audioBuf1);
	m_buf1Size = 0;
	m_audioBuf = nullptr;
}

void
AudioDecoder::abort()
{
	Decoder::abort();
	close();
}

void
AudioDecoder::play()
{
	alSourcePlay(m_alSrc);
}

void
AudioDecoder::pause()
{
	alSourcePause(m_alSrc);
}

void
AudioDecoder::setListenerGain(double gain)
{
	alListenerf(AL_GAIN, gain);
}

double
AudioDecoder::pitch() const
{
	ALfloat pitch = 1.0;
	alGetSourcef(m_alSrc, AL_PITCH, &pitch);
	return pitch;
}

void
AudioDecoder::setPitch(double pitch)
{
	alSourcef(m_alSrc, AL_PITCH, pitch);
	m_vs->notifySpeed();
}

void
AudioDecoder::flush()
{
	for(;;) {
		alSourceStop(m_alSrc);

		ALint bufferCnt = 0;
		alGetSourcei(m_alSrc, AL_BUFFERS_QUEUED, &bufferCnt);
		if(bufferCnt == 0)
			break;

		bufferCnt = 0;
		alGetSourcei(m_alSrc, AL_BUFFERS_PROCESSED, &bufferCnt);
		if(bufferCnt == 0) {
			av_log(nullptr, AV_LOG_WARNING, "openal: source didn't stop... retrying flush\n");
			av_usleep(10);
			continue;
		}

		ALuint *bufs = new ALuint[bufferCnt];
		alSourceUnqueueBuffers(m_alSrc, bufferCnt, bufs);
		alDeleteBuffers(bufferCnt, bufs);
		delete[] bufs;
		m_hwBufQueueSize = 0;
	}
}

void
AudioDecoder::close()
{
	flush();
	alcMakeContextCurrent(nullptr);
	if(m_alCtx) {
		alcDestroyContext(m_alCtx);
		m_alCtx = nullptr;
	}
	if(m_alDev) {
		alcCloseDevice(m_alDev);
		m_alDev = nullptr;
	}
	if(m_alSrc)
		alDeleteSources(1, &m_alSrc);
	m_alSrc = 0;
}

bool
AudioDecoder::open(int64_t wantChLayout, int wantNbChan, int wantSampleRate)
{
	const static QMap<int, const char *> bufFmtMap = {
		{ 4, "AL_FORMAT_QUAD16" },
		{ 6, "AL_FORMAT_51CHN16" },
		{ 7, "AL_FORMAT_61CHN16" },
		{ 8, "AL_FORMAT_71CHN16" },
	};

	int err;

	if(wantSampleRate <= 0 || wantNbChan <= 0) {
		av_log(nullptr, AV_LOG_ERROR, "openal: invalid sample rate or channel count!\n");
		return -1;
	}

	int availNbChan = wantNbChan;
	for(;;) {
		while(availNbChan > 2 && !bufFmtMap.contains(availNbChan))
			availNbChan--;
		if(availNbChan <= 2) {
			m_bufFmt = availNbChan == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
			break;
		}
		m_bufFmt = alGetEnumValue(bufFmtMap[wantNbChan]);
		if(m_bufFmt)
			break;
		availNbChan--;
	}

	if(!wantChLayout || wantNbChan != availNbChan || wantNbChan != av_get_channel_layout_nb_channels(wantChLayout)) {
		wantChLayout = av_get_default_channel_layout(availNbChan);
		wantChLayout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;
	}

	m_alDev = alcOpenDevice(nullptr);
	if(!m_alDev) {
		av_log(nullptr, AV_LOG_ERROR, "openal: error opening default audio device!\n");
		close();
		return false;
	}

	m_alCtx = alcCreateContext(m_alDev, nullptr);
	if(!m_alCtx) {
		av_log(nullptr, AV_LOG_ERROR, "openal: error creating audio context!\n");
		close();
		return false;
	}
	if(!alcMakeContextCurrent(m_alCtx)) {
		av_log(nullptr, AV_LOG_ERROR, "openal: error setting current audio context!\n");
		close();
		return false;
	}

	alGetError(); // clear error

	alGenSources(1, &m_alSrc);
	if((err = alGetError()) != AL_NO_ERROR) {
		av_log(nullptr, AV_LOG_ERROR, "openal: error generating audio source: %d\n", err);
		close();
		return false;
	}

	m_fmtTgt.fmt = AV_SAMPLE_FMT_S16;
	m_fmtTgt.freq = wantSampleRate;
	m_fmtTgt.channelLayout = wantChLayout;
	m_fmtTgt.channels = availNbChan;
	m_fmtTgt.frameSize = av_samples_get_buffer_size(nullptr, m_fmtTgt.channels, 1, m_fmtTgt.fmt, 1);
	m_fmtTgt.bytesPerSec = av_samples_get_buffer_size(nullptr, m_fmtTgt.channels, m_fmtTgt.freq, m_fmtTgt.fmt, 1);
	if(m_fmtTgt.bytesPerSec <= 0 || m_fmtTgt.frameSize <= 0) {
		av_log(nullptr, AV_LOG_ERROR, "av_samples_get_buffer_size failed\n");
		close();
		return false;
	}

	alListenerf(AL_GAIN, m_vs->player->muted() ? 0. : m_vs->player->volume());

	m_fmtSrc = m_fmtTgt;
	m_hwBufQueueSize = 0;
	m_bufSize = 0;

	// init averaging filter
	m_diffAvgCoef = exp(log(0.01) / AUDIO_DIFF_AVG_NB);
	m_diffAvgCount = 0;

	return true;
}

void
AudioDecoder::queueBuffer(uint8_t *data, int len)
{
	int err = alGetError(); // reset error
	ALuint buf;

	// get next buffer to fill - maybe use alGetSourcei(AL_BUFFERS_QUEUED / AL_BUFFERS_PROCESSED)
	alSourceUnqueueBuffers(m_alSrc, 1, &buf);
	if((err = alGetError()) != AL_NO_ERROR) {
		if(err == AL_INVALID_VALUE) {
			// buffer can not be unqueued because it has not been processed yet.
			alGenBuffers(1, &buf);
			if((err = alGetError()) != AL_NO_ERROR) {
				av_log(nullptr, AV_LOG_ERROR, "openal: alGenBuffers() failed: %d\n", err);
				return;
			} else {
				m_bufCnt++;
			}
		} else {
			av_log(nullptr, AV_LOG_ERROR, "openal: alSourceUnqueueBuffers() failed: %d\n", err);
			return;
		}
	} else {
		ALint hwBufSize;
		alGetBufferi(buf, AL_SIZE, &hwBufSize);
		if((err = alGetError()) != AL_NO_ERROR) {
			av_log(nullptr, AV_LOG_ERROR, "openal: alGetBufferi(AL_SIZE) failed: %d\n", err);
			return;
		}
		m_hwBufQueueSize -= hwBufSize;
	}

	// copy data to buffer
	alBufferData(buf, m_bufFmt, data, len, m_fmtTgt.freq);
	if((err = alGetError()) != AL_NO_ERROR) {
		av_log(nullptr, AV_LOG_ERROR, "openal: alBufferData() failed: %d\n", err);
		return;
	}

	// queue buffer
	alSourceQueueBuffers(m_alSrc, 1, &buf); // stream
	if((err = alGetError()) != AL_NO_ERROR) {
		av_log(nullptr, AV_LOG_ERROR, "openal: alSourceQueueBuffers() failed: %d\n", err);
		return;
	}

	m_hwBufQueueSize += len;

	// get current state
	ALint state = AL_INITIAL;
	alGetSourcei(m_alSrc, AL_SOURCE_STATE, &state);
	if((err = alGetError()) != AL_NO_ERROR) {
		av_log(nullptr, AV_LOG_ERROR, "openal: alGetSourcei(AL_SOURCE_STATE) failed: %d\n", err);
		return;
	}

	// start playing
	if(state != AL_PLAYING)
		alSourcePlay(m_alSrc);
}

/* return the wanted number of samples to get better sync if sync_type is video
 * or external master clock */
int
AudioDecoder::syncAudio(int nbSamples)
{
	int wantedNbSamples = nbSamples;

	// if not master, then we try to remove or add samples to correct the clock
	if(m_vs->masterSyncType() != AV_SYNC_AUDIO_MASTER) {
		int minNbSamples, maxNbSamples;

		double diff = m_vs->audClk.get() - m_vs->masterTime();

		if(!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD) {
			m_diffCum = diff + m_diffAvgCoef * m_diffCum;
			if(m_diffAvgCount < AUDIO_DIFF_AVG_NB) {
				// not enough measures to have a correct estimate
				m_diffAvgCount++;
			} else {
				// estimate the A-V difference
				const double avgDiff = m_diffCum * (1.0 - m_diffAvgCoef);
				if(avgDiff != 0.) {
					wantedNbSamples = nbSamples + (int)(diff * m_fmtSrc.freq);
					minNbSamples = ((nbSamples * (100 - SAMPLE_CORRECTION_PERCENT_MAX) / 100));
					maxNbSamples = ((nbSamples * (100 + SAMPLE_CORRECTION_PERCENT_MAX) / 100));
					wantedNbSamples = av_clip(wantedNbSamples, minNbSamples, maxNbSamples);
				}
				av_log(nullptr, AV_LOG_TRACE, "diff=%f adiff=%f sample_diff=%d apts=%0.3f\n",
					   diff, avgDiff, wantedNbSamples - nbSamples, m_clock);
			}
		} else {
			// too big difference : may be initial PTS errors, so reset A-V filter
			m_diffAvgCount = 0;
			m_diffCum = 0;
		}
	}

	return wantedNbSamples;
}

/**
 * Decode one audio frame and return its uncompressed size.
 *
 * The processed audio frame is decoded, converted if required, and
 * stored in m_audioBuf, with size in bytes given by the return
 * value.
 */
int
AudioDecoder::decodeFrame(Frame *af)
{
	if(m_vs->paused)
		return -1;

	if(af->serial != m_queue->serial())
		return -1;

	int dataSize = av_samples_get_buffer_size(nullptr, af->frame->channels,
										   af->frame->nb_samples,
										   (AVSampleFormat)af->frame->format, 1);
	int resampledDataSize;

	int64_t decChannelLayout =
		(af->frame->channel_layout &&
		 af->frame->channels == av_get_channel_layout_nb_channels(af->frame->channel_layout)) ?
		af->frame->channel_layout : av_get_default_channel_layout(af->frame->channels);
	int wantedNbSamples = syncAudio(af->frame->nb_samples);

	if(af->frame->format != m_fmtSrc.fmt
	|| decChannelLayout != m_fmtSrc.channelLayout
	|| af->frame->sample_rate != m_fmtSrc.freq
	|| (wantedNbSamples != af->frame->nb_samples && !m_swrCtx)) {
		swr_free(&m_swrCtx);
		m_swrCtx = swr_alloc_set_opts(nullptr,
										 m_fmtTgt.channelLayout, m_fmtTgt.fmt, m_fmtTgt.freq,
										 decChannelLayout, (AVSampleFormat)af->frame->format, af->frame->sample_rate,
										 0, nullptr);
		if(!m_swrCtx || swr_init(m_swrCtx) < 0) {
			av_log(nullptr, AV_LOG_ERROR,
				   "Cannot create sample rate converter for conversion of %d Hz %s %d channels to %d Hz %s %d channels!\n",
				   af->frame->sample_rate, av_get_sample_fmt_name((AVSampleFormat)af->frame->format),
				   af->frame->channels,
				   m_fmtTgt.freq, av_get_sample_fmt_name(m_fmtTgt.fmt), m_fmtTgt.channels);
			swr_free(&m_swrCtx);
			return -1;
		}
		m_fmtSrc.channelLayout = decChannelLayout;
		m_fmtSrc.channels = af->frame->channels;
		m_fmtSrc.freq = af->frame->sample_rate;
		m_fmtSrc.fmt = (AVSampleFormat)af->frame->format;
	}

	if(m_swrCtx) {
		const uint8_t **in = (const uint8_t **)af->frame->extended_data;
		uint8_t **out = &m_audioBuf1;
		int outCount = (int64_t)wantedNbSamples * m_fmtTgt.freq / af->frame->sample_rate + 256;
		int outSize = av_samples_get_buffer_size(nullptr, m_fmtTgt.channels, outCount, m_fmtTgt.fmt, 0);
		int len2;
		if(outSize < 0) {
			av_log(nullptr, AV_LOG_ERROR, "av_samples_get_buffer_size() failed\n");
			return -1;
		}
		if(wantedNbSamples != af->frame->nb_samples) {
			if(swr_set_compensation(m_swrCtx,
					(wantedNbSamples - af->frame->nb_samples) * m_fmtTgt.freq / af->frame->sample_rate,
					wantedNbSamples * m_fmtTgt.freq / af->frame->sample_rate) < 0) {
				av_log(nullptr, AV_LOG_ERROR, "swr_set_compensation() failed\n");
				return -1;
			}
		}
		av_fast_malloc(&m_audioBuf1, &m_buf1Size, outSize);
		if(!m_audioBuf1)
			return AVERROR(ENOMEM);
		len2 = swr_convert(m_swrCtx, out, outCount, in, af->frame->nb_samples);
		if(len2 < 0) {
			av_log(nullptr, AV_LOG_ERROR, "swr_convert() failed\n");
			return -1;
		}
		if(len2 == outCount) {
			av_log(nullptr, AV_LOG_WARNING, "audio buffer is probably too small\n");
			if(swr_init(m_swrCtx) < 0)
				swr_free(&m_swrCtx);
		}
		m_audioBuf = m_audioBuf1;
		resampledDataSize = len2 * m_fmtTgt.channels * av_get_bytes_per_sample(m_fmtTgt.fmt);
	} else {
		m_audioBuf = af->frame->data[0];
		resampledDataSize = dataSize;
	}

	av_unused double audioClock0 = m_clock;
	// update the audio clock with the pts
	if(!isnan(af->pts))
		m_clock = af->pts + (double)af->frame->nb_samples / af->frame->sample_rate;
	else
		m_clock = NAN;
	m_clockSerial = af->serial;
#ifdef DEBUG
	{
		static double lastClock;
		printf("audio: delay=%0.3f clock=%0.3f clock0=%0.3f\n", m_clock - lastClock, m_clock, audioClock0);
		lastClock = m_clock;
	}
#endif
	return resampledDataSize;
}

int
AudioDecoder::getFrame(AVFrame *frame)
{
	const int gotFrame = Decoder::decodeFrame(frame, nullptr);

	if(gotFrame <= 0 || frame->pts == AV_NOPTS_VALUE)
		return gotFrame;

	const double dPts = double(frame->pts) / frame->sample_rate;

	if(!isnan(dPts) && m_vs->seekDecoder > 0. && m_vs->seekDecoder > dPts) {
		av_frame_unref(frame);
		return 0;
	}

	return gotFrame;
}

void
AudioDecoder::queueFrame(Frame *af)
{
	m_callbackTime = av_gettime_relative();

	int audioSize = decodeFrame(af);
	if(audioSize < 0) {
		// if error, just output silence
		m_audioBuf = nullptr;
		m_bufSize = (AUDIO_MIN_BUFFER_SIZE / m_fmtTgt.frameSize) * m_fmtTgt.frameSize;
	} else {
		if(m_vs->showMode != SHOW_MODE_VIDEO)
			m_vs->renderThread->updateSampleDisplay((int16_t *)(void *)m_audioBuf, audioSize);
		m_bufSize = audioSize;
	}

	if(!m_audioBuf || m_vs->demuxer->paused()) {
		uint8_t *silence = new uint8_t[m_bufSize]();
		queueBuffer(silence, m_bufSize);
		delete[] silence;
	} else {
		queueBuffer((uint8_t *)m_audioBuf, audioSize);
	}

	ALint hwBufOffset = 0;
	alGetSourcei(m_alSrc, AL_BYTE_OFFSET, &hwBufOffset);
	if(!isnan(m_clock)) {
		m_vs->audClk.setAt(
					 m_clock - double(m_hwBufQueueSize - hwBufOffset) / m_fmtTgt.bytesPerSec,
					 m_clockSerial,
					 m_callbackTime / double(AV_TIME_BASE));
		m_vs->extClk.syncTo(&m_vs->audClk);
	}
}

void
AudioDecoder::run()
{
	AVFrame *frame = av_frame_alloc();
	Frame *af = new Frame();

	if(!frame)
		return;

	m_clockSerial = -1;

	for(;;) {
		const int got_frame = getFrame(frame);
		assert(got_frame != AVERROR(EAGAIN));
		assert(got_frame != AVERROR_EOF);
		if(got_frame < 0)
			break;

		if(got_frame) {
			AVRational tb = { 1, frame->sample_rate };

			if(!(af->frame = av_frame_alloc()))
				break;

			af->pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);
			af->pos = frame->pkt_pos;
			af->serial = m_pktSerial;
			af->duration = av_q2d(AVRational{ frame->nb_samples, frame->sample_rate });

			av_frame_move_ref(af->frame, frame);

			// time to unqueue one sample in microseconds (AV_TIME_BASE)
			const int64_t sleepTime = int64_t(double(AUDIO_MIN_BUFFER_SIZE / m_fmtTgt.frameSize) / m_fmtTgt.freq * AV_TIME_BASE);
			// bytes needed for 100ms of audio
			const ALint hwMinBytes = m_fmtTgt.bytesPerSec / 10;

			while(!m_vs->abortRequested) {
				ALint bufReady = 0;
				alGetSourcei(m_alSrc, AL_BUFFERS_PROCESSED, &bufReady);
				if(bufReady > 0)
					break;

				ALint hwBufOffset = 0;
				alGetSourcei(m_alSrc, AL_BYTE_OFFSET, &hwBufOffset);
				if(m_hwBufQueueSize - hwBufOffset < hwMinBytes)
					break;

				av_usleep(sleepTime);
			}

			queueFrame(af);

			av_frame_unref(af->frame);
			av_frame_free(&af->frame);
		}
	}

	av_frame_free(&frame);
	delete af;
	return;
}
