/*
    SPDX-FileCopyrightText: 2003 Fabrice Bellard
    SPDX-FileCopyrightText: 2020-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
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
	  m_fmtSrc({}),
	  m_fmtTgt({}),
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
AudioDecoder::open(AVChannelLayout *wantChLayout, int wantSampleRate)
{
	const static QMap<int, const char *> bufFmtMap = {
		{ 4, "AL_FORMAT_QUAD16" },
		{ 6, "AL_FORMAT_51CHN16" },
		{ 7, "AL_FORMAT_61CHN16" },
		{ 8, "AL_FORMAT_71CHN16" },
	};

	int err;

	if(wantSampleRate <= 0 || !wantChLayout || wantChLayout->nb_channels <= 0) {
		av_log(nullptr, AV_LOG_ERROR, "openal: invalid sample rate or channel count!\n");
		return false;
	}

	int availNbChan = wantChLayout->nb_channels;
	for(;;) {
		while(availNbChan > 2 && !bufFmtMap.contains(availNbChan))
			availNbChan--;
		if(availNbChan <= 2) {
			m_bufFmt = availNbChan == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
			break;
		}
		m_bufFmt = alGetEnumValue(bufFmtMap[wantChLayout->nb_channels]);
		if(m_bufFmt)
			break;
		availNbChan--;
	}

	if(wantChLayout->nb_channels != availNbChan || wantChLayout->order != AV_CHANNEL_ORDER_NATIVE) {
		av_channel_layout_uninit(wantChLayout);
		av_channel_layout_default(wantChLayout, availNbChan);
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
	if((err = av_channel_layout_copy(&m_fmtTgt.chLayout, wantChLayout)) < 0) {
		av_log(nullptr, AV_LOG_ERROR, "av_channel_layout_copy() failed (errL %d).\n", err);
		close();
		return false;
	}

	m_fmtTgt.frameSize = av_samples_get_buffer_size(nullptr, m_fmtTgt.chLayout.nb_channels, 1, m_fmtTgt.fmt, 1);
	m_fmtTgt.bytesPerSec = av_samples_get_buffer_size(nullptr, m_fmtTgt.chLayout.nb_channels, m_fmtTgt.freq, m_fmtTgt.fmt, 1);
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
	if(state != AL_PLAYING) {
		play();
		if(m_vs->paused || m_vs->step)
			pause();
	}
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

		if(!std::isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD) {
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
				av_log(nullptr, AV_LOG_TRACE, "diff=%f adiff=%f sample_diff=%d\n",
					   diff, avgDiff, wantedNbSamples - nbSamples);
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
	// CONVERTED maxrd2
	if(af->serial != m_queue->serial())
		return -1;

	int dataSize = av_samples_get_buffer_size(nullptr, af->frame->ch_layout.nb_channels,
										   af->frame->nb_samples,
										   (AVSampleFormat)af->frame->format, 1);
	int resampledDataSize;

	int wantedNbSamples = syncAudio(af->frame->nb_samples);

	if(af->frame->format != m_fmtSrc.fmt
	|| av_channel_layout_compare(&af->frame->ch_layout, &m_fmtSrc.chLayout)
	|| af->frame->sample_rate != m_fmtSrc.freq
	|| (wantedNbSamples != af->frame->nb_samples && !m_swrCtx)) {
		swr_free(&m_swrCtx);
		swr_alloc_set_opts2(&m_swrCtx,
							&m_fmtTgt.chLayout, m_fmtTgt.fmt, m_fmtTgt.freq,
							&af->frame->ch_layout, AVSampleFormat(af->frame->format), af->frame->sample_rate,
							0, nullptr);
		if(!m_swrCtx || swr_init(m_swrCtx) < 0) {
			av_log(nullptr, AV_LOG_ERROR,
				   "Cannot create sample rate converter for conversion of %d Hz %s %d channels to %d Hz %s %d channels!\n",
				   af->frame->sample_rate, av_get_sample_fmt_name((AVSampleFormat)af->frame->format),
				   af->frame->ch_layout.nb_channels,
				   m_fmtTgt.freq, av_get_sample_fmt_name(m_fmtTgt.fmt), m_fmtTgt.chLayout.nb_channels);
			swr_free(&m_swrCtx);
			return -1;
		}
		if(av_channel_layout_copy(&m_fmtSrc.chLayout, &af->frame->ch_layout) < 0)
			return -1;
		m_fmtSrc.freq = af->frame->sample_rate;
		m_fmtSrc.fmt = (AVSampleFormat)af->frame->format;
	}

	if(m_swrCtx) {
		const int outCount = (int64_t)wantedNbSamples * m_fmtTgt.freq / af->frame->sample_rate + 256;
		const int outSize = av_samples_get_buffer_size(nullptr, m_fmtTgt.chLayout.nb_channels, outCount, m_fmtTgt.fmt, 0);
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
		const int outSamplesPerChannel = swr_convert(m_swrCtx, &m_audioBuf1, outCount,
						   (const uint8_t **)af->frame->extended_data, af->frame->nb_samples);
		if(outSamplesPerChannel < 0) {
			av_log(nullptr, AV_LOG_ERROR, "swr_convert() failed\n");
			return -1;
		}
		if(outSamplesPerChannel == outCount) {
			av_log(nullptr, AV_LOG_WARNING, "audio buffer is probably too small\n");
			if(swr_init(m_swrCtx) < 0)
				swr_free(&m_swrCtx);
		}
		m_audioBuf = m_audioBuf1;
		resampledDataSize = outSamplesPerChannel * m_fmtTgt.chLayout.nb_channels * av_get_bytes_per_sample(m_fmtTgt.fmt);
	} else {
		m_audioBuf = af->frame->data[0];
		resampledDataSize = dataSize;
	}

	return resampledDataSize;
}

int
AudioDecoder::getFrame(AVFrame *frame)
{
	const int gotFrame = Decoder::decodeFrame(frame, nullptr);

	if(gotFrame <= 0 || frame->pts == AV_NOPTS_VALUE)
		return gotFrame;

	const double dPts = double(frame->pts) / frame->sample_rate;

	if(!std::isnan(dPts) && m_vs->seekDecoder > 0. && m_vs->seekDecoder > dPts) {
		av_frame_unref(frame);
		return 0;
	}

	return gotFrame;
}

void
AudioDecoder::queueFrame(Frame *af)
{
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

	if(!m_audioBuf) {
		uint8_t *silence = new uint8_t[m_bufSize]();
		queueBuffer(silence, m_bufSize);
		delete[] silence;
	} else {
		queueBuffer((uint8_t *)m_audioBuf, audioSize);
	}
}

void
AudioDecoder::run()
{
	AVFrame *frame = av_frame_alloc();
	Frame *af = new Frame();

	if(!frame)
		return;

	for(;;) {
		const int got_frame = getFrame(frame);
		Q_ASSERT(got_frame != AVERROR(EAGAIN));
		Q_ASSERT(got_frame != AVERROR_EOF);
		if(got_frame < 0)
			break;

		if(got_frame) {
			if(!(af->frame = av_frame_alloc()))
				break;

			Decoder::FrameData *fd = reinterpret_cast<Decoder::FrameData*>(frame->opaque_ref ? frame->opaque_ref->data : nullptr);

			af->pts = frame->pts == AV_NOPTS_VALUE ? NAN : double(frame->pts) / frame->sample_rate;
			af->pos = fd ? fd->pkt_pos : -1;
			af->serial = m_pktSerial;
			af->duration = double(frame->nb_samples) / frame->sample_rate;

			av_frame_move_ref(af->frame, frame);

			// time to unqueue one sample in microseconds (AV_TIME_BASE)
			const int64_t sleepTime = int64_t(double(AUDIO_MIN_BUFFER_SIZE / m_fmtTgt.frameSize) / (m_fmtTgt.freq * m_vs->audClk.speed()) * AV_TIME_BASE);
			// bytes needed for 100ms of audio
			const ALint hwMinBytes = m_vs->audClk.speed() * m_fmtTgt.bytesPerSec * .100;

			while(!m_vs->abortRequested && !isInterruptionRequested()) {
				ALint hwBufOffset = 0;
				alGetSourcei(m_alSrc, AL_BYTE_OFFSET, &hwBufOffset);
				if(!std::isnan(af->pts)) {
					m_vs->audClk.setAt(
								 af->pts - double(m_hwBufQueueSize - hwBufOffset) / m_fmtTgt.bytesPerSec,
								 af->serial,
								 av_gettime_relative() / double(AV_TIME_BASE));
					m_vs->extClk.syncTo(&m_vs->audClk);
				}

				if(!m_vs->paused) {
					if(m_hwBufQueueSize - hwBufOffset < hwMinBytes)
						break;

					ALint bufReady = 0;
					alGetSourcei(m_alSrc, AL_BUFFERS_PROCESSED, &bufReady);
					if(bufReady > 0)
						break;
				}

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
