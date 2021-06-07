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

#include "framequeue.h"

#include <QMutex>
#include <QWaitCondition>

#include "packetqueue.h"

using namespace SubtitleComposer;

FrameQueue::FrameQueue()
	: m_rIndex(0),
	  m_wIndex(0),
	  m_size(0),
	  m_maxSize(0),
	  m_keepLast(0),
	  m_rIndexShown(0),
	  m_mutex(nullptr),
	  m_cond(nullptr),
	  m_pktQ(nullptr)
{
	memset(m_queue, 0, sizeof(m_queue));
}

void
FrameQueue::unrefItem(Frame *vp)
{
	av_frame_unref(vp->frame);
	avsubtitle_free(&vp->sub);
}

int
FrameQueue::init(PacketQueue *pktq, int maxSize, int keepLast)
{
	m_rIndex = m_wIndex = m_size = m_rIndexShown = 0;
	m_mutex = new QMutex();
	m_cond = new QWaitCondition();
	m_pktQ = pktq;
	m_maxSize = FFMIN(maxSize, FRAME_QUEUE_SIZE);
	m_keepLast = !!keepLast;
	for(int i = 0; i < m_maxSize; i++)
		if((m_queue[i].frame = av_frame_alloc()) == nullptr)
			return AVERROR(ENOMEM);
	return 0;
}

void
FrameQueue::destory()
{
	for(int i = 0; i < m_maxSize; i++) {
		Frame *vp = &m_queue[i];
		FrameQueue::unrefItem(vp);
		av_frame_free(&vp->frame);
	}
	delete m_mutex;
	delete m_cond;
}

void
FrameQueue::signal()
{
	m_mutex->lock();
	m_cond->wakeOne();
	m_mutex->unlock();
}

Frame *
FrameQueue::peek()
{
	return &m_queue[(m_rIndex + m_rIndexShown) % m_maxSize];
}

Frame *
FrameQueue::peekNext()
{
	return &m_queue[(m_rIndex + m_rIndexShown + 1) % m_maxSize];
}

Frame *
FrameQueue::peekLast()
{
	return &m_queue[m_rIndex];
}

Frame *
FrameQueue::peekWritable()
{
	// wait until we have space to put a new frame
	m_mutex->lock();
	while(m_size >= m_maxSize && !m_pktQ->m_abortRequest)
		m_cond->wait(m_mutex);
	m_mutex->unlock();

	if(m_pktQ->m_abortRequest)
		return nullptr;

	return &m_queue[m_wIndex];
}

Frame *
FrameQueue::peekReadable()
{
	// wait until we have a new readable frame
	m_mutex->lock();
	while(m_size - m_rIndexShown <= 0 && !m_pktQ->m_abortRequest)
		m_cond->wait(m_mutex);
	m_mutex->unlock();

	if(m_pktQ->m_abortRequest)
		return nullptr;

	return &m_queue[(m_rIndex + m_rIndexShown) % m_maxSize];
}

void
FrameQueue::push()
{
	if(++m_wIndex == m_maxSize)
		m_wIndex = 0;
	m_mutex->lock();
	m_size++;
	m_cond->wakeOne();
	m_mutex->unlock();
}

void
FrameQueue::next()
{
	if(m_keepLast && !m_rIndexShown) {
		m_rIndexShown = 1;
		return;
	}
	FrameQueue::unrefItem(&m_queue[m_rIndex]);
	if(++m_rIndex == m_maxSize)
		m_rIndex = 0;
	m_mutex->lock();
	m_size--;
	m_cond->wakeOne();
	m_mutex->unlock();
}

/* return the number of undisplayed frames in the queue */
int
FrameQueue::nbRemaining()
{
	return m_size - m_rIndexShown;
}

/* return last shown position */
int64_t
FrameQueue::lastPos()
{
	Frame *fp = &m_queue[m_rIndex];
	if(m_rIndexShown && fp->serial == m_pktQ->m_serial)
		return fp->pos;
	else
		return -1;
}

