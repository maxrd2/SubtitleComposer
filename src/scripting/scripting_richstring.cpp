/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "scripting_richstring.h"
#include "scripting_list.h"

#include <QRegularExpression>

using namespace SubtitleComposer;

Scripting::RichString::RichString(const SubtitleComposer::RichString &backend, QObject *parent) :
	QObject(parent),
	m_backend(backend)
{}

bool
Scripting::RichString::isEmpty() const
{
	return m_backend.isEmpty();
}

int
Scripting::RichString::length() const
{
	return m_backend.length();
}

int
Scripting::RichString::count() const
{
	return m_backend.length();
}

int
Scripting::RichString::size() const
{
	return m_backend.length();
}

QString
Scripting::RichString::plainText() const
{
	return m_backend.string();
}

void
Scripting::RichString::setPlainText(const QString &string, int styleFlags)
{
	m_backend.setString(string, styleFlags);
}

QString
Scripting::RichString::richText() const
{
	return m_backend.richString();
}

void
Scripting::RichString::setRichText(const QString &string)
{
	m_backend.setRichString(string);
}

const QChar
Scripting::RichString::charAt(int index)
{
	if(index < 0 || index >= m_backend.size())
		return '\0';
	return m_backend.at(index);
}

void
Scripting::RichString::setCharAt(int index, const QChar &chr)
{
	if(index >= 0 && index < m_backend.size())
		m_backend[index] = chr;
}

int
Scripting::RichString::styleFlagsAt(int index) const
{
	if(index < 0 || index >= m_backend.size())
		return 0;
	return m_backend.styleFlagsAt(index);
}

void
Scripting::RichString::setStyleFlagsAt(int index, int styleFlags) const
{
	if(index >= 0 && index < m_backend.size())
		m_backend.setStyleFlagsAt(index, styleFlags);
}

int
Scripting::RichString::cummulativeStyleFlags() const
{
	return m_backend.cummulativeStyleFlags();
}

bool
Scripting::RichString::hasStyleFlags(int styleFlags) const
{
	return m_backend.hasStyleFlags(styleFlags);
}

void
Scripting::RichString::setStyleFlags(int index, int len, int styleFlags)
{
	if(index < 0 || index >= m_backend.size())
		return;
	m_backend.setStyleFlags(index, len, styleFlags);
}

void
Scripting::RichString::setStyleFlags(int index, int len, int styleFlags, bool on)
{
	if(index < 0 || index >= m_backend.size())
		return;
	m_backend.setStyleFlags(index, len, styleFlags, on);
}

void
Scripting::RichString::clear()
{
	m_backend.clear();
}

void
Scripting::RichString::truncate(int size)
{
	if(size >= 0)
		m_backend.truncate(size);
}

QObject *
Scripting::RichString::insert(int index, QObject *object)
{
	if(index >= 0 && index <= m_backend.size()) {
		if(const Scripting::RichString * str = qobject_cast<const Scripting::RichString *>(object))
			m_backend.insert(index, str->m_backend);
	}
	return this;
}

QObject *
Scripting::RichString::insertPlain(int index, const QString &str)
{
	if(index >= 0 && index <= m_backend.size())
		m_backend.insert(index, str);
	return this;
}

QObject *
Scripting::RichString::append(QObject *object)
{
	if(const Scripting::RichString * str = qobject_cast<const Scripting::RichString *>(object))
		m_backend.append(str->m_backend);
	return this;
}

QObject *
Scripting::RichString::appendPlain(const QString &str)
{
	m_backend.append(str);
	return this;
}

QObject *
Scripting::RichString::prepend(QObject *object)
{
	if(const Scripting::RichString * str = qobject_cast<const Scripting::RichString *>(object))
		m_backend.prepend(str->m_backend);
	return this;
}

QObject *
Scripting::RichString::prependPlain(const QString &str)
{
	m_backend.prepend(str);
	return this;
}

QObject *
Scripting::RichString::remove(int index, int len)
{
	if(index >= 0 && index < m_backend.size())
		m_backend.remove(index, len);
	return this;
}

QObject *
Scripting::RichString::removeAll(const QString &str, bool regExp, bool caseSensitive)
{
	if(regExp)
		m_backend.remove(QRegularExpression(str, caseSensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption));
	else
		m_backend.remove(str, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
	return this;
}

QObject *
Scripting::RichString::replace(int index, int len, QObject *object)
{
	if(const Scripting::RichString * replacement = qobject_cast<const Scripting::RichString *>(object)) {
		if(index >= 0 && index < m_backend.size())
			m_backend.replace(index, len, replacement->m_backend);
	}
	return this;
}

QObject *
Scripting::RichString::replacePlain(int index, int len, const QString &replacement)
{
	if(index >= 0 && index < m_backend.size())
		m_backend.replace(index, len, replacement);
	return this;
}

QObject *
Scripting::RichString::replaceAll(const QString &before, QObject *object, bool regExp, bool caseSensitive)
{
	if(const Scripting::RichString * after = qobject_cast<const Scripting::RichString *>(object)) {
		if(regExp)
			m_backend.replace(QRegularExpression(before, caseSensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption), after->m_backend);
		else
			m_backend.replace(before, after->m_backend, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
	}
	return this;
}

QObject *
Scripting::RichString::replaceAllPlain(const QString &before, const QString &after, bool regExp, bool caseSensitive)
{
	if(regExp)
		m_backend.replace(QRegularExpression(before, caseSensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption), after);
	else
		m_backend.replace(before, after, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
	return this;
}

Scripting::List *
Scripting::RichString::split(const QString &sep, bool regExp, bool caseSensitive) const
{
	RichStringList tokens;
	if(regExp)
		tokens = m_backend.split(QRegularExpression(sep, caseSensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption), Qt::KeepEmptyParts);
	else
		tokens = m_backend.split(sep, Qt::KeepEmptyParts, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);

	Scripting::RichString *parent = const_cast<Scripting::RichString *>(this);

	QList<QObject *> list;
	for(RichStringList::ConstIterator it = tokens.constBegin(), end = tokens.constEnd(); it != end; ++it)
		list.append(new Scripting::RichString(*it, parent));

	return new Scripting::List(list, Scripting::RichString::staticMetaObject.className(), parent);
}

QObject *
Scripting::RichString::left(int len) const
{
	return new RichString(m_backend.left(len), const_cast<Scripting::RichString *>(this));
}

QObject *
Scripting::RichString::right(int len) const
{
	return new RichString(m_backend.right(len), const_cast<Scripting::RichString *>(this));
}

QObject *
Scripting::RichString::mid(int index, int len) const
{
	if(index < 0 || index >= m_backend.size())
		return new RichString(SubtitleComposer::RichString(), const_cast<Scripting::RichString *>(this));
	return new RichString(m_backend.mid(index, len), const_cast<Scripting::RichString *>(this));
}

QObject *
Scripting::RichString::toLower() const
{
	return new RichString(m_backend.toLower(), const_cast<Scripting::RichString *>(this));
}

QObject *
Scripting::RichString::toUpper() const
{
	return new RichString(m_backend.toUpper(), const_cast<Scripting::RichString *>(this));
}

QObject *
Scripting::RichString::simplified() const
{
	return new RichString(m_backend.simplified(), const_cast<Scripting::RichString *>(this));
}

QObject *
Scripting::RichString::trimmed() const
{
	return new RichString(m_backend.trimmed(), const_cast<Scripting::RichString *>(this));
}

int
Scripting::RichString::compareTo(QObject *object) const
{
	const Scripting::RichString *string = qobject_cast<const Scripting::RichString *>(object);
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
Scripting::RichString::compareToPlain(const QString &string) const
{
	if(static_cast<const QString &>(m_backend) < string)
		return -1;
	else if(static_cast<const QString &>(m_backend) == string)
		return 0;
	else
		return 1;
}


