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

#ifndef AUDIODECODER_H
#define AUDIODECODER_H

#include "videoplayer/backend/decoder.h"


struct SwrContext;
struct ALCdevice;
struct ALCcontext;

namespace SubtitleComposer {
class VideoState;
struct Frame;

class AudioDecoder : public Decoder
{
	Q_OBJECT

public:
	AudioDecoder(VideoState *state, QObject *parent = nullptr);

	void destroy() override;
	void abort() override;

	void setListenerGain(double gain);
	double pitch() const;
	void setPitch(double pitch);

private:
	void run() override;

	struct Params {
		int freq;
		int channels;
		int64_t channelLayout;
		AVSampleFormat fmt;
		int frameSize;
		int bytesPerSec;
	};

	int decodeFrame(Frame *af);
	int getFrame(AVFrame *frame);
	void queueFrame(Frame *af);
	void queueBuffer(uint8_t *data, int len);
	int syncAudio(int nbSamples);

	bool open(int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate);
	void close();
	void flush();
	void play();
	void pause();

	VideoState *m_vs;

	Params m_fmtSrc;
	Params m_fmtTgt;
	SwrContext *m_swrCtx;
	int m_hwBufQueueSize; // in bytes
	uint8_t *m_audioBuf;
	unsigned int m_bufSize; // in bytes
	uint8_t *m_audioBuf1;
	unsigned int m_buf1Size;
	double m_clock;
	int m_clockSerial;
	int64_t m_callbackTime;

	double m_diffCum; /* used for AV difference average computation */
	double m_diffAvgCoef;
	int m_diffAvgCount;

	ALCdevice *m_alDev;
	ALCcontext *m_alCtx;
	unsigned int m_alSrc;
	int m_bufCnt;
	int m_bufFmt;

	friend class StreamDemuxer;
};
}

#endif // AUDIODECODER_H
