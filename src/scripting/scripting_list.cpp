/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "scripting_list.h"

using namespace SubtitleComposer;

Scripting::List::List(const char *contentClass, QObject *parent) :
	QObject(parent),
	m_contentClass(contentClass)
{}

Scripting::List::List(const QObjectList &backend, const char *contentClass, QObject *parent) :
	QObject(parent),
	m_contentClass(contentClass),
	m_backend(backend)
{}

bool
Scripting::List::isEmpty() const
{
	return m_backend.isEmpty();
}

int
Scripting::List::length() const
{
	return m_backend.count();
}

int
Scripting::List::count() const
{
	return m_backend.count();
}

int
Scripting::List::size() const
{
	return m_backend.size();
}

QObject *
Scripting::List::at(int index) const
{
	if(index < 0 || index >= m_backend.count())
		return 0;
	return m_backend.at(index);
}

void
Scripting::List::insert(int index, QObject *object)
{
	if(!object || index < 0 || index > m_backend.count() || strcmp(object->metaObject()->className(), m_contentClass))
		return;
	m_backend.insert(index, object);
}

void
Scripting::List::append(QObject *object)
{
	if(!object || strcmp(object->metaObject()->className(), m_contentClass))
		return;
	m_backend.append(object);
}

void
Scripting::List::prepend(QObject *object)
{
	if(object && !strcmp(object->metaObject()->className(), m_contentClass))
		m_backend.prepend(object);
}

void
Scripting::List::removeFirst()
{
	if(!m_backend.isEmpty())
		return m_backend.removeFirst();
}

void
Scripting::List::removeLast()
{
	if(!m_backend.isEmpty())
		m_backend.removeLast();
}

void
Scripting::List::removeAt(int index)
{
	if(index >= 0 && index < m_backend.count())
		m_backend.removeAt(index);
}

void
Scripting::List::clear()
{
	m_backend.clear();
}


