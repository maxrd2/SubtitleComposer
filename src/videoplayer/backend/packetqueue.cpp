/*
    SPDX-FileCopyrightText: 2003 Fabrice Bellard
    SPDX-FileCopyrightText: 2020-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "packetqueue.h"

#include <QMutex>
#include <QWaitCondition>

#include "ffplayer.h"

using namespace SubtitleComposer;

PacketQueue::PacketQueue()
	: m_firstPkt(nullptr),
	  m_lastPkt(nullptr),
	  m_nbPackets(0),
	  m_size(0),
	  m_duration(0),
	  m_abortRequest(false),
	  m_serial(0),
	  m_mutex(nullptr),
	  m_cond(nullptr)
{
}

int
PacketQueue::put_private(AVPacket **pkt)
{
	if(m_abortRequest) {
		if((*pkt)->data == FFPlayer::flushPkt())
			(*pkt)->data = nullptr;
		av_packet_free(pkt);
		return -1;
	}

	PacketList *pkt1 = new PacketList();
	pkt1->pkt = *pkt;
	*pkt = nullptr;

	pkt1->next = nullptr;
	if(pkt1->pkt->data == FFPlayer::flushPkt())
		m_serial++;
	pkt1->serial = m_serial;

	if(!m_lastPkt)
		m_firstPkt = pkt1;
	else
		m_lastPkt->next = pkt1;
	m_lastPkt = pkt1;
	m_nbPackets++;
	m_size += pkt1->pkt->size + sizeof(*pkt1);
	m_duration += pkt1->pkt->duration;
	// XXX: should duplicate packet data in DV case
	m_cond->wakeOne();
	return 0;
}

int
PacketQueue::putFlushPacket()
{
	QMutexLocker l(m_mutex);
	AVPacket *pkt = av_packet_alloc();
	Q_ASSERT(pkt != nullptr);
	pkt->data = FFPlayer::flushPkt();
	return put_private(&pkt);
}

int
PacketQueue::put(AVPacket **pkt)
{
	QMutexLocker l(m_mutex);
	return put_private(pkt);
}

int
PacketQueue::putNullPacket(int streamIndex)
{
	AVPacket *pkt = av_packet_alloc();
	Q_ASSERT(pkt != nullptr);
	pkt->stream_index = streamIndex;
	return put(&pkt);
}

int
PacketQueue::init()
{
	m_firstPkt = m_lastPkt = nullptr;
	m_nbPackets = m_size = m_serial = 0;
	m_duration = 0;
	m_mutex = new QMutex();
	m_cond = new QWaitCondition();
	m_abortRequest = true;
	return 0;
}

void
PacketQueue::flush()
{
	QMutexLocker l(m_mutex);
	PacketList *pkt, *pkt1;
	for(pkt = m_firstPkt; pkt; pkt = pkt1) {
		pkt1 = pkt->next;
		av_packet_free(&pkt->pkt);
		delete pkt;
	}
	m_lastPkt = nullptr;
	m_firstPkt = nullptr;
	m_nbPackets = 0;
	m_size = 0;
	m_duration = 0;
}

void
PacketQueue::destroy()
{
	flush();
	delete m_mutex;
	delete m_cond;
}

void
PacketQueue::abort()
{
	QMutexLocker l(m_mutex);
	m_abortRequest = true;
	m_cond->wakeOne();
}

void
PacketQueue::start()
{
	QMutexLocker l(m_mutex);
	m_abortRequest = false;
	AVPacket *pkt = av_packet_alloc();
	Q_ASSERT(pkt != nullptr);
	pkt->data = FFPlayer::flushPkt();
	put_private(&pkt);
}

int
PacketQueue::get(AVPacket **pkt, int block, int *serial)
{
	QMutexLocker l(m_mutex);

	for(;;) {
		if(m_abortRequest)
			return -1;

		PacketList *pkt1 = m_firstPkt;
		if(pkt1) {
			m_firstPkt = pkt1->next;
			if(!m_firstPkt)
				m_lastPkt = nullptr;
			m_nbPackets--;
			m_size -= pkt1->pkt->size + sizeof(*pkt1);
			m_duration -= pkt1->pkt->duration;
			*pkt = pkt1->pkt;
			if(serial)
				*serial = pkt1->serial;
			delete pkt1;
			return 1;
		}

		if(!block)
			return 0;

		m_cond->wait(m_mutex);
	}
}

