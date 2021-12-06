/*
    SPDX-FileCopyrightText: 2021-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "richcss.h"

#include <QDebug>
#include <QStringBuilder>

using namespace SubtitleComposer;


namespace SubtitleComposer {
class RichClass {
public:
	RichClass() {}
};
}

typedef bool (*charCompare)(QChar ch);

RichCSS::RichCSS(QObject *parent)
	: QObject(parent)
{
}

RichCSS::RichCSS(RichCSS &other)
	: QObject(),
	  m_unformatted(other.m_unformatted),
	  m_stylesheet(other.m_stylesheet)
{
}

RichCSS &
RichCSS::operator=(const RichCSS &rhs)
{
	m_unformatted = rhs.m_unformatted;
	m_stylesheet = rhs.m_stylesheet;
	return *this;
}


static bool
skipComment(const QChar **c, const QChar *end)
{
	const QChar *last = end - 1;
	// skip comment blocks
	if(**c == QChar('/') && *c != last && *((*c) + 1) == QChar('*')) {
		*c += 2;
		for(;;) {
			if(*c == end)
				break;
			if(**c == QChar('*') && *c != last && *((*c) + 1) == QChar('/')) {
				*c += 2;
				break;
			}
			++(*c);
		}
		return true;
	}
	return false;
}

inline static int
skipChar(const QStringRef text, int off, const charCompare &cf)
{
	auto it = text.cbegin() + off;
	const auto end = text.cend();
	while(it != end) {
		if(skipComment(&it, end) && it == end)
			break;
		if(!cf(*it))
			break;
		++it;
	}
	return it - text.cbegin();
}

QStringList
RichCSS::RuleList::toStringList()
{
	QStringList s;
	for(auto it = cbegin(); it != cend(); ++it)
		s << QString::fromLatin1(it->name) % QChar(':') % it->value % QChar(';');
	return s;
}

void
RichCSS::clear()
{
	m_stylesheet.clear();
	m_unformatted.clear();
	emit changed();
}

void
RichCSS::parse(const QStringRef &css)
{
	m_unformatted.append(css);

	int off = 0;
	while(off < css.size()) {
		off = skipChar(css, off, [](QChar c){ return c.isSpace(); });
		int end = skipChar(css, off, [](QChar c){ return /*!c.isSpace() &&*/ c != QChar('{') /*&& c.isPrint()*/; });
		QStringRef cssSel = css.mid(off, end - off);
		off = end;

		off = skipChar(css, off, [](QChar c){ return c.isSpace() || c == QChar('{'); });
		end = skipChar(css, off, [](QChar c){ return c != QChar('}'); });
		QStringRef cssStyles = css.mid(off, end - off);
		off = end + 1;

		if(!cssSel.isEmpty() && !cssStyles.isEmpty())
			parseBlock(cssSel, cssStyles);
		else
			qWarning() << "invalid css - selector:" << cssSel << "rules:" << cssStyles;
	}
	emit changed();
}

inline static bool
nonSepSel(const QChar *c)
{
	if(c->isSpace())
		return false;
	const char l = c->toLatin1();
	return l != '>' && l != '+' && l != '~';
}

static bool
copyCssChar(QChar *&dst, const QChar *&c, const QChar *se)
{
	if(*c != QChar('\\')) {
		*dst++ = *c;
		return false;
	}

	const QChar *e = c + 7; // max 6 hex digits
	uint32_t ucs32 = 0x80000000;
	const QChar *n = c + 1;
	for(;;) {
		if(n == se || n == e)
			break;
		const char d = n->toLatin1();
		if(d >= '0' && d <= '9')
			ucs32 += d - '0';
		else if(d >= 'a' && d <= 'f')
			ucs32 += d - 'a' + 10;
		else if(d >= 'A' && d <= 'F')
			ucs32 += d - 'A' + 10;
		else
			break;
		ucs32 <<= 4;
		n++;
	}
	if(ucs32 != 0x80000000) {
		// parsed unicode char
		// NOTE: Spec at https://www.w3.org/International/questions/qa-escapes says that
		//       space after 6th digit is not needed, but can be included.
		//       Their examples ignore it - we do too.
		c = n != e && n->isSpace() ? n : n - 1;
		*dst++ = QChar(ucs32 >> 4);
	} else {
		// copy backslash
		*dst++ = *c++;
		// and next char
		if(c != se)
			*dst++ = *c;
	}
	return true;
}

RichCSS::Selector
RichCSS::parseCssSelector(const QStringRef &selector)
{
	QString str;
	str.resize(selector.size());
	QChar *sel = str.data();
	const QChar *c = selector.cbegin();
	const QChar *se = selector.cend();
	while(c != se && c->isSpace())
		c++;
	for(; c != se; c++) {
		if(skipComment(&c, se) && c == se)
			break;
		if(c->isSpace()) {
			// NOTE: if c is space *p can't underflow - we skipped starting spaces
			const QChar *p = sel - 1;
			const QChar *n = c + 1;
			if(n != se && nonSepSel(p) && nonSepSel(n))
				*sel++ = QChar::Space;
			continue;
		}
		if(copyCssChar(sel, c, se))
			continue;
		if(*c == QChar('[')) {
			while(++c != se) {
				if(c->isSpace())
					continue;
				if(skipComment(&c, se) && c == se)
					break;
				if(copyCssChar(sel, c, se))
					continue;
				if(*c == QChar(']'))
					break;
				if(*c == QChar('"')) {
					while(++c != se) {
						if(copyCssChar(sel, c, se))
							continue;
						if(*c == QChar('"'))
							break;
					}
					if(c == se)
						break;
				}
			}
			if(c == se)
				break;
		}
	}
	str.truncate(sel - str.data());
	return str;
}

inline static bool
nonSepVal(const QChar *c)
{
	if(c->isSpace())
		return false;
	const char l = c->toLatin1();
	return l != '(' && l != ')' && l != '"' && l != '\'' && l != ',';
}

static QString
cleanupCssValue(QStringRef value)
{
	QString str;
	str.resize(value.size());
	QChar *val = str.data();
	const QChar *c = value.cbegin();
	const QChar *se = value.cend();
	while(c != se && c->isSpace())
		c++;
	for(; c != se; c++) {
		if(skipComment(&c, se) && c == se)
			break;
		if(c->isSpace()) {
			// NOTE: if c is space *p can't underflow - we skipped starting spaces
			const QChar *p = val - 1;
			const QChar *n = c + 1;
			if(n != se && nonSepVal(p) && nonSepVal(n))
				*val++ = QChar::Space;
			continue;
		}
		if(copyCssChar(val, c, se))
			continue;
		if(*c == QChar('"') || *c == QChar('\'')) {
			const QChar ce = *c;
			while(++c != se) {
				if(copyCssChar(val, c, se))
					continue;
				if(*c == ce)
					break;
			}
			if(c == se)
				break;
		}
	}
	str.truncate(val - str.data());
	return str;
}

RichCSS::RuleList
RichCSS::parseCssRules(const QStringRef &rules)
{
	RuleList cssRules;
	const auto ite = rules.cend();
	auto it = rules.cbegin();
	auto ie = it;
	for(;;) {
		for(; ie != ite && *ie != QChar(':'); ++ie);
		if(ie == ite)
			break;
		QStringRef cssKey = rules.mid(it - rules.cbegin(), ie - it).trimmed();

		it = ++ie;
		for(;;) {
			if(ie == ite || *ie == QChar(';'))
				break;
			if(*ie == QChar('"') || *ie == QChar('\'')) {
				const QChar sc = *ie;
				while(++ie != ite && *ie != sc);
				if(ie == ite)
					break;
			}
			++ie;
		}
		QString cssValue = cleanupCssValue(rules.mid(it - rules.cbegin(), ie - it).trimmed());

		if(!cssKey.isEmpty() && !cssValue.isEmpty()) {
			RuleList::iterator it = cssRules.begin();
			for(;;) {
				if(it == cssRules.end()) {
					// new rule
					cssRules.push_back(Rule{cssKey.toLatin1(), cssValue});
					break;
				}
				if(it->name == cssKey) {
					// overwrite duplicate rule
					it->value = cssValue;
					break;
				}
				++it;
			}
		}

		if(ie == ite)
			break;
		it = ++ie;
	}
	return cssRules;
}

void
RichCSS::mergeCssRules(RuleList &base, const RuleList &override) const
{
	// we want the merged rule list to keep the original order - override is supposed to come after base in stylesheet

	// TODO: some rules can override multiple rules - we should expand those - e.g.
	//    background -> background-image background-repeat background-position ...
	//    font -> font-size line-height font-weight font-family ...

	// first remove duplicates from base
	for(const Rule &ro: override) {
		auto it = base.begin();
		while(it != base.end()) {
			if(it->name == ro.name)
				it = base.erase(it); // TODO: some rules can merge with existing rules, unless we expand them above
			else
				++it;
		}
	}

	// then append new rules
	base.append(override);
}

void
RichCSS::parseBlock(const QStringRef &selector, const QStringRef &rules)
{
	QString cssSel = parseCssSelector(selector);
	RuleList cssRules = parseCssRules(rules);

	for(Block &b: m_stylesheet) {
		if(b.selector == cssSel) {
			mergeCssRules(b.rules, cssRules);
			return;
		}
	}
	m_stylesheet.push_back(Block{cssSel, cssRules});
}


QMap<QByteArray, QString>
RichCSS::match(QSet<QString> selectors) const
{
	QMap<QByteArray, QString> styles;
	for(const Block &b: qAsConst(m_stylesheet)) {
		const QChar *s = b.selector.constData();
		const QChar *ss = s;
		const QChar *e = s + b.selector.size();
		bool matched = true;
		for(;;) {
			if(*s == QChar::Space || *s == QChar('>') || *s == QChar(',') || s == e) {
				if(selectors.contains(QString(ss, s - ss))) {
					if(*s == QChar(',') || s == e)
						break; // matched full selector, we're done
				} else {
					while(*s != QChar(',') && s != e)
						s++;
					if(s == e) {
						// not matched and no more selectors  - bail
						matched = false;
						break;
					}
					// not matched but have more selectors after ','
				}
				ss = s;
			}
			s++;
		}
		if(!matched)
			continue;
		// merge styles
		for(const Rule &r: qAsConst(b.rules))
			styles[r.name] = r.value;
	}
	return styles;
}
