/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SUBTITLEITERATOR_H
#define SUBTITLEITERATOR_H

#include "core/range.h"
#include "core/rangelist.h"
#include "core/subtitle.h"

#include <QExplicitlySharedDataPointer>
#include <QObject>
#include <QList>

namespace SubtitleComposer {
class SubtitleLine;
class Subtitle;
class SubtitleIterator : public QObject
{
	Q_OBJECT
	Q_PROPERTY(int index READ index WRITE toIndex)

public:
	enum {
		AfterLast = -1,
		BehindFirst = -2,
		Invalid = -3
	};

	explicit SubtitleIterator(const Subtitle &subtitle, const RangeList &ranges = Range::full(), bool toLast = false);
	SubtitleIterator(const SubtitleIterator &it);
	SubtitleIterator & operator=(const SubtitleIterator &it);
	virtual ~SubtitleIterator();

	RangeList ranges() const;

	void toFirst();
	void toLast();
	bool toIndex(int index);

	inline int index() { return m_index; }
	inline int firstIndex() { return m_index == Invalid ? -1 : m_ranges.firstIndex(); }
	inline int lastIndex() { return m_index == Invalid ? -1 : m_ranges.lastIndex(); }

	inline SubtitleLine * current() const { return const_cast<SubtitleLine *>(m_subtitle->line(m_index)); }
	inline operator SubtitleLine *() const { return const_cast<SubtitleLine *>(m_subtitle->line(m_index)); }

	SubtitleIterator & operator++();
	SubtitleIterator & operator+=(int steps);
	SubtitleIterator & operator--();
	SubtitleIterator & operator-=(int steps);

private:
	QExplicitlySharedDataPointer<const Subtitle> m_subtitle;
	RangeList m_ranges;
	int m_index;
	RangeList::ConstIterator m_rangesIterator;
};
}

#endif
