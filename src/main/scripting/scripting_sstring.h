#ifndef SCRIPTING_SSTRING_H
#define SCRIPTING_SSTRING_H

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

#include "../../core/sstring.h"
#include "scripting_list.h"

#include <QtCore/QObject>
#include <QtCore/QString>

namespace SubtitleComposer {
namespace Scripting {
class SString : public QObject
{
	Q_OBJECT

public slots:
	bool isEmpty() const;

	int count() const;
	int size() const;
	int length() const;

	QString plainText() const;
	void setPlainText(const QString &string, int styleFlags = 0);

	QString richText() const;
	void setRichText(const QString &string);

	const QChar charAt(int index);
	void setCharAt(int index, const QChar &chr);

	int styleFlagsAt(int index) const;
	void setStyleFlagsAt(int index, int styleFlags) const;

	int cummulativeStyleFlags() const;
	bool hasStyleFlags(int styleFlags) const;

	void setStyleFlags(int index, int len, int styleFlags);
	void setStyleFlags(int index, int len, int styleFlags, bool on);

	void clear();
	void truncate(int size);

	QObject * insert(int index, QObject *str);
	QObject * insertPlain(int index, const QString &str);
	QObject * append(QObject *str);
	QObject * appendPlain(const QString &str);
	QObject * prepend(QObject *str);
	QObject * prependPlain(const QString &str);

	QObject * remove(int index, int len);
	QObject * removeAll(const QString &str, bool regExp = false, bool caseSensitive = true);

	QObject * replace(int index, int len, QObject *replacement);
	QObject * replacePlain(int index, int len, const QString &replacement);
	QObject * replaceAll(const QString &before, QObject *after, bool regExp = false, bool caseSensitive = true);
	QObject * replaceAllPlain(const QString &before, const QString &after, bool regExp = false, bool caseSensitive = true);

	List * split(const QString &sep, bool regExp, bool caseSensitive = true) const;

	QObject * left(int len) const;
	QObject * right(int len) const;
	QObject * mid(int index, int len = -1) const;

	QObject * toLower() const;
	QObject * toUpper() const;

	QObject * simplified() const;
	QObject * trimmed() const;

	int compareTo(QObject *string) const;
	int compareToPlain(const QString &string) const;

private:
	friend class StringsModule;
	friend class SubtitleLine;

	SString(const SubtitleComposer::SString &backend, QObject *parent);

	SubtitleComposer::SString m_backend;
};
}
}
#endif
