/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SSTRING_H
#define SSTRING_H

#include <cstring>

#include <QColor>
#include <QDataStream>
#include <QDebug>
#include <QList>
#include <QString>

QT_FORWARD_DECLARE_CLASS(QRegExp)
QT_FORWARD_DECLARE_CLASS(QRegularExpression)

namespace SubtitleComposer {
class SString;
class SStringStyle;
}

QDataStream & operator<<(QDataStream &stream, const SubtitleComposer::SString &string);
QDataStream & operator>>(QDataStream &stream, SubtitleComposer::SString &string);

namespace SubtitleComposer {
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

	SString(const QString &string = QString(), quint8 styleFlags = 0, QRgb styleColor = 0);
	SString(const SString &sstring);
	SString & operator=(const SString &sstring);

	~SString();

	inline QString string() const { return *this; }
	void setString(const QString &string, quint8 styleFlags = 0, QRgb styleColor = 0);

	QString richString() const;
	SString & setRichString(const QStringRef &string);
	inline SString & setRichString(const QString &string) { return setRichString(QStringRef(&string)); }

	quint8 styleFlagsAt(int index) const;
	void setStyleFlagsAt(int index, quint8 styleFlags) const;
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
	SString & remove(const QRegExp &rx) { return replace(rx, QString()); }
	SString & remove(const QRegularExpression &rx) { return replace(rx, QString()); }

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
	SString & replace(const QRegularExpression &regExp, const SString &replacement);

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

	inline int length() const { return QString::length(); }

private:
	inline int length(int index, int len) const { return len < 0 || (index + len) > length() ? length() - index : len; }

private:
	friend QDataStream & ::operator<<(QDataStream &stream, const SString &string);
	friend QDataStream & ::operator>>(QDataStream &stream, SString &string);
	friend struct SSHelper;
	SStringStyle *m_style;
};

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

inline bool
SString::operator==(const SString &sstring) const
{
	return !operator!=(sstring);
}

}

#endif
