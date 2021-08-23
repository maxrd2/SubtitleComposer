/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "scripting_range.h"

using namespace SubtitleComposer;

/// RANGE IMPLEMENTATION
/// ====================

Scripting::Range::Range(const SubtitleComposer::Range &range, QObject *parent) :
	QObject(parent),
	m_backend(range)
{}

int
Scripting::Range::start() const
{
	return m_backend.start();
}

int
Scripting::Range::end() const
{
	return m_backend.end();
}

int
Scripting::Range::length() const
{
	return m_backend.length();
}

bool
Scripting::Range::contains(int index) const
{
	return m_backend.contains(index);
}

bool
Scripting::Range::contains(const QObject *object) const
{
	if(const Scripting::Range * range = qobject_cast<const Scripting::Range *>(object))
		return m_backend.contains(range->m_backend);
	else
		return false;
}


