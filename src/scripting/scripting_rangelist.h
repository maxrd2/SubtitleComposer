#ifndef SCRIPTING_RANGELIST_H
#define SCRIPTING_RANGELIST_H

/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "core/rangelist.h"

#include <QObject>

namespace SubtitleComposer {
namespace Scripting {
class RangeList : public QObject
{
	Q_OBJECT

public slots:
	QObject * complement() const;

	bool isEmpty() const;

	int rangesCount() const;
	int indexesCount() const;

	int firstIndex() const;
	int lastIndex() const;

	QObject * range(int rangeIndex) const;

	bool contains(int index) const;

	void clear();

	void trimToIndex(int index);
	void trimToRange(const QObject *range);

	QObject * addIndex(int index);
	QObject * addRange(const QObject *range);

	void shiftIndexesForwards(int fromIndex, int delta, bool fillSplitGap);
	void shiftIndexesBackwards(int fromIndex, int delta);

private:
	friend class RangesModule;
	friend class Subtitle;

	RangeList(const SubtitleComposer::RangeList &backend, QObject *parent);

	SubtitleComposer::RangeList m_backend;
};
}
}
#endif
