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
struct PacketQueue;

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
	int uploaded;
	int flip_v;
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
