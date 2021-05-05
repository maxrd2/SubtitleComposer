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

#ifndef DECODER_H
#define DECODER_H

#include <QThread>

extern "C" {
#include "libavformat/avformat.h"
}

QT_FORWARD_DECLARE_CLASS(QWaitCondition)

namespace SubtitleComposer {
class PacketQueue;
class FrameQueue;
class FFPlayer;

class Decoder : public QThread
{
	Q_OBJECT

public:
	Decoder(QObject *parent = nullptr);

	void init(AVCodecContext *avctx, PacketQueue *pq, FrameQueue *fq, QWaitCondition *emptyQueueCond);
	void start();
	int decodeFrame(AVFrame *frame, AVSubtitle *sub);
	virtual void destroy();
	virtual void abort();

	inline int pktSerial() const { return m_pktSerial; }
	inline int width() const { return m_avCtx->width; }
	inline int height() const { return m_avCtx->height; }
	inline int finished() const { return m_finished; }

	inline void startPts(int64_t pts, const AVRational &tb) { m_startPts = pts; m_startPtsTb = tb; }

protected:
	int m_reorderPts;
	AVPacket *m_pkt;
	PacketQueue *m_queue;
	FrameQueue *m_frameQueue;
	AVCodecContext *m_avCtx;
	int m_pktSerial;
	int m_finished;
	QWaitCondition *m_emptyQueueCond;
	int64_t m_startPts;
	AVRational m_startPtsTb;
	int64_t m_nextPts;
	AVRational m_nextPtsTb;
};
}

#endif // DECODER_H
