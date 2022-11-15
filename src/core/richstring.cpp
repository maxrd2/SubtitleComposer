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

namespace SubtitleComposer {
class RichStyle {
public:
	RichStyle() {}
	RichStyle(quint8 flags, QRgb color, quint64 klass, qint32 voice)
		: m_flags(flags), m_color(color), m_class(klass), m_voice(voice) {}

	inline quint8 flags() const { return m_flags; }
	inline quint8 & flags() { return m_flags; }
	inline quint8 flags(RichString::StyleFlag mask) const { return m_flags & mask; }

	inline QRgb color() const { return flags(RichString::Color) ? m_color : s_null.m_color; }
	inline QRgb & color() { return m_color; }

	inline quint64 klass() const { return m_class; }
	inline quint64 & klass() { return m_class; }

	inline qint32 voice() const { return m_voice; }
	inline qint32 & voice() { return m_voice; }

	static const RichStyle s_null;

	inline bool operator==(const RichStyle &other) const {
		return flags() == other.flags()
			&& (!flags(RichString::Color) || color() == other.color())
			&& (voice() == other.voice())
			&& (klass() == other.klass());
	}
	inline bool operator!=(const RichStyle &other) { return !operator==(other); }

private:
	friend class Iterator;
	quint8 m_flags;
	QRgb m_color;
	quint64 m_class;
	qint32 m_voice;
};

const RichStyle RichStyle::s_null(0, 0, 0, -1);

class RichStringStyle {
	friend QDataStream & ::operator<<(QDataStream &stream, const SubtitleComposer::RichString &string);
	friend QDataStream & ::operator>>(QDataStream &stream, SubtitleComposer::RichString &string);

public:
	RichStringStyle(int len);
	RichStringStyle(int len, quint8 styleFlags, QRgb styleColor, const QString &klass, const QString &voice);
	RichStringStyle(int len, quint8 styleFlags, QRgb styleColor, const QSet<QString> &classList, const QString &voice);
	RichStringStyle(const RichStringStyle &other);
	~RichStringStyle();

	void clear();

	RichStringStyle & operator=(const RichStringStyle &other);

	qint32 voiceIndex(const QString &name);
	qint32 classIndex(const QString &name);

	inline QString voiceName(int index) const { return m_voiceList.at(index); }
	inline QString className(int index) const { return m_classList.at(index); }

	inline RichStyle * operator[](int index) { Q_ASSERT(index >= 0 && index < m_length); return &m_style[index]; }
	inline const RichStyle & at(int index) const { return index >= 0 && index < m_length ? m_style[index] : RichStyle::s_null; }

	inline void insert(int index, int len) { replace(index, 0, len); }
	void replace(int index, int len, int newLen);
	inline void fill(int index, int len, const RichStyle &style);
	void copy(int index, int len, const RichStringStyle &src, int srcOffset=0);

	void swap(RichStringStyle &other, bool swapLists);

	inline void richText(QString &out, int prevIndex, int curIndex, bool opening);

private:
	void detach();
	void updateCapacity();

private:
	QVector<QString> m_classList;
	QVector<QString> m_voiceList;
	RichStyle *m_style;
	int m_length;
	int m_capacity;
};

struct ReplaceHelper {
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

	RichStyle *oldStyle = m_style;

	detach();
	//Q_ASSERT(index + lenRemove + tailLength == m_length);
	m_length += lenAdd - lenRemove;
	//Q_ASSERT(index + lenAdd + tailLength == m_length);
	updateCapacity();

	if(index) {
		// restore data before inserted part
		memcpy(m_style, oldStyle, index * sizeof(*m_style));
	}
	if(tailLength) {
		// restore data after inserted part
		const int tailStart = index + lenAdd;
		const int tailStartOld = index + lenRemove;
		memcpy(m_style + tailStart, oldStyle + tailStartOld, tailLength * sizeof(*m_style));
	}

	delete[] oldStyle;
}

inline void
RichStringStyle::fill(int index, int len, const RichStyle &style)
{
	Q_ASSERT(index + len <= m_length);
	if(len) {
		RichStyle *s = m_style + index;
		const RichStyle *e = s + len;
		while(s != e)
			*s++ = style;
	}
}

void
RichStringStyle::copy(int index, int len, const RichStringStyle &src, int srcOffset)
{
	Q_ASSERT(index + len <= m_length);
	if(len)
		memcpy(m_style + index, src.m_style + srcOffset, len * sizeof(*m_style));
}

RichStringStyle::RichStringStyle(int len)
	: m_style(nullptr),
	  m_length(len),
	  m_capacity(0)
{
	if(m_length)
		updateCapacity();
}

RichStringStyle::RichStringStyle(int len, quint8 styleFlags, QRgb styleColor, const QString &klass, const QString &voice)
	: m_style(nullptr),
	  m_length(len),
	  m_capacity(0)
{
	if(!klass.isEmpty())
		m_classList.append(klass);
	if(!voice.isEmpty())
		m_voiceList.append(voice);
	if(m_length) {
		updateCapacity();
		fill(0, m_length, RichStyle(quint8(styleFlags & RichString::AllStyles), styleColor, m_classList.size(), m_voiceList.size() - 1));
	}
}

RichStringStyle::RichStringStyle(int len, quint8 styleFlags, QRgb styleColor, const QSet<QString> &classList, const QString &voice)
	: m_style(nullptr),
	  m_length(len),
	  m_capacity(0)
{
	quint64 classMap = 0;
	for(const QString &klass: classList) {
		m_classList.append(klass);
		classMap <<= 1;
		classMap |= 1;
	}
	if(!voice.isEmpty())
		m_voiceList.append(voice);
	if(m_length) {
		updateCapacity();
		fill(0, m_length, RichStyle(quint8(styleFlags & RichString::AllStyles), styleColor, classMap, m_voiceList.size() - 1));
	}
}

RichStringStyle::RichStringStyle(const RichStringStyle &other)
	: m_classList(other.m_classList),
	  m_voiceList(other.m_voiceList),
	  m_style(nullptr),
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
	delete[] m_style;
}

void
RichStringStyle::clear()
{
	m_classList.clear();
	m_voiceList.clear();
	m_length = 0;
	updateCapacity();
}

RichStringStyle &
RichStringStyle::operator=(const RichStringStyle &other)
{
	m_classList = other.m_classList;
	m_voiceList = other.m_voiceList;
	m_length = other.m_length;
	updateCapacity();
	copy(0, m_length, other);
	return *this;
}

qint32
RichStringStyle::voiceIndex(const QString &name)
{
	if(name.isEmpty())
		return -1;
	int i = m_voiceList.indexOf(name);
	if(i == -1) {
		i = m_voiceList.size();
		m_voiceList.push_back(name);
	}
	return i;
}

qint32
RichStringStyle::classIndex(const QString &klass)
{
	if(klass.isEmpty())
		return -1;
	int i = m_classList.indexOf(klass);
	if(i == -1) {
		i = m_classList.size();
		m_classList.push_back(klass);
	}
	return i;
}

void
RichStringStyle::swap(RichStringStyle &other, bool swapLists)
{
	if(swapLists) {
		qSwap(m_classList, other.m_classList);
		qSwap(m_voiceList, other.m_voiceList);
	}
	qSwap(m_style, other.m_style);
	qSwap(m_length, other.m_length);
	qSwap(m_capacity, other.m_capacity);
}

void
RichStringStyle::detach()
{
	m_style = nullptr;
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
	else if(m_style)
		return;

	delete[] m_style;
	m_style = m_capacity ? new RichStyle[m_capacity] : nullptr;
}


RichString::RichString(const QString &string, quint8 styleFlags, QRgb styleColor, const QSet<QString> &classList, const QString &voice)
	: QString(string),
	  m_style(new RichStringStyle(string.length(), styleFlags, styleColor, classList, voice))
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
RichString::setString(const QString &string, quint8 styleFlags, QRgb styleColor, const QString klass, const QString voice)
{
	QString::operator=(string);
	*m_style = RichStringStyle(string.length(), styleFlags, styleColor, klass, voice);
}

quint8
RichString::styleFlagsAt(int index) const
{
	return m_style->at(index).flags();
}

void
RichString::setStyleFlagsAt(int index, quint8 styleFlags) const
{
	if(index < 0 || index >= length())
		return;
	(*m_style)[index]->flags() = styleFlags;
}

QRgb
RichString::styleColorAt(int index) const
{
	return m_style->at(index).color();
}

void
RichString::setStyleColorAt(int index, QRgb rgbColor) const
{
	if(index < 0 || index >= length())
		return;
	RichStyle *it = (*m_style)[index];
	if(rgbColor == 0)
		it->flags() &= ~RichString::Color;
	else
		it->flags() |= RichString::Color;
	it->color() = rgbColor;
}

quint64
RichString::styleClassesAt(int index) const
{
	return m_style->at(index).klass();
}

void
RichString::setStyleClassesAt(int index, quint64 classes) const
{
	(*m_style)[index]->klass() = classes;
}

QSet<QString>
RichString::styleClassNamesAt(int index) const
{
	QSet<QString> res;
	quint64 k = styleClassesAt(index);
	for(int i = 0; k; k >>= 1, i++) {
		if(k & 1)
			res.insert(m_style->className(i));
	}
	return res;
}

void
RichString::setStyleClassNamesAt(int index, const QSet<QString> &classes) const
{
	quint64 k = 0;
	for(const QString &cl: classes) {
		qint32 i = m_style->classIndex(cl);
		if(i >= 0)
			k |= 1ULL << i;
	}
	setStyleClassesAt(index, k);
}

qint32
RichString::styleVoiceAt(int index) const
{
	return m_style->at(index).voice();
}

void
RichString::setStyleVoiceAt(int index, qint32 voice) const
{
	(*m_style)[index]->voice() = voice;
}

QString
RichString::styleVoiceNameAt(int index) const
{
	const qint32 i = styleVoiceAt(index);
	return i >= 0 ? m_style->voiceName(i) : QString();
}

void
RichString::setStyleVoiceNameAt(int index, const QString &voice) const
{
	const qint32 v = m_style->voiceIndex(voice);
	setStyleVoiceAt(index, v);
}

QDataStream &
operator<<(QDataStream &stream, const RichString &string)
{
	stream << static_cast<const QString &>(string);
	stream.writeRawData(reinterpret_cast<const char *>(string.m_style->m_style), string.length() * sizeof(*string.m_style->m_style));
	stream << string.m_style->m_classList;
	stream << string.m_style->m_voiceList;
	return stream;
}

QDataStream &
operator>>(QDataStream &stream, RichString &string)
{
	stream >> static_cast<QString &>(string);
	string.m_style->m_length = string.length();
	string.m_style->updateCapacity();
	stream.readRawData(reinterpret_cast<char *>(string.m_style->m_style), string.length() * sizeof(*string.m_style->m_style));
	stream >> string.m_style->m_classList;
	stream >> string.m_style->m_voiceList;
	return stream;
}

inline void
RichStringStyle::richText(QString &out, int prevIndex, int curIndex, bool opening)
{
	const RichStyle &prev = at(prevIndex);
	const RichStyle &cur = at(curIndex);
	if(opening) {
		if(prev.voice() != cur.voice())
			out += "<v " + m_voiceList.at(cur.voice()) + ">";
		if(prev.klass() != cur.klass()) {
			quint64 k = ~prev.klass() & cur.klass();
			for(int i = 0; k; k >>= 1, i++)
				out += "<c." + m_classList.at(i) + ">";
		}
		const quint8 sf = ~prev.flags() & cur.flags();
		if(sf & RichString::Italic)
			out += "<i>";
		if(sf & RichString::Bold)
			out += "<b>";
		if(sf & RichString::Underline)
			out += "<u>";
		if(sf & RichString::StrikeThrough)
			out += "<s>";
		if(sf & RichString::Color || (cur.flags(RichString::Color) && prev.color() != cur.color()))
			out += "<font color=" + QColor(cur.color()).name() + ">";
	} else {
		const quint8 sf = prev.flags() & ~cur.flags();
		if(sf & RichString::StrikeThrough)
			out += "</s>";
		if(sf & RichString::Underline)
			out += "</u>";
		if(sf & RichString::Bold)
			out += "</b>";
		if(sf & RichString::Italic)
			out += "</i>";
		if((sf & RichString::Color) || (prev.flags(RichString::Color) && prev.color() != cur.color()))
			out += "</font>";
		if(prev.klass() != cur.klass()) {
			quint64 k = prev.klass() & ~cur.klass();
			for(int i = 0; k; k >>= 1, i++)
				out += "</c." + m_classList.at(i) + ">";
		}
	}
}

QString
RichString::richString() const
{
	QString ret;

	if(isEmpty())
		return ret;

	const int len = length();
	int prev = 0;

	m_style->richText(ret, -1, prev, true);

	for(int cur = 1; cur < len; cur++) {
		if(m_style->at(prev) == m_style->at(cur))
			continue;

		// place closing html tags before spaces/newlines
		int cps = cur;
		while(cur > 0) {
			const QChar ch = at(cur - 1);
			if(ch != '\n' && ch != '\r' && ch != ' ' && ch != '\t')
				break;
			cur--;
		}

		// text
		ret += QString::mid(prev, cur - prev)
				.replace('<', "&lt;")
				.replace('>', "&gt;");

		// closing tags
		m_style->richText(ret, prev, cps, false);

		// place opening html tags after spaces/newlines
		while(cps < len) {
			const QChar ch = at(cps);
			if(ch != '\n' && ch != '\r' && ch != ' ' && ch != '\t')
				break;
			cps++;
		}

		// spaces
		ret += QString::midRef(cur, cps - cur);
		cur = cps;

		// opening tags
		m_style->richText(ret, prev, cur, true);

		prev = cur;
	}
	if(prev != length()) {
		ret += QString::mid(prev, length() - prev)
			.replace('<', "&lt;")
			.replace('>', "&gt;");
		m_style->richText(ret, prev, -1, false);
	}
	return ret;
}

RichString &
RichString::setRichString(const QStringRef &string)
{
	staticRE$(tagRegExp, "(<"
			"(/?(v |c\\.|\\w+))"
			"([^>]*\\bcolor=\"?([\\w#]+)\"?|[^>]+)?"
			"[^>]*?>|&([^;]+);|\\n)", REu | REi);
	staticRE$(colorRegExp, "style=\"[^\">]*\\bcolor:([\\w#]+)", REu | REi);

	QRegularExpressionMatchIterator it = tagRegExp.globalMatch(string);

	clear();

	QStringList colorTags;
	bool softBreak = false;
	quint8 currentStyle = 0;
	QColor currentColor;
	int offsetPos = 0, matchedPos;
	quint32 currentClass = 0;
	qint32 currentVoice = -1;
	for(;;) {
		quint8 newStyle = currentStyle;
		QColor newColor(currentColor);
		quint32 newClass = currentClass;
		qint32 newVoice = currentVoice;

		const bool validIter = it.hasNext();
		QRegularExpressionMatch m;

		QString mTag;
		QStringRef ent;

		if(validIter) {
			m = it.next();

			matchedPos = m.capturedStart();
			ent = m.capturedRef(6);
			if(ent.isNull()) {
				mTag = m.captured(2).toLower();
				if(mTag == QLatin1String("b") || mTag == QLatin1String("strong")) {
					newStyle |= RichString::Bold;
				} else if(mTag == QLatin1String("i") || mTag == QLatin1String("em")) {
					newStyle |= RichString::Italic;
				} else if(mTag == QLatin1String("u")) {
					newStyle |= RichString::Underline;
				} else if(mTag == QLatin1String("s")) {
					newStyle |= RichString::StrikeThrough;
				} else if(mTag == QLatin1String("font")) {
					const QString &color = m.captured(5);
					if(!color.isEmpty()) {
						newStyle |= RichString::Color;
						newColor.setNamedColor(color.toLower());
						colorTags.push_back(currentColor.name());
						colorTags.push_back(mTag);
					}
				} else if(mTag == QLatin1String("v ")) {
					newVoice = m_style->voiceIndex(m.captured(4));
				} else if(mTag == QLatin1String("c.")) {
					const int i = m_style->classIndex(m.captured(4));
					if(i >= 0)
						newClass |= 1UL << i;
				} else if(mTag == QLatin1String("/b") || mTag == QLatin1String("/strong")) {
					newStyle &= ~RichString::Bold;
				} else if(mTag == QLatin1String("/i") || mTag == QLatin1String("/em")) {
					newStyle &= ~RichString::Italic;
				} else if(mTag == QLatin1String("/u")) {
					newStyle &= ~RichString::Underline;
				} else if(mTag == QLatin1String("/s")) {
					newStyle &= ~RichString::StrikeThrough;
				} else if(mTag == QLatin1String("/c.")) {
					const int i = m_style->classIndex(m.captured(4));
					if(i >= 0)
						newClass &= ~(1UL << i);
				}
				if(!mTag.isEmpty()) {
					if(mTag.front() != QLatin1Char('/')) {
						QRegularExpressionMatch mc = colorRegExp.match(m.capturedRef(4));
						if(mc.hasMatch()) {
							newStyle |= RichString::Color;
							newColor.setNamedColor(mc.captured(1).toLower());
							colorTags.push_back(currentColor.name());
							colorTags.push_back(mTag);
						}
					} else if(!colorTags.empty() && mTag.midRef(1) == colorTags.back()) {
						colorTags.pop_back();
						if(colorTags.size() == 1) {
							newStyle &= ~RichString::Color;
							newColor.setNamedColor("-invalid-");
						} else {
							newStyle |= RichString::Color;
							newColor.setNamedColor(colorTags.back());
						}
						colorTags.pop_back();
					}
				}
			}
		} else {
			matchedPos = string.length();
		}

		if(const int len = matchedPos - offsetPos) {
			if(softBreak) {
				if(!isEmpty() && QString::back() != QChar::LineFeed)
					append(QChar::LineFeed);
				softBreak = false;
			}
			const int index = length();
			QString::append(string.mid(offsetPos, len));
			m_style->insert(index, len);
			m_style->fill(index, len, RichStyle(currentStyle, currentColor.isValid() ? currentColor.rgb() : 0, currentClass, currentVoice));
		}

		currentStyle = newStyle;
		currentColor = newColor;
		currentClass = newClass;
		currentVoice = newVoice;

		if(validIter) {
			if(!ent.isNull()) {
				if(ent == QLatin1String("nbsp")) {
					append(QChar::Nbsp);
				} else if(ent == QLatin1String("lt")) {
					append(QLatin1Char('<'));
				} else if(ent == QLatin1String("gt")) {
					append(QLatin1Char('>'));
				} else if(ent == QLatin1String("amp")) {
					append(QLatin1Char('&'));
				} else if(ent == QLatin1String("quot")) {
					append(QLatin1Char('"'));
				} else {
					qWarning().nospace().noquote() << "Unknown entity \"&" << ent << ";\"";
					append(ent.toString());
				}
			} else {
				if(m.capturedRef(1).front() == QChar::LineFeed) {
					softBreak = true;
				} else if(mTag == QLatin1String("br")) {
					append(QChar::LineFeed);
					softBreak = false;
				} else if(mTag == QLatin1String("p")) {
					if(!isEmpty() && QString::back() != QChar::LineFeed) {
						append(QChar::LineFeed);
						softBreak = false;
					}
				} else if(mTag == QLatin1String("/p")) {
					softBreak = true;
				}
			}
		} else {
			break;
		}

		offsetPos = m.capturedEnd();
	}

	return *this;
}

int
RichString::cummulativeStyleFlags() const
{
	int cummulativeStyleFlags = 0;
	for(int i = 0, size = length(); i < size; i++) {
		cummulativeStyleFlags |= m_style->at(i).flags();
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
		cummulativeStyleFlags |= m_style->at(i).flags();
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
		(*m_style)[index]->flags() = styleFlags;

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
			(*m_style)[index]->flags() |= styleFlags;
	} else {
		styleFlags = ~styleFlags;
		for(; index < end; index++)
			(*m_style)[index]->flags() &= styleFlags;
	}

	return *this;
}

RichString &
RichString::setStyleColor(int index, int len, QRgb color)
{
	if(index < 0 || index >= length())
		return *this;

	for(int end = index + length(index, len); index < end; index++) {
		(*m_style)[index]->color() = color;
		if(color)
			(*m_style)[index]->flags() |= Color;
		else
			(*m_style)[index]->flags() &= ~Color;
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
		const RichStyle cs = m_style->at(qMax(0, index - 1));
		QString::insert(index, ch);
		m_style->insert(index, 1);
		m_style->fill(index, 1, cs);
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
		const RichStyle cs = m_style->at(qMax(0, index - 1));
		QString::insert(index, str);
		m_style->insert(index, str.length());
		m_style->fill(index, str.length(), cs);
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

	const RichStyle cs = m_style->at(index);

	QString::replace(index, len, replacement);

	if(len != replacement.length())
		m_style->replace(index, len, replacement.length());
	else if(len == 1)
		return *this; // there's no need to change the styles (char substitution)
	m_style->fill(index, replacement.length(), cs);

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

	const ReplaceHelper::MatchRefList matchList = ReplaceHelper::match(*this, before, after, cs);
	if(!matchList.empty())
		ReplaceHelper::replace(matchList, *this, after);

	return *this;
}

RichString &
RichString::replace(const QString &before, const RichString &after, Qt::CaseSensitivity cs)
{
	if(before.isEmpty() && after.isEmpty())
		return *this;

	const ReplaceHelper::MatchRefList matchList = ReplaceHelper::match(*this, before, after, cs);
	if(!matchList.empty())
		ReplaceHelper::replace(matchList, *this, after);

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

	const ReplaceHelper::MatchRefList matchList = ReplaceHelper::match(*this, ch, after, cs);
	if(!matchList.empty())
		ReplaceHelper::replace(matchList, *this, after);

	return *this;
}

RichString &
RichString::replace(QChar ch, const RichString &after, Qt::CaseSensitivity cs)
{
	const ReplaceHelper::MatchRefList matchList = ReplaceHelper::match(*this, ch, after, cs);
	if(!matchList.empty())
		ReplaceHelper::replace(matchList, *this, after);
	return *this;
}

RichString &
RichString::replace(const QRegExp &regExp, const QString &replacement)
{
	const ReplaceHelper::MatchRefList matchList = ReplaceHelper::match(*this, regExp, replacement);
	if(!matchList.empty())
		ReplaceHelper::replace(matchList, *this, replacement);
	return *this;
}

RichString &
RichString::replace(const QRegExp &regExp, const RichString &replacement)
{
	const ReplaceHelper::MatchRefList matchList = ReplaceHelper::match(*this, regExp, replacement);
	if(!matchList.empty())
		ReplaceHelper::replace(matchList, *this, replacement);
	return *this;
}

RichString &
RichString::replace(const QRegularExpression &regExp, const QString &replacement)
{
	const ReplaceHelper::MatchRefList matchList = ReplaceHelper::match(*this, regExp, replacement);
	if(!matchList.empty())
		ReplaceHelper::replace(matchList, *this, replacement);
	return *this;
}

RichString &
RichString::replace(const QRegularExpression &regExp, const RichString &replacement)
{
	const ReplaceHelper::MatchRefList matchList = ReplaceHelper::match(*this, regExp, replacement);
	if(!matchList.empty())
		ReplaceHelper::replace(matchList, *this, replacement);
	return *this;
}

RichStringList
RichString::split(const QString &sep, Qt::SplitBehaviorFlags behavior, Qt::CaseSensitivity cs) const
{
	RichStringList ret;

	if(sep.length()) {
		int off = 0;
		for(;;) {
			const int matchedIndex = indexOf(sep, off, cs);
			if(matchedIndex == -1)
				break;
			if(behavior == Qt::KeepEmptyParts || matchedIndex != off)
				ret << mid(off, matchedIndex - off);
			off = matchedIndex + sep.length();
		}
		if(behavior == Qt::KeepEmptyParts || off < length() - 1)
			ret << mid(off);
	} else if(behavior == Qt::KeepEmptyParts || length()) {
		ret << *this;
	}

	return ret;
}

RichStringList
RichString::split(const QChar &sep, Qt::SplitBehaviorFlags behavior, Qt::CaseSensitivity cs) const
{
	RichStringList ret;

	int off = 0;
	for(;;) {
		const int matchedIndex = indexOf(sep, off, cs);
		if(matchedIndex == -1)
			break;
		if(behavior == Qt::KeepEmptyParts || matchedIndex != off)
			ret << mid(off, matchedIndex - off);
		off = matchedIndex + 1;
	}
	if(behavior == Qt::KeepEmptyParts || off < length() - 1)
		ret << mid(off);

	return ret;
}

RichStringList
RichString::split(const QRegExp &sep, Qt::SplitBehaviorFlags behavior) const
{
	RichStringList ret;

	int off = 0;
	for(;;) {
		const int matchedIndex = sep.indexIn(*this, off);
		if(matchedIndex == -1)
			break;
		if(behavior == Qt::KeepEmptyParts || matchedIndex != off)
			ret << mid(off, matchedIndex - off);
		off = matchedIndex + sep.matchedLength();
	}
	if(behavior == Qt::KeepEmptyParts || off < length() - 1)
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
		const RichStyle &s1 = m_style->at(i);
		const RichStyle &s2 = richstring.m_style->at(i);
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
ReplaceHelper::MatchRefList
ReplaceHelper::match(RichString &str, const QString &before, const T &after, Qt::CaseSensitivity cs)
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
ReplaceHelper::MatchRefList
ReplaceHelper::match(RichString &str, const QRegExp &regExp, const T &replacement)
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
ReplaceHelper::MatchRefList
ReplaceHelper::match(RichString &str, const QRegularExpression &regExp, const T &replacement)
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
ReplaceHelper::replace(const MatchRefList &matchList, RichString &str, const T &replacement)
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
			if(std::is_same<decltype(replacement), const RichString &>::value)
				newStyle.copy(startNew, md.length, *static_cast<const RichString &>(replacement).m_style, md.offset);
			else
				newStyle.fill(startNew, md.length, str.m_style->at(strStyleOffset < 0 ? 0 : strStyleOffset));
			startNew += md.length;
		}
	}
	str.swap(newString);
	str.m_style->swap(newStyle, false);
}
