/*
    SPDX-FileCopyrightText: 2003 Fabrice Bellard
    SPDX-FileCopyrightText: 2020-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VIDEODECODER_H
#define VIDEODECODER_H

#include "videoplayer/backend/decoder.h"

namespace SubtitleComposer {
class VideoState;

class VideoDecoder : public Decoder
{
	Q_OBJECT

public:
	VideoDecoder(VideoState *state, QObject *parent = nullptr);

private:
	void run() override;

	int getVideoFrame(AVFrame *frame);
	int queuePicture(AVFrame *srcFrame, double pts, double duration, int64_t pos, int serial);

	VideoState *m_vs;

	double m_timeBase;

	int m_frameDropsEarly;
};
}

#endif // VIDEODECODER_H
