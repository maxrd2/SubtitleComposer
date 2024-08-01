/*
    SPDX-FileCopyrightText: 2003 Fabrice Bellard
    SPDX-FileCopyrightText: 2020-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DECODER_H
#define DECODER_H

#include <QThread>

extern "C" {
#include "libavcodec/avcodec.h"
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

	struct FrameData {
		int64_t pkt_pos;
	};
};
}

#endif // DECODER_H
