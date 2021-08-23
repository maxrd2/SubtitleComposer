/*
    SPDX-FileCopyrightText: 2003 Fabrice Bellard
    SPDX-FileCopyrightText: 2020 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "subtitledecoder.h"

#include "videoplayer/backend/ffplayer.h"

using namespace SubtitleComposer;

SubtitleDecoder::SubtitleDecoder(QObject *parent)
	: Decoder(parent)
{

}

void
SubtitleDecoder::run()
{
	for(;;) {
		Frame *sp = m_frameQueue->peekWritable();
		if(!sp)
			break;

		int gotSubtitle = decodeFrame(nullptr, &sp->sub);
		if(gotSubtitle < 0)
			break;

		double pts = 0;

		if(gotSubtitle && sp->sub.format == 0) {
			if(sp->sub.pts != AV_NOPTS_VALUE)
				pts = sp->sub.pts / (double)AV_TIME_BASE;
			sp->pts = pts;
			sp->serial = pktSerial();
			sp->width = width();
			sp->height = height();
			sp->uploaded = false;

			// now we can update the picture count
			m_frameQueue->push();
		} else if(gotSubtitle) {
			avsubtitle_free(&sp->sub);
		}
	}
}
