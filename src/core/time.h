#ifndef SCTIME_H
#define SCTIME_H

/***************************************************************************
 *   Copyright (C) 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#include <QString>

namespace SubtitleComposer {
class Time
{
public:
	static constexpr double MaxMseconds = 86399999.0;
	static constexpr double MaxSeconds = MaxMseconds / 1000.0;

	/*explicit*/ Time(double mseconds = 0);
	Time(int hours, int minutes, int seconds, int mseconds);
	Time(const Time &time);

	void setSecondsTime(double seconds);
	void setMillisTime(double mseconds);

	inline double toMillis() const { return m_mseconds; }
	inline double toSeconds() const { return m_mseconds / 1000.0; }
	QString toString(bool showMillis = true) const;

	int hours() const;
	bool setHours(int hours);
	int minutes() const;
	bool setMinutes(int hours);
	int seconds() const;
	bool setSeconds(int seconds);
	int mseconds() const;
	bool setMseconds(int mseconds);

	void shift(double mseconds);
	Time shifted(double m_seconds) const;
	void adjust(double shiftMseconds, double scaleFactor);
	Time adjusted(double shiftMseconds, double scaleFactor) const;

	Time & operator=(const Time &time);
	Time & operator=(double mseconds);
	Time & operator+=(const Time &time);
	Time & operator+=(double mseconds);
	Time & operator-=(const Time &time);
	Time & operator-=(double mseconds);

	Time operator+(const Time &time) const;
	Time operator+(double mseconds) const;
	Time operator-(const Time &time) const;
	Time operator-(double mseconds) const;

	bool operator==(const Time &time) const;
	bool operator==(double mseconds) const;
	bool operator!=(const Time &time) const;
	bool operator!=(double mseconds) const;
	bool operator<(const Time &time) const;
	bool operator<(double mseconds) const;
	bool operator<=(const Time &time) const;
	bool operator<=(double mseconds) const;
	bool operator>(const Time &time) const;
	bool operator>(double mseconds) const;
	bool operator>=(const Time &time) const;
	bool operator>=(double mseconds) const;

private:
	double m_mseconds;
};
}
#endif
