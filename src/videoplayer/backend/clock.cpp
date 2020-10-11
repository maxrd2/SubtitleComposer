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

#include "clock.h"

#include <cmath>

extern "C" {
#include "libavutil/time.h"
#include "libavutil/avutil.h"
}

#include "videoplayer/backend/packetqueue.h"

using namespace SubtitleComposer;

Clock::Clock()
	: m_pts(0.),
	  m_ptsDrift(0.),
	  m_lastUpdated(0.),
	  m_speed(0.),
	  m_serial(0),
	  m_paused(false),
	  m_queueSerial(nullptr)
{

}

double
Clock::get() const
{
	if(*m_queueSerial != m_serial)
		return NAN;
	if(m_paused) {
		return m_pts;
	} else {
		double time = av_gettime_relative() / double(AV_TIME_BASE);
		return m_ptsDrift + time - (time - m_lastUpdated) * (1.0 - m_speed);
	}
}

void
Clock::setAt(double pts, int serial, double time)
{
	m_pts = pts;
	m_lastUpdated = time;
	m_ptsDrift = m_pts - time;
	m_serial = serial;
}

void
Clock::set(double pts, int serial)
{
	double time = av_gettime_relative() / double(AV_TIME_BASE);
	setAt(pts, serial, time);
}

void
Clock::setSpeed(double speed)
{
	set(get(), m_serial);
	m_speed = speed;
}

void
Clock::init(const PacketQueue *queue)
{
	m_speed = 1.0;
	m_paused = 0;
	m_queueSerial = queue == nullptr ? &m_serial : &queue->m_serial;
	set(NAN, -1);
}

void
Clock::syncTo(Clock *other)
{
	double clock = get();
	double otherClock = other->get();
	if(!isnan(otherClock) && (isnan(clock) || fabs(clock - otherClock) > AV_NOSYNC_THRESHOLD))
		set(otherClock, other->m_serial);
}
