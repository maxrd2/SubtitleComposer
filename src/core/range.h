/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RANGE_H
#define RANGE_H

#include <climits>

#include <QList>

#include <QDebug>

namespace SubtitleComposer {
class Range
{
	friend class RangeList;

public:
	static const int MaxIndex = INT_MAX;

	Range(int index) :
		m_start(index),
		m_end(index)
	{
		Q_ASSERT(index >= 0);
		Q_ASSERT(index <= MaxIndex);
	}

	Range(int startIndex, int endIndex) :
		m_start(startIndex),
		m_end(endIndex)
	{
		Q_ASSERT(m_start >= 0);
		Q_ASSERT(m_end >= 0);
		Q_ASSERT(m_start <= m_end);
		Q_ASSERT(m_start <= MaxIndex);
	}

	Range(const Range &range) :
		m_start(range.m_start),
		m_end(range.m_end)
	{}

	Range & operator=(const Range &range)
	{
		if(this == &range)
			return *this;

		m_start = range.m_start;
		m_end = range.m_end;

		return *this;
	}

	inline static Range full()
	{
		return Range(0, MaxIndex);
	}

	inline static Range lower(int index)
	{
		Q_ASSERT(index >= 0);
		Q_ASSERT(index <= MaxIndex);
		return Range(0, index);
	}

	inline static Range upper(int index)
	{
		Q_ASSERT(index >= 0);
		Q_ASSERT(index <= MaxIndex);
		return Range(index, MaxIndex);
	}

	inline bool isFullRange(int maxIndex = MaxIndex) const
	{
		return m_start == 0 && m_end == maxIndex;
	}

	inline int start() const
	{
		return m_start;
	}

	inline int end() const
	{
		return m_end;
	}

	inline int length() const
	{
		return m_end - m_start + 1;
	}

	inline bool contains(int index) const
	{
		return index >= m_start && index <= m_end;
	}

	inline bool contains(const Range &range) const
	{
		return m_end <= range.m_end && m_start >= range.m_start;
	}

	inline bool operator==(const Range &range) const
	{
		return m_start == range.m_start && m_end == range.m_end;
	}

	inline bool operator!=(const Range &range) const
	{
		return m_start != range.m_start || m_end != range.m_end;
	}

	inline bool operator<(const Range &range) const
	{
		return m_end < range.m_start;
	}

	inline bool operator<=(const Range &range) const
	{
		return m_end <= range.m_end && m_start <= range.m_start;
	}

	inline bool operator>(const Range &range) const
	{
		return m_start > range.m_end;
	}

	inline bool operator>=(const Range &range) const
	{
		return m_start >= range.m_start && m_end >= range.m_end;
	}

private:
	Range() {}

	int m_start;
	int m_end;
};
}

#endif
