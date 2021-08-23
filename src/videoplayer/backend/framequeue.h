/*
    SPDX-FileCopyrightText: 2003 Fabrice Bellard
    SPDX-FileCopyrightText: 2020 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FRAMEQUEUE_H
#define FRAMEQUEUE_H

extern "C" {
#include "libavformat/avformat.h"
}

#include <QObject>

QT_FORWARD_DECLARE_CLASS(QMutex)
QT_FORWARD_DECLARE_CLASS(QWaitCondition)

#define VIDEO_PICTURE_QUEUE_SIZE 3
#define SUBPICTURE_QUEUE_SIZE 16
#define SAMPLE_QUEUE_SIZE 9
#define FRAME_QUEUE_SIZE FFMAX(SAMPLE_QUEUE_SIZE, FFMAX(VIDEO_PICTURE_QUEUE_SIZE, SUBPICTURE_QUEUE_SIZE))

namespace SubtitleComposer {
class PacketQueue;

struct Frame {
	AVFrame *frame;
	AVSubtitle sub;
	int serial;
	double pts;           /* presentation timestamp for the frame */
	double duration;      /* estimated duration of the frame */
	int64_t pos;          /* byte position of the frame in the input file */
	int width;
	int height;
	int format;
	AVRational sar;
	bool uploaded;
};

class FrameQueue
{
	friend class RenderThread;

public:
	FrameQueue();

	void unrefItem(Frame *vp);
	int init(PacketQueue *pktq, int maxSize, int keepLast);
	void destory();
	void signal();
	Frame * peek();
	Frame * peekNext();
	Frame * peekLast();
	Frame * peekWritable();
	Frame * peekReadable();
	void push();
	void next();
	int nbRemaining();
	int64_t lastPos();

private:
	Frame m_queue[FRAME_QUEUE_SIZE];
	int m_rIndex;
	int m_wIndex;
	int m_size;
	int m_maxSize;
	int m_keepLast;
	int m_rIndexShown;
	QMutex *m_mutex;
	QWaitCondition *m_cond;
	PacketQueue *m_pktQ;
};
}

#endif // FRAMEQUEUE_H
