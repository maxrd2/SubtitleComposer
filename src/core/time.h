#ifndef SCTIME_H
#define SCTIME_H

/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QString>

namespace SubtitleComposer {
class Time
{
public:
	Time(double mseconds = .0);
	Time(int hours, int minutes, int seconds, int mseconds);
	Time(const Time &time);

	inline void setSecondsTime(double seconds) { setMillisTime(seconds * 1000.0); }
	inline void setMillisTime(double millis) { m_millis = qMax(0.0, millis); }

	inline double toMillis() const { return m_millis; }
	inline double toSeconds() const { return m_millis / 1000.0; }
	QString toString(bool showMillis=true, bool showHours=true) const;

	bool setHours(int hours);
	bool setMinutes(int hours);
	bool setSeconds(int seconds);
	bool setMillis(int millis);

	inline int hours() const { return int(m_millis + 0.5) / 3600000; }
	inline int minutes() const { return (int(m_millis + 0.5) % 3600000) / 60000; }
	inline int seconds() const { return (int(m_millis + 0.5) % 60000) / 1000; }
	inline int millis() const { return int(m_millis + 0.5) % 1000; }

	inline void shift(double millis) { setMillisTime(m_millis + millis); }
	inline Time shifted(double millis) const { return Time(m_millis + millis); }

	inline Time & operator=(const Time &time) { m_millis = time.m_millis; return *this; }
	inline Time & operator=(double millis) { setMillisTime(millis); return *this; }
	inline Time & operator+=(const Time &time) { setMillisTime(m_millis + time.m_millis); return *this; }
	inline Time & operator+=(double millis) { setMillisTime(m_millis + millis); return *this; }
	inline Time & operator-=(const Time &time) { setMillisTime(m_millis - time.m_millis); return *this; }
	inline Time & operator-=(double millis) { setMillisTime(m_millis - millis); return *this; }

	inline Time & operator*=(double factor) { setMillisTime(m_millis * factor); return *this; }
	inline Time & operator/=(double divisor) { setMillisTime(m_millis / divisor); return *this; }

	inline Time operator+(const Time &time) const { return Time(m_millis + time.m_millis); }
	inline Time operator+(double millis) const { return Time(m_millis + millis); }
	inline Time operator-(const Time &time) const { return Time(m_millis - time.m_millis); }
	inline Time operator-(double millis) const { return Time(m_millis - millis); }

	inline Time operator*(double factor) const { return Time(m_millis * factor); }
	inline Time operator/(double divisor) const { return Time(m_millis / divisor); }

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
