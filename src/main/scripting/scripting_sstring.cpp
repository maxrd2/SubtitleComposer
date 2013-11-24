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

#include "scripting_sstring.h"

#include <QtCore/QRegExp>

using namespace SubtitleComposer;

Scripting::SString::SString(const SubtitleComposer::SString &backend, QObject *parent) :
	QObject(parent),
	m_backend(backend)
{}

bool
Scripting::SString::isEmpty() const
{
	return m_backend.isEmpty();
}

int
Scripting::SString::length() const
{
	return m_backend.length();
}

int
Scripting::SString::count() const
{
	return m_backend.length();
}

int
Scripting::SString::size() const
{
	return m_backend.length();
}

QString
Scripting::SString::plainText() const
{
	return m_backend.string();
}

void
Scripting::SString::setPlainText(const QString &string, int styleFlags)
{
	m_backend.setString(string, styleFlags);
}

QString
Scripting::SString::richText() const
{
	return m_backend.richString();
}

void
Scripting::SString::setRichText(const QString &string)
{
	m_backend.setRichString(string);
}

const QChar
Scripting::SString::charAt(int index)
{
	if(index < 0 || index >= m_backend.count())
		return '\0';
	return m_backend.at(index);
}

void
Scripting::SString::setCharAt(int index, const QChar &chr)
{
	if(index >= 0 && index < m_backend.count())
		m_backend[index] = chr;
}

int
Scripting::SString::styleFlagsAt(int index) const
{
	if(index < 0 || index >= m_backend.count())
		return 0;
	return m_backend.styleFlagsAt(index);
}

void
Scripting::SString::setStyleFlagsAt(int index, int styleFlags) const
{
	if(index >= 0 && index < m_backend.count())
		m_backend.setStyleFlagsAt(index, styleFlags);
}

int
Scripting::SString::cummulativeStyleFlags() const
{
	return m_backend.cummulativeStyleFlags();
}

bool
Scripting::SString::hasStyleFlags(int styleFlags) const
{
	return m_backend.hasStyleFlags(styleFlags);
}

void
Scripting::SString::setStyleFlags(int index, int len, int styleFlags)
{
	if(index < 0 || index >= m_backend.count())
		return;
	m_backend.setStyleFlags(index, len, styleFlags);
}

void
Scripting::SString::setStyleFlags(int index, int len, int styleFlags, bool on)
{
	if(index < 0 || index >= m_backend.count())
		return;
	m_backend.setStyleFlags(index, len, styleFlags, on);
}

void
Scripting::SString::clear()
{
	m_backend.clear();
}

void
Scripting::SString::truncate(int size)
{
	if(size >= 0)
		m_backend.truncate(size);
}

QObject *
Scripting::SString::insert(int index, QObject *object)
{
	if(index >= 0 && index <= m_backend.count()) {
		if(const Scripting::SString * str = qobject_cast<const Scripting::SString *>(object))
			m_backend.insert(index, str->m_backend);
	}
	return this;
}

QObject *
Scripting::SString::insertPlain(int index, const QString &str)
{
	if(index >= 0 && index <= m_backend.count())
		m_backend.insert(index, str);
	return this;
}

QObject *
Scripting::SString::append(QObject *object)
{
	if(const Scripting::SString * str = qobject_cast<const Scripting::SString *>(object))
		m_backend.append(str->m_backend);
	return this;
}

QObject *
Scripting::SString::appendPlain(const QString &str)
{
	m_backend.append(str);
	return this;
}

QObject *
Scripting::SString::prepend(QObject *object)
{
	if(const Scripting::SString * str = qobject_cast<const Scripting::SString *>(object))
		m_backend.prepend(str->m_backend);
	return this;
}

QObject *
Scripting::SString::prependPlain(const QString &str)
{
	m_backend.prepend(str);
	return this;
}

QObject *
Scripting::SString::remove(int index, int len)
{
	if(index >= 0 && index < m_backend.count())
		m_backend.remove(index, len);
	return this;
}

QObject *
Scripting::SString::removeAll(const QString &str, bool regExp, bool caseSensitive)
{
	if(regExp)
		m_backend.remove(QRegExp(str, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive));
	else
		m_backend.remove(str, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
	return this;
}

QObject *
Scripting::SString::replace(int index, int len, QObject *object)
{
	if(const Scripting::SString * replacement = qobject_cast<const Scripting::SString *>(object)) {
		if(index >= 0 && index < m_backend.count())
			m_backend.replace(index, len, replacement->m_backend);
	}
	return this;
}

QObject *
Scripting::SString::replacePlain(int index, int len, const QString &replacement)
{
	if(index >= 0 && index < m_backend.count())
		m_backend.replace(index, len, replacement);
	return this;
}

QObject *
Scripting::SString::replaceAll(const QString &before, QObject *object, bool regExp, bool caseSensitive)
{
	if(const Scripting::SString * after = qobject_cast<const Scripting::SString *>(object)) {
		if(regExp)
			m_backend.replace(QRegExp(before, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive), after->m_backend);
		else
			m_backend.replace(before, after->m_backend, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
	}
	return this;
}

QObject *
Scripting::SString::replaceAllPlain(const QString &before, const QString &after, bool regExp, bool caseSensitive)
{
	if(regExp)
		m_backend.replace(QRegExp(before, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive), after);
	else
		m_backend.replace(before, after, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
	return this;
}

Scripting::List *
Scripting::SString::split(const QString &sep, bool regExp, bool caseSensitive) const
{
	SStringList tokens;
	if(regExp)
		tokens = m_backend.split(QRegExp(sep, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive), QString::KeepEmptyParts);
	else
		tokens = m_backend.split(sep, QString::KeepEmptyParts, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);

	Scripting::SString *parent = const_cast<Scripting::SString *>(this);

	QList<QObject *> list;
	for(SStringList::ConstIterator it = tokens.begin(), end = tokens.end(); it != end; ++it)
		list.append(new Scripting::SString(*it, parent));

	return new Scripting::List(list, Scripting::SString::staticMetaObject.className(), parent);
}

QObject *
Scripting::SString::left(int len) const
{
	return new SString(m_backend.left(len), const_cast<Scripting::SString *>(this));
}

QObject *
Scripting::SString::right(int len) const
{
	return new SString(m_backend.right(len), const_cast<Scripting::SString *>(this));
}

QObject *
Scripting::SString::mid(int index, int len) const
{
	if(index < 0 || index >= m_backend.count())
		return new SString(SubtitleComposer::SString(), const_cast<Scripting::SString *>(this));
	return new SString(m_backend.mid(index, len), const_cast<Scripting::SString *>(this));
}

QObject *
Scripting::SString::toLower() const
{
	return new SString(m_backend.toLower(), const_cast<Scripting::SString *>(this));
}

QObject *
Scripting::SString::toUpper() const
{
	return new SString(m_backend.toUpper(), const_cast<Scripting::SString *>(this));
}

QObject *
Scripting::SString::simplified() const
{
	return new SString(m_backend.simplified(), const_cast<Scripting::SString *>(this));
}

QObject *
Scripting::SString::trimmed() const
{
	return new SString(m_backend.trimmed(), const_cast<Scripting::SString *>(this));
}

int
Scripting::SString::compareTo(QObject *object) const
{
	const Scripting::SString *string = qobject_cast<const Scripting::SString *>(object);
	if(!string)
		return -2;

	if(m_backend < string->m_backend)
		return -1;
	else if(m_backend == string->m_backend)
		return 0;
	else
		return 1;
}

int
Scripting::SString::compareToPlain(const QString &string) const
{
	if(m_backend < string)
		return -1;
	else if(m_backend == string)
		return 0;
	else
		return 1;
}

#include "scripting_sstring.moc"
