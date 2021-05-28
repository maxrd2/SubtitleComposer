#ifndef SCTIME_H
#define SCTIME_H

/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2018 Mladen Milinkovic <max@smoothware.net>
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

#include <QString>

namespace SubtitleComposer {
class Time
{
public:
	// FIXME: why 24h limit? remove it
	constexpr static double MaxMseconds = 86399999.0;
	constexpr static double MaxSeconds = MaxMseconds / 1000.0;

	Time(double mseconds = .0);
	Time(int hours, int minutes, int seconds, int mseconds);
	Time(const Time &time);

	inline void setSecondsTime(double seconds) { setMillisTime(seconds * 1000.0); }
	inline void setMillisTime(double millis) { m_millis = qMax(0.0, qMin(millis, double(MaxMseconds))); }

	inline double toMillis() const { return m_millis; }
	inline double toSeconds() const { return m_millis / 1000.0; }
	QString toString(bool showMillis=true, bool showHours=true) const;

	bool setHours(int hours);
	bool setMinutes(int hours);
	bool setSeconds(int seconds);
	bool setMillis(int millis);

	// FIXME: why 24h limit? remove it
	inline int hours() const { return int(m_millis + 0.5) / 3600000; }
	inline int minutes() const { return (int(m_millis + 0.5) % 3600000) / 60000; }
	inline int seconds() const { return (int(m_millis + 0.5) % 60000) / 1000; }
	inline int millis() const { return int(m_millis + 0.5) % 1000; }

	inline void shift(double millis) { setMillisTime(m_millis + millis); }
	inline Time shifted(double millis) const { return Time(m_millis + millis); }

	// TODO: drop these
	inline bool setMseconds(int millis) { return setMillis(millis); }
	inline int mseconds() const { return millis(); }

	// FIXME: this is confusing... add/change to * operator
	inline void adjust(double shiftMseconds, double scaleFactor) { setMillisTime(shiftMseconds + m_millis * scaleFactor); }
	inline Time adjusted(double shiftMseconds, double scaleFactor) const { return Time(shiftMseconds + m_millis * scaleFactor); }

	inline Time & operator=(const Time &time) { m_millis = time.m_millis; return *this; }
	inline Time & operator=(double millis) { setMillisTime(millis); return *this; }
	inline Time & operator+=(const Time &time) { setMillisTime(m_millis + time.m_millis); return *this; }
	inline Time & operator+=(double millis) { setMillisTime(m_millis + millis); return *this; }
	inline Time & operator-=(const Time &time) { setMillisTime(m_millis - time.m_millis); return *this; }
	inline Time & operator-=(double millis) { setMillisTime(m_millis - millis); return *this; }

	inline Time operator+(const Time &time) const { return Time(m_millis + time.m_millis); }
	inline Time operator+(double millis) const { return Time(m_millis + millis); }
	inline Time operator-(const Time &time) const { return Time(m_millis - time.m_millis); }
	inline Time operator-(double millis) const { return Time(m_millis - millis); }

	inline bool operator==(const Time &time) const { return m_millis == time.m_millis; }
	inline bool operator==(double millis) const { return m_millis == millis; }
	inline bool operator!=(const Time &time) const { return m_millis != time.m_millis; }
	inline bool operator!=(double millis) const { return m_millis != millis; }
	inline bool operator<(const Time &time) const { return m_millis < time.m_millis; }
	inline bool operator<(double millis) const { return m_millis < millis; }
	inline bool operator<=(const Time &time) const { return m_millis <= time.m_millis; }
	inline bool operator<=(double millis) const { return m_millis <= millis; }
	inline bool operator>(const Time &time) const { return m_millis > time.m_millis; }
	inline bool operator>(double millis) const { return m_millis > millis; }
	inline bool operator>=(const Time &time) const { return m_millis >= time.m_millis; }
	inline bool operator>=(double millis) const { return m_millis >= millis; }

private:
	double m_millis;
};
}
#endif
