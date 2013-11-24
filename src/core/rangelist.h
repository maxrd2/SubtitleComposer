#ifndef RANGELIST_H
#define RANGELIST_H

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "range.h"

#include <QtCore/QList>
#include <QStringList>

#include <KDebug>

namespace SubtitleComposer {
class RangeList
{
public:
	typedef QList<Range>::Iterator Iterator;
	typedef QList<Range>::ConstIterator ConstIterator;

	RangeList() {}

	RangeList(const Range &range)
	{
		m_ranges.append(range);
	}

	RangeList(const RangeList &ranges) : m_ranges(ranges.m_ranges) {}

	RangeList & operator=(const RangeList &ranges)
	{
		if(this == &ranges)
			return *this;

		m_ranges = ranges.m_ranges;

		return *this;
	}

	bool operator==(const RangeList &ranges) const
	{
		return m_ranges == ranges.m_ranges;
	}

	bool operator!=(const RangeList &ranges) const
	{
		return m_ranges != ranges.m_ranges;
	}

	bool isFullRange(int maxIndex = Range::MaxIndex) const
	{
		return m_ranges.count() == 1 && m_ranges.first().isFullRange(maxIndex);
	}

	RangeList complement() const
	{
		kDebug() << this->inspect();

		if(m_ranges.empty())
			return Range(0, Range::MaxIndex);

		RangeList ret;

		QList<Range>::ConstIterator it = m_ranges.begin();

		if((*it).m_start > 0)
			ret << Range(0, (*it).m_start - 1);

		int lastEnd = (*it).m_end;
		++it;
		for(QList<Range>::ConstIterator end = m_ranges.end(); it != end; ++it) {
			ret << Range(lastEnd + 1, (*it).m_start - 1);
			lastEnd = (*it).m_end;
		}
		if(lastEnd < Range::MaxIndex)
			ret << Range(lastEnd + 1, Range::MaxIndex);

		kDebug() << ret.inspect();

		return ret;
	}

	bool contains(int index) const
	{
		for(QList<Range>::ConstIterator it = m_ranges.begin(), end2 = m_ranges.end(); it != end2; ++it)
			if((*it).contains(index))
				return true;

		return false;
	}

	Range range(int rangeIndex) const
	{
		Q_ASSERT(rangeIndex >= 0);
		Q_ASSERT(rangeIndex < m_ranges.count());

		return m_ranges.at(rangeIndex);
	}

	bool isEmpty() const
	{
		return m_ranges.isEmpty();
	}

	int rangesCount() const
	{
		return m_ranges.count();
	}

	int indexesCount() const
	{
		int count = m_ranges.count();

		for(QList<Range>::ConstIterator it = m_ranges.begin(), end2 = m_ranges.end(); it != end2; ++it)
			count += (*it).m_end - (*it).m_start;

		return count;
	}

	Range first() const
	{
		Q_ASSERT(!m_ranges.empty());

		return m_ranges.first();
	}

	Range last() const
	{
		Q_ASSERT(!m_ranges.empty());

		return m_ranges.last();
	}

	int firstIndex() const
	{
		Q_ASSERT(!m_ranges.empty());

		return m_ranges.first().start();
	}

	int lastIndex() const
	{
		Q_ASSERT(!m_ranges.empty());

		return m_ranges.last().end();
	}

	void clear()
	{
		m_ranges.clear();
	}

	void trimToIndex(int index)
	{
		trimToRange(Range::lower(index));
	}

	void trimToRange(const Range &range)
	{
		if(m_ranges.empty())
			return;

		// GENERAL ALGORITHM FOLLOWS
		// - we search the lower and the upper items affected.
		//   - update lower item start and end values
		//   - remove all items below lower up and above upper (lower and upper excluded)

		//       0         1         2
		//     XXXXX     XXXXX     XXXXX           [L, U]
		//  [    ]                                 [0, 0]
		//  []                                     [0, 4]
		//  [         ]                            [0, 0]
		//  [            ]                         [0, 1]
		//      [               ]                  [0, 1]
		//           []                            [1, 3]
		//           [     ]                       [1, 1]
		//                [     ]                  [1, 1]
		//  [                              ]       [0, 2]
		//                            [    ]       [2, 2]
		//                                 [  ]    [3, 2]
		//  [                         ]            [0, 2]

		const int SIZE = m_ranges.count();
		const int LAST_INDEX = SIZE - 1;

		int lowerIndex = SIZE;
		int upperIndex = LAST_INDEX;

		for(int index = 0; index <= LAST_INDEX; ++index) {
			Range &currentRange = m_ranges[index];

			if(lowerIndex == SIZE) {
				if(range.m_start <= currentRange.m_end) {
					lowerIndex = index;
					if(range.m_end < currentRange.m_start) {
						// special case, we must remove all items
						upperIndex = LAST_INDEX + 1;
						break;
					}
				}
			} else {
				if(range.m_end < currentRange.m_start) {
					upperIndex = index - 1;
					break;
				}
			}
		}

		if(lowerIndex > LAST_INDEX || upperIndex > LAST_INDEX) {
			m_ranges.clear();
		} else {
			Range &lowerRange = m_ranges[lowerIndex];
			Range &upperRange = m_ranges[upperIndex];

			if(range.m_start > lowerRange.m_start)
				lowerRange.m_start = range.m_start;
			if(range.m_end < upperRange.m_end)
				upperRange.m_end = range.m_end;

			// remove all items after upperRange
			for(int index = upperIndex + 1; index < SIZE; ++index)
				m_ranges.removeAt(upperIndex + 1);

			// remove all items before lowerRange
			for(int index = 0; index < lowerIndex; ++index)
				m_ranges.removeAt(0);
		}
	}

	void operator<<(const Range &range)
	{
		// first resolve the most common (and easy) cases
		if(m_ranges.empty()) {
			m_ranges.append(range);
			return;
		} else {
			Range &lastRange = m_ranges.last();
			if(lastRange.m_end < range.m_start) {           // range is higher than lastRange
				if(lastRange.m_end + 1 == range.m_start) // range is absorbed by lastRange
					lastRange.m_end = range.m_end;
				else
					m_ranges.append(range);
				return;
			} else {
				Range &firstRange = m_ranges.first();
				if(firstRange.m_start > range.m_end) {          // range is lower than firstRange
					if(range.m_end + 1 == firstRange.m_start) // range is absorbed by firstRange
						firstRange.m_start = range.m_start;
					else
						m_ranges.append(range);
					return;
				}
			}
		}

		// GENERAL ALGORITHM FOLLOWS
		// - we search the lower and the upper items affected.
		//   - update lower item start and end values
		//   - remove all items following lower up to upper (lower excluded)

		//       0         1         2
		//     XXXXX     XXXXX     XXXXX           [L, U]
		//  [    ]                                 [0, 0]
		//  [         ]                            [0, 0]
		//  [            ]                         [0, 1]
		//  [                   ]                  [0, 1]
		//          [ ]                            [0, 0]
		//          [      ]                       [0, 1]
		//  [                              ]       [0, 2]
		//                            [    ]       [2, 2]
		//                                  [ ]    [2, 2] this case isn't correctly handled below but was caught before
		//           []                            [1, 3]

		const int LAST_INDEX = m_ranges.count() - 1;

		int lowerIndex = LAST_INDEX;
		int upperIndex = LAST_INDEX;

		for(int index = 0; index < LAST_INDEX; ++index) {
			Range &currentRange = m_ranges[index];

			if(lowerIndex == LAST_INDEX) {
				if(range.m_start <= currentRange.m_end + 1) {
					lowerIndex = index;
					if(range.m_end < currentRange.m_start - 1) {
						// special case, we must insert a new Range at lowerIndex
						upperIndex = LAST_INDEX + 1;
						break;
					}
				}
			} else {
				if(range.m_end < currentRange.m_start - 1) {
					upperIndex = index - 1;
					break;
				}
			}
		}

		if(upperIndex > LAST_INDEX) {
			// insert new range at lowerIndex
			m_ranges.insert(m_ranges.begin() + lowerIndex, range);
		} else {
			Range &lowerRange = m_ranges[lowerIndex];
			Range &upperRange = m_ranges[upperIndex];

			if(range.m_start < lowerRange.m_start)
				lowerRange.m_start = range.m_start;
			if(range.m_end > upperRange.m_end)
				lowerRange.m_end = range.m_end;
			else
				lowerRange.m_end = upperRange.m_end;

			for(int index = lowerIndex + 1; index <= upperIndex; ++index)
				m_ranges.removeAt(lowerIndex + 1);
		}
	}

	void shiftIndexesForwards(int fromIndex, int delta, bool fillSplitGap)
	{
		Q_ASSERT(fromIndex >= 0);
		Q_ASSERT(delta >= 0);

		if(!delta || m_ranges.isEmpty())
			return;

		for(int index = 0, count = m_ranges.count(); index < count; ++index) {
			Range &range = m_ranges[index];
			if(range.m_start < fromIndex && fromIndex <= range.m_end) {             // range must be filled or split to insert gap
				if(fillSplitGap)
					shiftRangeForwards(range, fromIndex, delta);
				else {
					Range range0(range.m_start, fromIndex - 1);
					range.m_start = fromIndex;
					shiftRangeForwards(range, fromIndex, delta);
					m_ranges.insert(m_ranges.begin() + index, range0);
				}
			} else
				shiftRangeForwards(range, fromIndex, delta);
		}
	}

	void shiftIndexesBackwards(int fromIndex, int delta)
	{
		Q_ASSERT(fromIndex >= 0);
		Q_ASSERT(delta >= 0);

		if(!delta || m_ranges.isEmpty())
			return;

		for(int index = 0, count = m_ranges.count(); index < count; ++index) {
			if(!shiftRangeBackwards(m_ranges[index], fromIndex, delta)) {           // range invalidated by shift
				m_ranges.removeAt(index);
				index--;
				count--;
			}
		}
	}

	QString inspect() const
	{
		QStringList ranges;
		for(int index = 0, count = m_ranges.count(); index < count; ++index)
			ranges += QString().sprintf("[%d,%d]", m_ranges.at(index).m_start, m_ranges.at(index).m_end);
		return ranges.join(", ");
	}

	inline ConstIterator begin() const
	{
		return m_ranges.begin();
	}

	inline ConstIterator end() const
	{
		return m_ranges.end();
	}

private:
	inline void shiftRangeForwards(Range &range, int fromIndex, int delta)
	{
		// Q_ASSERT( delta > 0 );

		if(fromIndex <= range.m_start) {
			range.m_start += delta;
			range.m_end += delta;
		} else if(fromIndex <= range.m_end) {
			range.m_end += delta;
		}
		// else // if ( fromIndex > range.m_end )
		// {
		//  // nothing to do in this case
		// }
	}

	inline bool shiftRangeBackwards(Range &range, int fromIndex, int delta)
	{
		// Q_ASSERT( delta > 0 );

		if(fromIndex <= range.m_start) {
			range.m_start -= delta;
			range.m_end -= delta;
			if(range.m_start < fromIndex) {
				if(range.m_end < fromIndex)
					return false; // range invalidated
				else
					range.m_start = fromIndex;
			}
		} else if(fromIndex <= range.m_end) {
			if(fromIndex + delta > range.m_end)
				range.m_end = fromIndex - 1;
			else
				range.m_end -= delta;
		}
		// else // if ( fromIndex > range.m_end )
		// {
		//  // nothing to do in this case
		// }

		return true;
	}

	QList<Range> m_ranges;
};
}

#endif
