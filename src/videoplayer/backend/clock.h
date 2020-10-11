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

#ifndef CLOCK_H
#define CLOCK_H

// no AV correction is done if too big error
#define AV_NOSYNC_THRESHOLD 10.0

namespace SubtitleComposer {
class PacketQueue;

class Clock
{
public:
	Clock();

	double get() const;
	void setAt(double pts, int serial, double time);
	void set(double pts, int serial);
	void setSpeed(double speed);
	void init(const PacketQueue *queue);
	void syncTo(Clock *other);

	inline double pts() const { return m_pts; }
	inline double lastUpdated() const { return m_lastUpdated; }
	inline double speed() const { return m_speed; }
	inline int serial() const { return m_serial; }
	inline int paused() const { return m_paused; }
	inline void pause(bool pause) { m_paused = pause; }

private:
	double m_pts; // clock base
	double m_ptsDrift; // clock base minus time at which we updated the clock
	double m_lastUpdated;
	double m_speed;
	int m_serial; // clock is based on a packet with this serial
	bool m_paused;
	const int *m_queueSerial; // pointer to the current packet queue serial, used for obsolete clock detection
};
}

#endif // CLOCK_H
