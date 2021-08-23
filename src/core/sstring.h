/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SSTRING_H
#define SSTRING_H

#include <string.h>

#include <QString>
#include <QRegExp>
#include <QList>
#include <QColor>
#include <QDataStream>

#include <QDebug>

void * memset_n(void *ptr, int value, size_t length, size_t size);

namespace SubtitleComposer {
class SString;

class SStringList : public QList<SString>
{
public:
	SStringList();
	SStringList(const SString &str);
	SStringList(const SStringList &list);
	SStringList(const QList<SString> &list);
	SStringList(const QStringList &list);
	SStringList(const QList<QString> &list);

	SString join(const SString &sep) const;

	SStringList& operator=(const SStringList& other) = default;
};

class SString : public QString
{
public:
	typedef enum {
		Bold = 0x1,
		Italic = 0x2,
		Underline = 0x4,
		StrikeThrough = 0x8,
		Color = 0x10,
		AllStyles = Bold | Italic | Underline | StrikeThrough | Color
	} StyleFlag;

	typedef enum {
		Compact,
		Verbose
	} RichOutputMode;

	SString(const QString &string = QString(), int styleFlags = 0, QRgb styleColor = 0);            // krazy:exclude=c++/explicit
	SString(const SString &sstring);
	SString & operator=(const SString &sstring);

	~SString();

	inline QString string() const { return *this; }
	void setString(const QString &string, int styleFlags = 0, QRgb styleColor = 0);         // always clears all the style flags

	QString richString(RichOutputMode mode = Compact) const;
	SString & setRichString(const QString &string);

	int styleFlagsAt(int index) const;
	void setStyleFlagsAt(int index, int styleFlags) const;
	QRgb styleColorAt(int index) const;
	void setStyleColorAt(int index, QRgb rgbColor) const;

	int cummulativeStyleFlags() const;
	bool hasStyleFlags(int styleFlags) const;
	SString & setStyleFlags(int index, int len, int styleFlags);
	SString & setStyleFlags(int index, int len, int styleFlags, bool on);
	SString & setStyleColor(int index, int len, QRgb color);

	void clear();

	SString & insert(int index, QChar ch);
	SString & insert(int index, const QString &str);
	SString & insert(int index, const SString &str);
	SString & append(QChar ch);
	SString & append(const QString &str);
	SString & append(const SString &str);
	SString & prepend(QChar ch);
	SString & prepend(const QString &str);
	SString & prepend(const SString &str);
	SString & operator+=(QChar ch);
	SString & operator+=(const QString &str);
	SString & operator+=(const SString &str);

	SString & remove(int index, int len);
	SString & remove(const QString &str, Qt::CaseSensitivity cs = Qt::CaseSensitive);
	SString & remove(QChar ch, Qt::CaseSensitivity cs = Qt::CaseSensitive);
	SString & remove(const QRegExp &rx);

	SString & replace(int index, int len, const QString &replacement);
	SString & replace(int index, int len, const SString &replacement);
	SString & replace(const QString &before, const QString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
	SString & replace(const QString &before, const SString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
	SString & replace(QChar before, QChar after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
	SString & replace(QChar ch, const QString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
	SString & replace(QChar ch, const SString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
	SString & replace(const QRegExp &regExp, const QString &replacement);
	SString & replace(const QRegExp &regExp, const SString &replacement);
	SString & replace(const QRegularExpression &regExp, const QString &replacement);
	SString & replace(const QRegularExpression &regExp, const SString &replacement, bool styleFromReplacement=true);

	SStringList split(const QString &sep, QString::SplitBehavior behavior = QString::KeepEmptyParts, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
	SStringList split(const QChar &sep, QString::SplitBehavior behavior = QString::KeepEmptyParts, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
	SStringList split(const QRegExp &sep, QString::SplitBehavior behavior = QString::KeepEmptyParts) const;

	SString left(int len) const;
	SString right(int len) const;
	SString mid(int index, int len = -1) const;

	SString toLower() const;
	SString toUpper() const;
	SString toTitleCase(bool lowerFirst) const;
	SString toSentenceCase(bool lowerFirst, bool *cont) const;

	SString simplified() const;
	SString trimmed() const;

	static void simplifyWhiteSpace(QString &text);
	void simplifyWhiteSpace();

	bool operator==(const SString &sstring) const;
	bool operator!=(const SString &sstring) const;

	friend inline QDataStream & operator<<(QDataStream &stream, const SString &string);
	friend inline QDataStream & operator>>(QDataStream &stream, SString &string);

	inline int length() const { return QString::length(); }

private:
	char * detachFlags();
	QRgb * detachColors();
	void setMinFlagsCapacity(int capacity);

	int length(int index, int len) const;

private:
	char *m_styleFlags;
	QRgb *m_styleColors;
	int m_capacity;
};

inline int
SString::styleFlagsAt(int index) const
{
	return index < 0 || index >= length() ? 0 : m_styleFlags[index];
}

inline void
SString::setStyleFlagsAt(int index, int styleFlags) const
{
	if(index < 0 || index >= length())
		return;
	m_styleFlags[index] = styleFlags;
}

inline QRgb
SString::styleColorAt(int index) const
{
	return index < 0 || index >= length() || (m_styleFlags[index] & SString::Color) == 0 ? 0 : m_styleColors[index];
}

inline void
SString::setStyleColorAt(int index, QRgb rgbColor) const
{
	if(index < 0 || index >= length())
		return;
	if(rgbColor == 0)
		m_styleFlags[index] &= ~SString::Color;
	else
		m_styleFlags[index] |= SString::Color;
	m_styleColors[index] = rgbColor;
}

inline SString &
SString::append(QChar ch)
{
	return insert(length(), ch);
}

inline SString &
SString::append(const QString &str)
{
	return insert(length(), str);
}

inline SString &
SString::append(const SString &str)
{
	return insert(length(), str);
}

inline SString &
SString::prepend(QChar ch)
{
	return insert(0, ch);
}

inline SString &
SString::prepend(const QString &str)
{
	return insert(0, str);
}

inline SString &
SString::prepend(const SString &str)
{
	return insert(0, str);
}

inline SString &
SString::operator+=(const QChar ch)
{
	return append(ch);
}

inline SString &
SString::operator+=(const QString &str)
{
	return append(str);
}

inline SString &
SString::operator+=(const SString &str)
{
	return append(str);
}

inline SString &
SString::remove(int index, int len)
{
	return replace(index, len, QString());
}

inline SString &
SString::remove(const QString &str, Qt::CaseSensitivity cs)
{
	return replace(str, QString(), cs);
}

inline SString &
SString::remove(QChar ch, Qt::CaseSensitivity cs)
{
	return replace(ch, QString(), cs);
}

inline SString &
SString::remove(const QRegExp &rx)
{
	return replace(rx, QString());
}

inline bool
SString::operator==(const SString &sstring) const
{
	return !operator!=(sstring);
}

inline int
SString::length(int index, int len) const
{
	return len < 0 || (index + len) > length() ? length() - index : len;
}

inline QDataStream &
operator<<(QDataStream &stream, const SubtitleComposer::SString &string) {
	stream << static_cast<const QString &>(string);
	stream.writeRawData(string.m_styleFlags, string.length() * sizeof(*string.m_styleFlags));
	stream.writeRawData(reinterpret_cast<const char *>(string.m_styleColors), string.length() * sizeof(*string.m_styleColors));

	return stream;
}

inline QDataStream &
operator>>(QDataStream &stream, SubtitleComposer::SString &string) {
	stream >> static_cast<QString &>(string);
	string.setMinFlagsCapacity(string.size());
	stream.readRawData(string.m_styleFlags, string.length() * sizeof(*string.m_styleFlags));
	stream.readRawData(reinterpret_cast<char *>(string.m_styleColors), string.length() * sizeof(*string.m_styleColors));

	return stream;
}
}

#endif
