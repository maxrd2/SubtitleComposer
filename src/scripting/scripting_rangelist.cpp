/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "scripting_rangelist.h"
#include "scripting_range.h"
#include "application.h"
#include "gui/treeview/lineswidget.h"

using namespace SubtitleComposer;

Scripting::RangeList::RangeList(const SubtitleComposer::RangeList &backend, QObject *parent) :
	QObject(parent),
	m_backend(backend)
{}

QObject *
Scripting::RangeList::complement() const
{
	return new Scripting::RangeList(m_backend.complement(), const_cast<Scripting::RangeList *>(this));
}

QObject *
Scripting::RangeList::range(int rangeIndex) const
{
	if(rangeIndex < 0 || rangeIndex >= m_backend.rangesCount())
		return 0;
	return new Scripting::Range(m_backend.range(rangeIndex), const_cast<Scripting::RangeList *>(this));
}

bool
Scripting::RangeList::contains(int index) const
{
	return m_backend.contains(index);
}

bool
Scripting::RangeList::isEmpty() const
{
	return m_backend.isEmpty();
}

int
Scripting::RangeList::rangesCount() const
{
	return m_backend.rangesCount();
}

int
Scripting::RangeList::indexesCount() const
{
	return m_backend.indexesCount();
}

int
Scripting::RangeList::firstIndex() const
{
	return m_backend.firstIndex();
}

int
Scripting::RangeList::lastIndex() const
{
	return m_backend.lastIndex();
}

void
Scripting::RangeList::clear()
{
	m_backend.clear();
}

void
Scripting::RangeList::trimToIndex(int index)
{
	m_backend.trimToIndex(index);
}

void
Scripting::RangeList::trimToRange(QObject *object)
{
	if(const Scripting::Range * range = qobject_cast<const Scripting::Range *>(object))
		m_backend.trimToRange(range->m_backend);
}

QObject *
Scripting::RangeList::addIndex(int index)
{
	if(index >= 0)
		m_backend << SubtitleComposer::Range(index);
	return this;
}

QObject *
Scripting::RangeList::addRange(QObject *object)
{
	if(const Scripting::Range * range = qobject_cast<const Scripting::Range *>(object))
		m_backend << range->m_backend;
	return this;
}

void
Scripting::RangeList::shiftIndexesForwards(int fromIndex, int delta, bool fillSplitGap)
{
	if(fromIndex >= 0 && delta > 0)
		m_backend.shiftIndexesForwards(fromIndex, delta, fillSplitGap);
}

void
Scripting::RangeList::shiftIndexesBackwards(int fromIndex, int delta)
{
	if(fromIndex >= 0 && delta > 0)
		m_backend.shiftIndexesBackwards(fromIndex, delta);
}


