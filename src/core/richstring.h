/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RICHSTRING_H
#define RICHSTRING_H

#include <cstring>

#include <QColor>
#include <QDataStream>
#include <QDebug>
#include <QList>
#include <QString>

QT_FORWARD_DECLARE_CLASS(QRegularExpression)

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
namespace Qt {
enum SplitBehaviorFlags {
	KeepEmptyParts = 0,
	SkipEmptyParts = 0x1,
};
Q_DECLARE_FLAGS(SplitBehavior, SplitBehaviorFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(SplitBehavior)
}
#endif

namespace SubtitleComposer {
class RichString;
class RichStringStyle;
}

QDataStream & operator<<(QDataStream &stream, const SubtitleComposer::RichString &string);
QDataStream & operator>>(QDataStream &stream, SubtitleComposer::RichString &string);

namespace SubtitleComposer {
class RichStringList : public QList<RichString>
{
public:
	RichStringList();
	RichStringList(const RichString &str);
	RichStringList(const RichStringList &list);
	RichStringList(const QList<RichString> &list);
	RichStringList(const QStringList &list);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	RichStringList(const QList<QString> &list);
#endif

	RichString join(const RichString &sep) const;

	RichStringList& operator=(const RichStringList& other) = default;
};

class RichString : public QString
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

	using StyleFlags = quint8;

	RichString(const QString &string = QString(), quint8 styleFlags = 0, QRgb styleColor = 0, const QSet<QString> &classList=QSet<QString>(), const QString &voice=QString());
	RichString(const RichString &richstring);
	RichString & operator=(const RichString &richstring);

	~RichString();

	inline QString string() const { return *this; }
	void setString(const QString &string, quint8 styleFlags = 0, QRgb styleColor = 0, const QString klass=QString(), const QString voice=QString());

	QString richString() const;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	RichString & setRichString(const QStringRef &string);
	inline RichString & setRichString(const QString &string) { return setRichString(QStringRef(&string)); }
#else
	RichString & setRichString(const QStringView &string);
	inline RichString & setRichString(const QString &string) { return setRichString(QStringView(string)); }
#endif

	StyleFlags styleFlagsAt(int index) const;
	void setStyleFlagsAt(int index, StyleFlags styleFlags) const;
	StyleFlags cummulativeStyleFlags() const;
	bool hasStyleFlags(quint8 styleFlags) const;
	RichString & setStyleFlags(int index, int len, StyleFlags styleFlags);
	RichString & setStyleFlags(int index, int len, StyleFlags styleFlags, bool on);

	QRgb styleColorAt(int index) const;
	void setStyleColorAt(int index, QRgb rgbColor) const;
	QSet<QRgb> cummulativeColors() const;
	RichString & setStyleColor(int index, int len, QRgb color);

	quint64 styleClassesAt(int index) const;
	QSet<QString> styleClassNamesAt(int index) const;
	QSet<QString> cummulativeClasses() const;
	void setStyleClassesAt(int index, quint64 classes) const;
	void setStyleClassNamesAt(int index, const QSet<QString> &classes) const;

	qint32 styleVoiceAt(int index) const;
	QString styleVoiceNameAt(int index) const;
	QSet<QString> cummulativeVoices() const;
	void setStyleVoiceAt(int index, qint32 voice) const;
	void setStyleVoiceNameAt(int index, const QString &voice) const;

	void clear();

	RichString & insert(int index, QChar ch);
	RichString & insert(int index, const QString &str);
	RichString & insert(int index, const RichString &str);

	inline RichString & append(QChar ch) { return insert(length(), ch); }
	inline RichString & append(const QString &str) { return insert(length(), str); }
	inline RichString & append(const RichString &str) { return insert(length(), str); }
	inline RichString & prepend(QChar ch) { return insert(0, ch); }
	inline RichString & prepend(const QString &str) { return insert(0, str); }
	inline RichString & prepend(const RichString &str) { return insert(0, str); }
	inline RichString & operator+=(QChar ch) { return append(ch); }
	inline RichString & operator+=(const QString &str) { return append(str); }
	inline RichString & operator+=(const RichString &str) { return append(str); }

	inline RichString & remove(int index, int len) { return replace(index, len, QString()); }
	inline RichString & remove(const QString &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) { return replace(str, QString(), cs); }
	inline RichString & remove(QChar ch, Qt::CaseSensitivity cs = Qt::CaseSensitive) { return replace(ch, QString(), cs); }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	RichString & remove(const QRegExp &rx) = delete;
#endif
	RichString & remove(const QRegularExpression &rx) { return replace(rx, QString()); }

	RichString & replace(int index, int len, const QString &replacement);
	RichString & replace(int index, int len, const RichString &replacement);
	RichString & replace(const QString &before, const QString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
	RichString & replace(const QString &before, const RichString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
	RichString & replace(QChar before, QChar after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
	RichString & replace(QChar ch, const QString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
	RichString & replace(QChar ch, const RichString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	RichString & replace(const QRegExp &regExp, const QString &replacement) = delete;
	RichString & replace(const QRegExp &regExp, const RichString &replacement) = delete;
#endif
	RichString & replace(const QRegularExpression &regExp, const QString &replacement);
	RichString & replace(const QRegularExpression &regExp, const RichString &replacement);

	RichStringList split(const QString &sep, Qt::SplitBehaviorFlags behavior = Qt::KeepEmptyParts, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
	RichStringList split(const QChar &sep, Qt::SplitBehaviorFlags behavior = Qt::KeepEmptyParts, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
	RichStringList split(const QRegularExpression &sep, Qt::SplitBehaviorFlags behavior = Qt::KeepEmptyParts) const;

	RichString left(int len) const;
	RichString right(int len) const;
	RichString mid(int index, int len = -1) const;

	RichString toLower() const;
	RichString toUpper() const;
	RichString toTitleCase(bool lowerFirst) const;
	RichString toSentenceCase(bool lowerFirst, bool *cont) const;

	RichString simplified() const;
	RichString trimmed() const;

	static void simplifyWhiteSpace(QString &text);
	void simplifyWhiteSpace();

	inline bool operator==(const RichString &richstring) const { return !operator!=(richstring); }
	bool operator!=(const RichString &richstring) const;

	inline int length() const { return QString::length(); }

private:
	inline int length(int index, int len) const { return len < 0 || (index + len) > length() ? length() - index : len; }

private:
	friend QDataStream & ::operator<<(QDataStream &stream, const RichString &string);
	friend QDataStream & ::operator>>(QDataStream &stream, RichString &string);
	friend struct ReplaceHelper;
	RichStringStyle *m_style;
};

}

#endif
