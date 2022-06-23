/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "core/richstring.h"

#include "helpers/common.h"

#include <QList>
#include <QStringList>
#include <QRegularExpression>

#include <QDebug>

#include <QColor>

#include <type_traits>

using namespace SubtitleComposer;

template<typename T>
inline void
memset_n(T *dest, const T &value, size_t count)
{
	if(sizeof(T) == 1) {
		memset(dest, value, count);
	} else {
		while(count-- > 0)
			*dest++ = value;
	}
}

namespace SubtitleComposer {
class RichStringStyle {
	friend QDataStream & ::operator<<(QDataStream &stream, const SubtitleComposer::RichString &string);
	friend QDataStream & ::operator>>(QDataStream &stream, SubtitleComposer::RichString &string);

	class Iterator {
	public:
		Iterator(const RichStringStyle *style, int index)
			: m_style(style),
			  m_index(index)
		{}

		inline Iterator & operator++() { m_index++; return *this; }
		inline Iterator & operator--() { m_index--; return *this; }

		inline bool operator==(const Iterator &other)
		{
			return flags() == other.flags()
					&& (!flags(RichString::Color) || color() == other.color());
		}
		inline bool operator!=(const Iterator &other) { return !(*this == other); }

		inline int index() const { return m_index; }
		inline quint8 flags(RichString::StyleFlag mask) const { return m_style->m_styleFlags[m_index] & mask; }
		inline quint8 & flags() const { return m_style->m_styleFlags[m_index]; }
		inline QRgb & color() const { return m_style->m_styleColors[m_index]; }

	private:
		const RichStringStyle *m_style;
		int m_index;
	};

public:
	RichStringStyle(int len);
	RichStringStyle(int len, quint8 styleFlags, QRgb styleColor);
	RichStringStyle(const RichStringStyle &other);
	~RichStringStyle();

	void clear();

	RichStringStyle & operator=(const RichStringStyle &other);

	qint32 voiceIndex(const QString &name);
	qint32 classIndex(const QString &name);

	inline Iterator iter(int index) const { return Iterator(this, index); }

	inline void insert(int index, int len) { replace(index, 0, len); }
	void replace(int index, int len, int newLen);
	void fill(int index, int len, quint8 flags, QRgb color);
	void copy(int index, int len, const RichStringStyle &src, int srcOffset=0);

	void swap(RichStringStyle &other);

private:
	void detach();
	void updateCapacity();

private:
	quint8 *m_styleFlags;
	QRgb *m_styleColors;
	int m_length;
	int m_capacity;
};

struct SSHelper {
	struct BackRef {
		int start;
		int end;
		int no;
	};
	struct MatchRef {
		int offset;
		int length;
		enum { NONE, SUBJECT, REPLACEMENT } ref;
	};
	using MatchRefList = QList<MatchRef>;

	template<class T>
	static MatchRefList match(RichString &str, const QString &before, const T &after, Qt::CaseSensitivity cs);

	template<class T>
	static MatchRefList match(RichString &str, const QRegExp &regExp, const T &replacement);

	template<class T>
	static MatchRefList match(RichString &str, const QRegularExpression &regExp, const T &replacement);

	template<class T>
	static void replace(const MatchRefList &matchList, RichString &str, const T &replacement);
};
}

void
RichStringStyle::replace(int index, int lenRemove, int lenAdd)
{
	Q_ASSERT(index + lenRemove <= m_length);
	const int tailLength = m_length - index - lenRemove;

	quint8 *oldStyleFlags = m_styleFlags;
	QRgb *oldStyleColors = m_styleColors;

	detach();
	//Q_ASSERT(index + lenRemove + tailLength == m_length);
	m_length += lenAdd - lenRemove;
	//Q_ASSERT(index + lenAdd + tailLength == m_length);
	updateCapacity();

	if(index) {
		// restore data before inserted part
		memcpy(m_styleFlags, oldStyleFlags, index * sizeof(*m_styleFlags));
		memcpy(m_styleColors, oldStyleColors, index * sizeof(*m_styleColors));
	}
	if(tailLength) {
		// restore data after inserted part
		const int tailStart = index + lenAdd;
		const int tailStartOld = index + lenRemove;
		memcpy(m_styleFlags + tailStart, oldStyleFlags + tailStartOld, tailLength * sizeof(*m_styleFlags));
		memcpy(m_styleColors + tailStart, oldStyleColors + tailStartOld, tailLength * sizeof(*m_styleColors));
	}

	delete[] oldStyleFlags;
	delete[] oldStyleColors;
}

void
RichStringStyle::fill(int index, int len, quint8 flags, QRgb color)
{
	Q_ASSERT(index + len <= m_length);
	if(len) {
		memset_n(m_styleFlags + index, flags, len);
		memset_n(m_styleColors + index, color, len);
	}
}

void
RichStringStyle::copy(int index, int len, const RichStringStyle &src, int srcOffset)
{
	Q_ASSERT(index + len <= m_length);
	if(len) {
		memcpy(m_styleFlags + index, src.m_styleFlags + srcOffset, len * sizeof(*m_styleFlags));
		memcpy(m_styleColors + index, src.m_styleColors + srcOffset, len * sizeof(*m_styleColors));
	}
}

RichStringStyle::RichStringStyle(int len)
	: m_styleFlags(nullptr),
	  m_styleColors(nullptr),
	  m_length(len),
	  m_capacity(0)
{
	if(m_length)
		updateCapacity();
}

RichStringStyle::RichStringStyle(int len, quint8 styleFlags, QRgb styleColor)
	: m_styleFlags(nullptr),
	  m_styleColors(nullptr),
	  m_length(len),
	  m_capacity(0)
{
	if(m_length) {
		updateCapacity();
		fill(0, m_length, quint8(styleFlags & RichString::AllStyles), styleColor);
	}
}

RichStringStyle::RichStringStyle(const RichStringStyle &other)
	: m_styleFlags(nullptr),
	  m_styleColors(nullptr),
	  m_length(other.m_length),
	  m_capacity(0)
{
	if(m_length) {
		updateCapacity();
		copy(0, m_length, other);
	}
}

RichStringStyle::~RichStringStyle()
{
	delete[] m_styleFlags;
	delete[] m_styleColors;
}

void
RichStringStyle::clear()
{
	m_length = 0;
	updateCapacity();
}

RichStringStyle &
RichStringStyle::operator=(const RichStringStyle &other)
{
	m_length = other.m_length;
	updateCapacity();
	copy(0, m_length, other);
	return *this;
}

void
RichStringStyle::swap(RichStringStyle &other)
{
	qSwap(m_styleFlags, other.m_styleFlags);
	qSwap(m_styleColors, other.m_styleColors);
	qSwap(m_length, other.m_length);
	qSwap(m_capacity, other.m_capacity);
}

void
RichStringStyle::detach()
{
	m_styleFlags = nullptr;
	m_styleColors = nullptr;
	m_capacity = 0;
}

void
RichStringStyle::updateCapacity()
{
	if(m_length > m_capacity)
		m_capacity = m_length * 2;
	else if(m_length == 0)
		m_capacity = 0;
	else if(m_capacity > 100 && m_length < m_capacity / 2)
		m_capacity = m_capacity / 2;
	else if(m_styleFlags)
		return;

	delete[] m_styleFlags;
	delete[] m_styleColors;
	if(m_capacity) {
		m_styleFlags = new quint8[m_capacity];
		m_styleColors = new QRgb[m_capacity];
	} else {
		m_styleFlags = nullptr;
		m_styleColors = nullptr;
	}
}


RichString::RichString(const QString &string, quint8 styleFlags, QRgb styleColor)
	: QString(string),
	  m_style(new RichStringStyle(string.length(), styleFlags, styleColor))
{
}

RichString::RichString(const RichString &richstring)
	: QString(richstring),
	  m_style(new RichStringStyle(*richstring.m_style))
{
}

RichString &
RichString::operator=(const RichString &richstring)
{
	if(this != &richstring) {
		QString::operator=(richstring);
		*m_style = RichStringStyle(*richstring.m_style);
	}
	return *this;
}

RichString::~RichString()
{
	delete m_style;
}

void
RichString::setString(const QString &string, quint8 styleFlags, QRgb styleColor)
{
	QString::operator=(string);
	*m_style = RichStringStyle(string.length(), styleFlags, styleColor);
}

quint8
RichString::styleFlagsAt(int index) const
{
	if(index < 0 || index >= length())
		return 0;
	return m_style->iter(index).flags();
}

void
RichString::setStyleFlagsAt(int index, quint8 styleFlags) const
{
	if(index < 0 || index >= length())
		return;
	m_style->iter(index).flags() = styleFlags;
}

QRgb
RichString::styleColorAt(int index) const
{
	if(index < 0 || index >= length())
		return 0;
	auto it = m_style->iter(index);
	return it.flags(Color) == 0 ? 0 : it.color();
}

void
RichString::setStyleColorAt(int index, QRgb rgbColor) const
{
	if(index < 0 || index >= length())
		return;
	auto it = m_style->iter(index);
	if(rgbColor == 0)
		it.flags() &= ~RichString::Color;
	else
		it.flags() |= RichString::Color;
	it.color() = rgbColor;
}

QDataStream &
operator<<(QDataStream &stream, const RichString &string)
{
	stream << static_cast<const QString &>(string);
	stream.writeRawData(reinterpret_cast<const char *>(string.m_style->m_styleFlags), string.length() * sizeof(*string.m_style->m_styleFlags));
	stream.writeRawData(reinterpret_cast<const char *>(string.m_style->m_styleColors), string.length() * sizeof(*string.m_style->m_styleColors));
	return stream;
}

QDataStream &
operator>>(QDataStream &stream, RichString &string)
{
	stream >> static_cast<QString &>(string);
	string.m_style->m_length = string.length();
	string.m_style->updateCapacity();
	stream.readRawData(reinterpret_cast<char *>(string.m_style->m_styleFlags), string.length() * sizeof(*string.m_style->m_styleFlags));
	stream.readRawData(reinterpret_cast<char *>(string.m_style->m_styleColors), string.length() * sizeof(*string.m_style->m_styleColors));
	return stream;
}

QString
RichString::richString() const
{
	QString ret;

	if(isEmpty())
		return ret;

	auto prevStyle = m_style->iter(0);
	int prevIndex = 0;

	if(prevStyle.flags(Italic))
		ret += "<i>";
	if(prevStyle.flags(Bold))
		ret += "<b>";
	if(prevStyle.flags(Underline))
		ret += "<u>";
	if(prevStyle.flags(StrikeThrough))
		ret += "<s>";
	if(prevStyle.flags(Color))
		ret += "<font color=" + QColor(prevStyle.color()).name() + ">";

	const int size = length();
	for(int index = 1; index < size; ++index) {
		auto style = m_style->iter(index);
		if(prevStyle != style) {
			ret += QString::mid(prevIndex, index - prevIndex)
					.replace('<', "&lt;")
					.replace('>', "&gt;");

			if(prevStyle.flags(StrikeThrough) && !style.flags(StrikeThrough))
				ret += "</s>";
			if(prevStyle.flags(Underline) && !style.flags(Underline))
				ret += "</u>";
			if(prevStyle.flags(Bold) && !style.flags(Bold))
				ret += "</b>";
			if(prevStyle.flags(Italic) && !style.flags(Italic))
				ret += "</i>";
			if(prevStyle.flags(Color) && (!style.flags(Color) || prevStyle.color() != style.color()))
				ret += "</font>";

			while(index < size) {
				// place opening html tags after spaces/newlines
				const QChar ch = at(index);
				if(ch != '\n' && ch != '\r' && ch != ' ' && ch != '\t')
					break;
				ret += ch;
				index++;
			}

			if(!prevStyle.flags(Italic) && style.flags(Italic))
				ret += "<i>";
			if(!prevStyle.flags(Bold) && style.flags(Bold))
				ret += "<b>";
			if(!prevStyle.flags(Underline) && style.flags(Underline))
				ret += "<u>";
			if(!prevStyle.flags(StrikeThrough) && style.flags(StrikeThrough))
				ret += "<s>";
			if(style.flags(Color) && (!prevStyle.flags(Color) || prevStyle.color() != style.color()))
				ret += "<font color=" + QColor(style.color()).name() + ">";

			prevStyle = style;
			prevIndex = index;
		}
	}
	if(prevIndex != length()) {
		ret += QString::mid(prevIndex, length() - prevIndex)
			.replace('<', "&lt;")
			.replace('>', "&gt;");
		if(prevStyle.flags(StrikeThrough))
			ret += "</s>";
		if(prevStyle.flags(Underline))
			ret += "</u>";
		if(prevStyle.flags(Bold))
			ret += "</b>";
		if(prevStyle.flags(Italic))
			ret += "</i>";
		if(prevStyle.flags(Color))
			ret += "</font>";
	}
	return ret;
}

RichString &
RichString::setRichString(const QStringRef &string)
{
	staticRE$(tagRegExp, "<(/?([bius]|font))[^>]*(\\s+color=\"?([\\w#]+)\"?)?[^>]*>", REu | REi);

	QRegularExpressionMatchIterator it = tagRegExp.globalMatch(string);

	clear();

	quint8 currentStyle = 0;
	QColor currentColor;
	int offsetPos = 0, matchedPos = -1;
	while(it.hasNext()) {
		QRegularExpressionMatch m = it.next();

		matchedPos = m.capturedStart();
		QString matched(m.captured(1).toLower());

		quint8 newStyle = currentStyle;
		QColor newColor(currentColor);

		if(matched == QLatin1String("b")) {
			newStyle |= RichString::Bold;
		} else if(matched == QLatin1String("i")) {
			newStyle |= RichString::Italic;
		} else if(matched == QLatin1String("u")) {
			newStyle |= RichString::Underline;
		} else if(matched == QLatin1String("s")) {
			newStyle |= RichString::StrikeThrough;
		} else if(matched == QLatin1String("font")) {
			const QString &color = m.captured(4);
			if(!color.isEmpty()) {
				newStyle |= RichString::Color;
				newColor.setNamedColor(color.toLower());
			}
		} else if(matched == QLatin1String("/b")) {
			newStyle &= ~RichString::Bold;
		} else if(matched == QLatin1String("/i")) {
			newStyle &= ~RichString::Italic;
		} else if(matched == QLatin1String("/u")) {
			newStyle &= ~RichString::Underline;
		} else if(matched == QLatin1String("/s")) {
			newStyle &= ~RichString::StrikeThrough;
		} else if(matched == QLatin1String("/font")) {
			newStyle &= ~RichString::Color;
			newColor.setNamedColor("-invalid-");
		}

		QString token(string.mid(offsetPos, matchedPos - offsetPos).toString());
		append(RichString(token, currentStyle, currentColor.isValid() ? currentColor.rgb() : 0));
		currentStyle = newStyle;
		currentColor = newColor;

		offsetPos = m.capturedEnd();
	}

	QString token(string.mid(offsetPos, matchedPos - offsetPos).toString());
	append(RichString(token /*.replace("&lt;", "<").replace("&gt;", ">")*/, currentStyle, currentColor.isValid() ? currentColor.rgb() : 0));

	return *this;
}

int
RichString::cummulativeStyleFlags() const
{
	int cummulativeStyleFlags = 0;
	for(int i = 0, size = length(); i < size; i++) {
		cummulativeStyleFlags |= m_style->iter(i).flags();
		if(cummulativeStyleFlags == AllStyles)
			break;
	}
	return cummulativeStyleFlags;
}

bool
RichString::hasStyleFlags(int styleFlags) const
{
	int cummulativeStyleFlags = 0;
	for(int i = 0, size = length(); i < size; i++) {
		cummulativeStyleFlags |= m_style->iter(i).flags();
		if((cummulativeStyleFlags & styleFlags) == styleFlags)
			return true;
	}
	return false;
}

RichString &
RichString::setStyleFlags(int index, int len, int styleFlags)
{
	if(index < 0 || index >= length())
		return *this;

	for(int end = index + length(index, len); index < end; index++)
		m_style->iter(index).flags() = styleFlags;

	return *this;
}

RichString &
RichString::setStyleFlags(int index, int len, int styleFlags, bool on)
{
	if(index < 0 || index >= length())
		return *this;

	const int end = index + length(index, len);
	if(on) {
		for(; index < end; index++)
			m_style->iter(index).flags() |= styleFlags;
	} else {
		styleFlags = ~styleFlags;
		for(; index < end; index++)
			m_style->iter(index).flags() &= styleFlags;
	}

	return *this;
}

RichString &
RichString::setStyleColor(int index, int len, QRgb color)
{
	if(index < 0 || index >= length())
		return *this;

	for(int end = index + length(index, len); index < end; index++) {
		m_style->iter(index).color() = color;
		if(color)
			m_style->iter(index).flags() |= Color;
		else
			m_style->iter(index).flags() &= ~Color;
	}

	return *this;
}

void
RichString::clear()
{
	QString::clear();
	m_style->clear();
}

RichString &
RichString::insert(int index, QChar ch)
{
	if(index >= 0 && index <= length()) {
		quint8 fillFlags = 0;
		QRgb fillColor = 0;
		if(length()) {
			auto it = m_style->iter(index ? index - 1 : 0);
			fillFlags = it.flags();
			fillColor = it.color();
		}

		QString::insert(index, ch);
		m_style->insert(index, 1);
		m_style->fill(index, 1, fillFlags, fillColor);
	}

	return *this;
}

RichString &
RichString::insert(int index, const QString &str)
{
	if(!length()) {
		setString(str);
		return *this;
	}

	if(str.length() && index >= 0 && index <= length()) {
		auto it = m_style->iter(index ? index - 1 : 0);
		const quint8 fillFlags = it.flags();
		const QRgb fillColor = it.color();

		QString::insert(index, str);
		m_style->insert(index, str.length());
		m_style->fill(index, str.length(), fillFlags, fillColor);
	}

	return *this;
}

RichString &
RichString::insert(int index, const RichString &str)
{
	if(!length()) {
		*this = str;
		return *this;
	}

	if(str.length() && index >= 0 && index <= length()) {
		QString::insert(index, str);
		m_style->insert(index, str.length());
		m_style->copy(index, str.length(), *str.m_style);
	}

	return *this;
}

RichString &
RichString::replace(int index, int len, const QString &replacement)
{
	if(index < 0 || index >= length())
		return *this;

	len = length(index, len);

	if(!len && !replacement.length())
		return *this; // nothing to do

	QString::replace(index, len, replacement);

	auto it = m_style->iter(index);
	const quint8 fillFlags = it.flags();
	const QRgb fillColor = it.color();
	if(len != replacement.length())
		m_style->replace(index, len, replacement.length());
	else if(len == 1)
		return *this; // there's no need to change the styles (char substitution)
	m_style->fill(index, replacement.length(), fillFlags, fillColor);

	return *this;
}

RichString &
RichString::replace(int index, int len, const RichString &replacement)
{
	if(index < 0 || index >= length())
		return *this;

	len = length(index, len);

	if(!len && !replacement.length())
		return *this; // nothing to do

	QString::replace(index, len, replacement);

	if(len != replacement.length())
		m_style->replace(index, len, replacement.length());
	m_style->copy(index, replacement.length(), *replacement.m_style);

	return *this;
}

RichString &
RichString::replace(const QString &before, const QString &after, Qt::CaseSensitivity cs)
{
	if(before.isEmpty() && after.isEmpty())
		return *this;

	if(before.length() == 1 && after.length() == 1) {
		// there's no need to change the styles (char substitution)
		QString::replace(before, after);
		return *this;
	}

	const SSHelper::MatchRefList matchList = SSHelper::match(*this, before, after, cs);
	if(!matchList.empty())
		SSHelper::replace(matchList, *this, after);

	return *this;
}

RichString &
RichString::replace(const QString &before, const RichString &after, Qt::CaseSensitivity cs)
{
	if(before.isEmpty() && after.isEmpty())
		return *this;

	const SSHelper::MatchRefList matchList = SSHelper::match(*this, before, after, cs);
	if(!matchList.empty())
		SSHelper::replace(matchList, *this, after);

	return *this;
}

RichString &
RichString::replace(QChar before, QChar after, Qt::CaseSensitivity cs)
{
	QString::replace(before, after, cs);
	return *this;
}

RichString &
RichString::replace(QChar ch, const QString &after, Qt::CaseSensitivity cs)
{
	if(after.length() == 1) {
		// there's no need to change the styles (char substitution)
		QString::replace(ch, after.at(0));
		return *this;
	}

	const SSHelper::MatchRefList matchList = SSHelper::match(*this, ch, after, cs);
	if(!matchList.empty())
		SSHelper::replace(matchList, *this, after);

	return *this;
}

RichString &
RichString::replace(QChar ch, const RichString &after, Qt::CaseSensitivity cs)
{
	const SSHelper::MatchRefList matchList = SSHelper::match(*this, ch, after, cs);
	if(!matchList.empty())
		SSHelper::replace(matchList, *this, after);
	return *this;
}

RichString &
RichString::replace(const QRegExp &regExp, const QString &replacement)
{
	const SSHelper::MatchRefList matchList = SSHelper::match(*this, regExp, replacement);
	if(!matchList.empty())
		SSHelper::replace(matchList, *this, replacement);
	return *this;
}

RichString &
RichString::replace(const QRegExp &regExp, const RichString &replacement)
{
	const SSHelper::MatchRefList matchList = SSHelper::match(*this, regExp, replacement);
	if(!matchList.empty())
		SSHelper::replace(matchList, *this, replacement);
	return *this;
}

RichString &
RichString::replace(const QRegularExpression &regExp, const QString &replacement)
{
	const SSHelper::MatchRefList matchList = SSHelper::match(*this, regExp, replacement);
	if(!matchList.empty())
		SSHelper::replace(matchList, *this, replacement);
	return *this;
}

RichString &
RichString::replace(const QRegularExpression &regExp, const RichString &replacement)
{
	const SSHelper::MatchRefList matchList = SSHelper::match(*this, regExp, replacement);
	if(!matchList.empty())
		SSHelper::replace(matchList, *this, replacement);
	return *this;
}

RichStringList
RichString::split(const QString &sep, QString::SplitBehavior behavior, Qt::CaseSensitivity cs) const
{
	RichStringList ret;

	if(sep.length()) {
		int off = 0;
		for(;;) {
			const int matchedIndex = indexOf(sep, off, cs);
			if(matchedIndex == -1)
				break;
			if(behavior == QString::KeepEmptyParts || matchedIndex != off)
				ret << mid(off, matchedIndex - off);
			off = matchedIndex + sep.length();
		}
		if(behavior == QString::KeepEmptyParts || off < length() - 1)
			ret << mid(off);
	} else if(behavior == QString::KeepEmptyParts || length()) {
		ret << *this;
	}

	return ret;
}

RichStringList
RichString::split(const QChar &sep, QString::SplitBehavior behavior, Qt::CaseSensitivity cs) const
{
	RichStringList ret;

	int off = 0;
	for(;;) {
		const int matchedIndex = indexOf(sep, off, cs);
		if(matchedIndex == -1)
			break;
		if(behavior == QString::KeepEmptyParts || matchedIndex != off)
			ret << mid(off, matchedIndex - off);
		off = matchedIndex + 1;
	}
	if(behavior == QString::KeepEmptyParts || off < length() - 1)
		ret << mid(off);

	return ret;
}

RichStringList
RichString::split(const QRegExp &sep, QString::SplitBehavior behavior) const
{
	RichStringList ret;

	int off = 0;
	for(;;) {
		const int matchedIndex = sep.indexIn(*this, off);
		if(matchedIndex == -1)
			break;
		if(behavior == QString::KeepEmptyParts || matchedIndex != off)
			ret << mid(off, matchedIndex - off);
		off = matchedIndex + sep.matchedLength();
	}
	if(behavior == QString::KeepEmptyParts || off < length() - 1)
		ret << mid(off);

	return ret;
}

RichString
RichString::left(int len) const
{
	len = length(0, len);
	RichString ret;
	ret.operator=(QString::left(len));
	ret.m_style->copy(0, len, *m_style, 0);
	return ret;
}

RichString
RichString::right(int len) const
{
	len = length(0, len);
	RichString ret;
	ret.operator=(QString::right(len));
	ret.m_style->copy(0, len, *m_style, length() - len);
	return ret;
}

RichString
RichString::mid(int index, int len) const
{
	if(index < 0) {
		if(len >= 0)
			len += index;
		index = 0;
	}

	if(index >= (int)length())
		return RichString();

	len = length(index, len);
	RichString ret;
	ret.operator=(QString::mid(index, len));
	ret.m_style->copy(0, len, *m_style, index);
	return ret;
}

RichString
RichString::toLower() const
{
	RichString ret(*this);
	ret.operator=(QString::toLower());
	return ret;
}

RichString
RichString::toUpper() const
{
	RichString ret(*this);
	ret.operator=(QString::toUpper());
	return ret;
}

RichString
RichString::toTitleCase(bool lowerFirst) const
{
	const QString wordSeparators(QStringLiteral(" -_([:,;./\\\t\n\""));

	RichString ret(*this);

	if(lowerFirst)
		ret.operator=(QString::toLower());

	bool wordStart = true;
	for(uint idx = 0, size = length(); idx < size; ++idx) {
		QCharRef chr = ret[idx];
		if(wordStart) {
			if(!wordSeparators.contains(chr)) {
				wordStart = false;
				chr = chr.toUpper();
			}
		} else if(wordSeparators.contains(chr)) {
			wordStart = true;
		}
	}

	return ret;
}

RichString
RichString::toSentenceCase(bool lowerFirst, bool *cont) const
{
	const QString sentenceEndChars(".?!");

	RichString ret(*this);

	if(lowerFirst)
		ret.operator=(QString::toLower());

	if(isEmpty())
		return ret;

	uint prevDots = 0;
	bool startSentence = cont ? !*cont : true;

	for(uint index = 0, size = length(); index < size; ++index) {
		QCharRef chr = ret[index];

		if(sentenceEndChars.contains(chr)) {
			if(chr == '.') {
				prevDots++;
				startSentence = prevDots < 3;
			} else {
				prevDots = 0;
				startSentence = true;
			}
		} else {
			if(startSentence && chr.isLetterOrNumber()) {
				chr = chr.toUpper();
				startSentence = false;
			}

			if(!chr.isSpace())
				prevDots = 0;
		}
	}

	if(cont)
		*cont = prevDots != 1 && !startSentence;

	return ret;
}

RichString
RichString::simplified() const
{
	const QRegExp simplifySpaceRegExp("\\s{2,MAXINT}");

	return trimmed().replace(simplifySpaceRegExp, " ");
}

RichString
RichString::trimmed() const
{
	const QRegExp trimRegExp("(^\\s+|\\s+$)");

	RichString ret(*this);
	return ret.remove(trimRegExp);
}

void
RichString::simplifyWhiteSpace(QString &text)
{
	int di = 0;
	bool lastWasSpace = true;
	bool lastWasLineFeed = true;
	for(int i = 0, l = text.size(); i < l; i++) {
		const QChar ch = text.at(i);
		if(lastWasSpace && (ch == QChar::Space || ch == QChar::Tabulation)) // skip consecutive spaces
			continue;
		if(lastWasLineFeed && (ch == QChar::LineFeed || ch == QChar::CarriageReturn)) // skip consecutive newlines
			continue;
		if(lastWasSpace && (ch == QChar::LineFeed || ch == QChar::CarriageReturn)) // skip space before newline
			di--;

		if(ch == QChar::Tabulation) // convert tab to space
			text[di] = QChar::Space;
		else if(ch == QChar::CarriageReturn) // convert cr to lf
			text[di] = QChar::LineFeed;
		else if(di != i) // copy other chars
			text[di] = ch;

		lastWasLineFeed = text[di] == QChar::LineFeed;
		lastWasSpace = lastWasLineFeed || text[di] == QChar::Space;

		di++;
	}
	if(lastWasLineFeed)
		di--;
	text.truncate(di);
}

void
RichString::simplifyWhiteSpace()
{
	int di = 0;
	bool lastWasSpace = true;
	bool lastWasLineFeed = true;
	for(int i = 0, l = size(); i < l; i++) {
		const QChar ch = at(i);
		if(lastWasSpace && (ch == QChar::Space || ch == QChar::Tabulation)) // skip consecutive spaces
			continue;
		if(lastWasLineFeed && (ch == QChar::LineFeed || ch == QChar::CarriageReturn)) // skip consecutive newlines
			continue;
		if(lastWasSpace && (ch == QChar::LineFeed || ch == QChar::CarriageReturn)) // skip space before newline
			di--;

		if(ch == QChar::Tabulation) // convert tab to space
			operator[](di) = QChar::Space;
		else if(ch == QChar::CarriageReturn) // convert cr to lf
			operator[](di) = QChar::LineFeed;
		else if(di != i) // copy other chars
			operator[](di) = ch;

		if(di != i)
			m_style->copy(di, 1, *m_style, i);

		lastWasLineFeed = at(di) == QChar::LineFeed;
		lastWasSpace = lastWasLineFeed || at(di) == QChar::Space;

		di++;
	}
	if(lastWasLineFeed)
		di--;
	truncate(di);
}

bool
RichString::operator!=(const RichString &richstring) const
{
	if(!(static_cast<const QString &>(*this) == static_cast<const QString &>(richstring)))
		return true;

	for(int i = 0, sz = length(); i < sz; i++) {
		auto s1 = m_style->iter(i);
		auto s2 = richstring.m_style->iter(i);
		if(s1.flags() != s2.flags())
			return true;
		if(s1.flags(Color) && s1.color() != s2.color())
			return true;
	}

	return false;
}

RichStringList::RichStringList()
{}

RichStringList::RichStringList(const RichString &str)
{
	append(str);
}

RichStringList::RichStringList(const RichStringList &list) :
	QList<RichString>(list)
{}

RichStringList::RichStringList(const QList<RichString> &list) :
	QList<RichString>(list)
{}

RichStringList::RichStringList(const QStringList &list)
{
	for(QStringList::ConstIterator it = list.begin(), end = list.end(); it != end; ++it)
		append(*it);
}

RichStringList::RichStringList(const QList<QString> &list)
{
	for(QList<QString>::ConstIterator it = list.begin(), end = list.end(); it != end; ++it)
		append(*it);
}

RichString
RichStringList::join(const RichString &sep) const
{
	RichString ret;

	bool skipSeparator = true;
	for(RichStringList::ConstIterator it = begin(), end = this->end(); it != end; ++it) {
		if(skipSeparator) {
			ret += *it;
			skipSeparator = false;
			continue;
		}
		ret += sep;
		ret += *it;
	}

	return ret;
}



// SSHelper - templated replacements

template<class T>
SSHelper::MatchRefList
SSHelper::match(RichString &str, const QString &before, const T &after, Qt::CaseSensitivity cs)
{
	MatchRefList matchList;
	int newLength = str.length();
	bool matched = false;
	if(before.isEmpty()) {
		// before is empty - do what QString does
		for(int i = 0; i < str.length(); i++) {
			if(!after.isEmpty())
				matchList.push_back(MatchRef{0, after.length(), MatchRef::REPLACEMENT});
			matchList.push_back(MatchRef{i, 1, MatchRef::SUBJECT});
		}
		if(!after.isEmpty()) {
			matchList.push_back(MatchRef{0, after.length(), MatchRef::REPLACEMENT});
			newLength += (str.length() + 1) * after.length();
		}
	} else {
		const int newLengthStep = after.length() - before.length();
		int off = 0;
		int len;
		for(;;) {
			const int idx = str.indexOf(before, off, cs);
			if(idx == -1)
				break;
			matched = true;
			newLength += newLengthStep;
			// subject part before the match
			if((len = idx - off))
				matchList.push_back(MatchRef{off, len, MatchRef::SUBJECT});
			// replacement
			if(!after.isEmpty())
				matchList.push_back(MatchRef{0, after.length(), MatchRef::REPLACEMENT});
			off = idx + before.length();
		}
		// subject part after all matches
		if((len = str.length() - off))
			matchList.push_back(MatchRef{off, len, MatchRef::SUBJECT});
	}
	// old/new total lengths
	if(matched || !matchList.empty())
		matchList.push_back(MatchRef{str.length(), newLength, MatchRef::NONE});
	return matchList;
}

template<class T>
SSHelper::MatchRefList
SSHelper::match(RichString &str, const QRegExp &regExp, const T &replacement)
{
	MatchRefList matchList;
	if(!regExp.isValid()) {
		qWarning()
			<< "SSHelper::match(): invalid regular expression:\n\t"
			<< regExp.pattern() << "\n\t" << regExp.errorString();
		return matchList;
	}

	// prepare backreference offset list
	QVector<BackRef> backRefs;
	const int capCount = regExp.captureCount();
	const QChar *repChar = replacement.unicode();
	const QChar *repEnd = repChar + replacement.size();
	while(repChar != repEnd) {
		if(*repChar++ != QLatin1Char('\\'))
			continue;
		int no = repChar->digitValue();
		repChar++;
		if(no >= 0 && no <= capCount) {
			const int start = int(repChar - replacement.unicode()) - 2;
			if(repChar != repEnd) {
				const int secondDigit = repChar->digitValue();
				const int nn = (no * 10) + secondDigit;
				if(secondDigit != -1 && nn <= capCount) {
					no = nn;
					repChar++;
				}
			}
			backRefs.push_back(BackRef{start, int(repChar - replacement.unicode()), no});
		}
	}

	// handle matches
	bool matched = false;
	int matchOffset = 0;
	int newLength = 0;
	int len;
	for(;;) {
		// caret should only be matched first time (TODO: why?)
		const QRegExp::CaretMode cm = matchOffset == 0 ? QRegExp::CaretAtZero : QRegExp::CaretWontMatch;
		const int matchedIndex = regExp.indexIn(str, matchOffset, cm);
		if(matchedIndex == -1)
			break;
		matched = true;

		// subject part before the match
		if((len = matchedIndex - matchOffset)) {
			matchList.push_back(MatchRef{matchOffset, len, MatchRef::SUBJECT});
			newLength += len;
		}

		int replacementOffset = 0;
		for(const BackRef &backRef: qAsConst(backRefs)) {
			// replacement before backref
			if((len = backRef.start - replacementOffset)) {
				matchList.push_back(MatchRef{replacementOffset, len, MatchRef::REPLACEMENT});
				newLength += len;
			}

			// subject part that backref points to
			if((len = regExp.cap(backRef.no).length())) {
				matchList.push_back(MatchRef{regExp.pos(backRef.no), len, MatchRef::SUBJECT});
				newLength += len;
			}

			replacementOffset = backRef.end;
		}

		// remainging replacement
		if((len = replacement.length() - replacementOffset)) {
			matchList.push_back(MatchRef{replacementOffset, len, MatchRef::REPLACEMENT});
			newLength += len;
		}

		const int ml = regExp.matchedLength();
		if(ml == 0) {
			matchList.push_back(MatchRef{matchOffset, 1, MatchRef::SUBJECT});
			newLength++;
			matchOffset = matchedIndex + 1;
		} else {
			matchOffset = matchedIndex + ml;
		}
	}
	// subject part after all matches
	if((len = str.length() - matchOffset) > 0) {
		matchList.push_back(MatchRef{matchOffset, len, MatchRef::SUBJECT});
		newLength += len;
	}
	// old/new total lengths
	if(matched || !matchList.empty())
		matchList.push_back(MatchRef{str.length(), newLength, MatchRef::NONE});

	return matchList;
}

template<class T>
SSHelper::MatchRefList
SSHelper::match(RichString &str, const QRegularExpression &regExp, const T &replacement)
{
	MatchRefList matchList;
	if(!regExp.isValid()) {
		qWarning()
			<< "SSHelper::match(): invalid regular expression at character " << regExp.patternErrorOffset() << ":\n\t"
			<< regExp.pattern() << "\n\t" << regExp.errorString();
		return matchList;
	}

	// prepare backreference offset list
	QVector<BackRef> backRefs;
	const int capCount = regExp.captureCount();
	backRefs.reserve(capCount);
	const QChar *repChar = replacement.unicode();
	const QChar *repEnd = repChar + replacement.size();
	while(repChar != repEnd) {
		if(*repChar++ != QLatin1Char('\\'))
			continue;
		int no = repChar->digitValue();
		repChar++;
		if(no >= 0 && no <= capCount) {
			const int start = int(repChar - replacement.unicode()) - 2;
			if(repChar != repEnd) {
				const int secondDigit = repChar->digitValue();
				const int nn = (no * 10) + secondDigit;
				if(secondDigit != -1 && nn <= capCount) {
					no = nn;
					repChar++;
				}
			}
			backRefs.push_back(BackRef{start, int(repChar - replacement.unicode()), no});
		}
	}

	// handle matches
	int matchOffset = 0;
	int newLength = 0;
	int len;
	QRegularExpressionMatchIterator iterator = regExp.globalMatch(str);
	const bool matched = iterator.hasNext();
	while(iterator.hasNext()) {
		QRegularExpressionMatch match = iterator.next();

		// subject part before the match
		if((len = match.capturedStart() - matchOffset)) {
			matchList.push_back(MatchRef{matchOffset, len, MatchRef::SUBJECT});
			newLength += len;
		}

		int replacementOffset = 0;
		for(const BackRef &backRef: qAsConst(backRefs)) {
			// replacement before backref
			if((len = backRef.start - replacementOffset)) {
				matchList.push_back(MatchRef{replacementOffset, len, MatchRef::REPLACEMENT});
				newLength += len;
			}

			// subject part that backref points to
			if((len = match.capturedLength(backRef.no))) {
				matchList.push_back(MatchRef{match.capturedStart(backRef.no), len, MatchRef::SUBJECT});
				newLength += len;
			}

			replacementOffset = backRef.end;
		}

		// remainging replacement
		if((len = replacement.length() - replacementOffset)) {
			matchList.push_back(MatchRef{replacementOffset, len, MatchRef::REPLACEMENT});
			newLength += len;
		}

		matchOffset = match.capturedEnd();
//		if(match.capturedLength() == 0) // TODO: TEST THIS
//			matchOffset++;
	}
	// subject part after all matches
	if((len = str.length() - matchOffset) > 0) {
		matchList.push_back(MatchRef{matchOffset, len, MatchRef::SUBJECT});
		newLength += len;
	}
	// old/new total lengths
	if(matched || !matchList.empty())
		matchList.push_back(MatchRef{str.length(), newLength, MatchRef::NONE});

	return matchList;
}

template<class T>
void
SSHelper::replace(const MatchRefList &matchList, RichString &str, const T &replacement)
{
	const int newLength = matchList.back().length; // last entry contains total lengths
	QString newString;
	newString.reserve(newLength);
	RichStringStyle newStyle(newLength);
	int startNew = 0;
	int strStyleOffset = -1;
	for(const MatchRef &md: matchList) {
		if(!md.length)
			continue;
		if(md.ref == md.SUBJECT) {
			newString.append(str.midRef(md.offset, md.length));
			newStyle.copy(startNew, md.length, *str.m_style, md.offset);
			startNew += md.length;
			strStyleOffset = md.offset + md.length;
		} else if(md.ref == md.REPLACEMENT) {
			newString.append(replacement.midRef(md.offset, md.length));
			if(std::is_same<decltype(replacement), const RichString &>::value) {
				newStyle.copy(startNew, md.length, *static_cast<const RichString &>(replacement).m_style, md.offset);
			} else {
				if(strStyleOffset < 0)
					strStyleOffset = 0;
				if(strStyleOffset < str.length()) {
					auto it = str.m_style->iter(strStyleOffset);
					newStyle.fill(startNew, md.length, it.flags(), it.color());
				} else {
					newStyle.fill(startNew, md.length, 0, 0);
				}
			}
			startNew += md.length;
		}
	}
	str.swap(newString);
	str.m_style->swap(newStyle);
}
