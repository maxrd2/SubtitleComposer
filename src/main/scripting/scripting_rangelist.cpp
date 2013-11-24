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

#include "scripting_rangelist.h"
#include "scripting_range.h"
#include "../application.h"
#include "../lineswidget.h"

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
Scripting::RangeList::trimToRange(const QObject *object)
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
Scripting::RangeList::addRange(const QObject *object)
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

#include "scripting_rangelist.moc"
