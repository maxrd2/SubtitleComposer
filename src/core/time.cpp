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

#include "core/time.h"

using namespace SubtitleComposer;


Time::Time(double mseconds)
{
	setMillisTime(mseconds);
}

Time::Time(int hours, int minutes, int seconds, int mseconds)
	: m_millis(0)
{
	setHours(hours);
	setMinutes(minutes);
	setSeconds(seconds);
	setMillis(mseconds);
}

Time::Time(const Time &time)
	: m_millis(time.m_millis)
{
}

QString
Time::toString(bool showMillis, bool showHours) const
{
	int time = m_millis + 0.5;

	int i = 14;
	QChar data[15];

#define APPPEND_DIGIT(base) { \
	data[i--] = QLatin1Char('0' + time % base); \
	time /= base; \
}

	// terminator
	data[i--] = QChar::Null;

	// milliseconds
	if(showMillis) {
		APPPEND_DIGIT(10);
		APPPEND_DIGIT(10);
		APPPEND_DIGIT(10);
		data[i--] = QLatin1Char('.');
	} else {
		time /= 1000;
	}

	// seconds
	APPPEND_DIGIT(10);
	APPPEND_DIGIT(6);
	data[i--] = QLatin1Char(':');

	// minutes
	APPPEND_DIGIT(10);
	APPPEND_DIGIT(6);

	if(time || showHours) {
		data[i--] = QLatin1Char(':');
		for(int n = 0; i >= 0 && (n < 2 || time); n++)
			APPPEND_DIGIT(10);
	}
#undef APPPEND_DIGIT

	return QString(data + i + 1);
}

bool
Time::setHours(int hours)
{
	if(hours < 0)
		return false;

	m_millis += (hours - this->hours()) * 3600000;

	return true;
}

bool
Time::setMinutes(int minutes)
{
	if(minutes < 0 || minutes > 59)
		return false;

	m_millis += (minutes - this->minutes()) * 60000;

	return true;
}

bool
Time::setSeconds(int seconds)
{
	if(seconds < 0 || seconds > 59)
		return false;

	m_millis += (seconds - this->seconds()) * 1000;

	return true;
}

bool
Time::setMillis(int millis)
{
	if(millis < 0 || millis > 999)
		return false;

	m_millis += millis - this->millis();

	return true;
}
