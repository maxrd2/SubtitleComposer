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
skipComment(const QChar **c)
{
	// skip comment blocks
	if(**c == QChar('/') && !(*c)->isNull() && *((*c) + 1) == QChar('*')) {
		*c += 2;
		for(;;) {
			if((*c)->isNull())
				break;
			if(**c == QChar('*') && !(*c)->isNull() && *((*c) + 1) == QChar('/')) {
				*c += 2;
				break;
			}
			++(*c);
		}
		return true;
	}
	return false;
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
RichCSS::parse(const QChar *css)
{
	if(!css || css->isNull())
		return;

	const QChar *cssStart = css;

	for(;;) {
		QString cssSel = parseCssSelector(&css);

		Q_ASSERT(css->isNull() || *css == QChar('{'));
		if(*css != QChar('{'))
			break;
		css++;

		RuleList cssRules = parseCssRules(&css);

		if(!cssSel.isEmpty() && !cssRules.isEmpty()) {
			auto it = m_stylesheet.begin();
			while(it != m_stylesheet.end()) {
				if(it->selector == cssSel) {
					mergeCssRules(it->rules, cssRules);
					break;
				}
				++it;
			}
			if(it == m_stylesheet.end())
				m_stylesheet.push_back(Block{cssSel, cssRules});
		}

		Q_ASSERT(css->isNull() || *css == QChar('}'));
		if(*css != QChar('}'))
			break;
		css++;
	}

	m_unformatted.append(cssStart, css - cssStart);

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
copyCssChar(QString *dst, const QChar *&c)
{
	if(*c != QChar('\\')) {
		dst->push_back(*c);
		return false;
	}

	const QChar *e = c + 7; // max 6 hex digits
	uint32_t ucs32 = 0x80000000;
	const QChar *n = c + 1;
	for(;;) {
		if(n->isNull() || n == e)
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
		dst->push_back(QChar(ucs32 >> 4));
	} else if(!(c + 1)->isNull()) {
		// copy backslash
		dst->push_back(*c++);
		// and next char
		dst->push_back(*c);
	}
	return true;
}

RichCSS::Selector
RichCSS::parseCssSelector(const QChar **stylesheet)
{
	QString sel;
	const QChar *&c = *stylesheet;

	// skip starting spaces and comments
	while(!c->isNull()) {
		if(c->isSpace())
			c++;
		else if(!skipComment(&c))
			break;
	}

	bool skippedSpace = false;
	for(; !c->isNull() && *c != QChar('{'); c++) {
		if(skipComment(&c) && (c->isNull() || *c == QChar('{')))
			break;
		if(c->isSpace()) {
			skippedSpace = true;
			continue;
		}
		if(skippedSpace) {
			const QChar *p = sel.data() + sel.size() - 1;
			if(nonSepSel(p) && nonSepSel(c))
				sel += QChar::Space;
			skippedSpace = false;
		}
		if(copyCssChar(&sel, c))
			continue;
		if(*c == QChar('[')) {
			while(!(++c)->isNull()) {
				if(c->isSpace())
					continue;
				if(skipComment(&c) && c->isNull())
					break;
				if(copyCssChar(&sel, c))
					continue;
				if(*c == QChar(']'))
					break;
				if(*c == QChar('"')) {
					while(!(++c)->isNull()) {
						if(copyCssChar(&sel, c))
							continue;
						if(*c == QChar('"'))
							break;
					}
					if(c->isNull())
						break;
				}
			}
			if(c->isNull())
				break;
		}
	}
	return sel;
}

inline static bool
nonSepVal(const QChar *c)
{
	if(c->isSpace())
		return false;
	const char l = c->toLatin1();
	return l != '(' && l != ')' && l != '"' && l != '\'' && l != ',';
}

QString
RichCSS::parseCssKey(const QChar **stylesheet)
{
	QString cssKey;
	const QChar *&c = *stylesheet;

	// skip starting spaces and comments
	while(!c->isNull()) {
		if(c->isSpace())
			c++;
		else if(!skipComment(&c))
			break;
	}

	bool skippedSpace = false;
	for(; !c->isNull() && *c != QChar('}') && *c != QChar(':') && *c != QChar(';'); c++) {
		if(skipComment(&c) && (c->isNull() || *c == QChar('}') || *c == QChar(':') || *c == QChar(';')))
			break;
		if(c->isSpace()) {
			skippedSpace = true;
			continue;
		}
		if(skippedSpace) {
			cssKey += QChar::Space;
			skippedSpace = false;
		}
		copyCssChar(&cssKey, c);
	}
	return cssKey;
}

QString
RichCSS::parseCssValue(const QChar **stylesheet)
{
	QString cssValue;
	const QChar *&c = *stylesheet;

	// skip starting spaces and comments
	while(!c->isNull()) {
		if(c->isSpace())
			c++;
		else if(!skipComment(&c))
			break;
	}

	bool skippedSpace = false;
	for(; !c->isNull() && *c != QChar('}') && *c != QChar(';'); c++) {
		if(skipComment(&c) && (c->isNull() || *c == QChar('}') || *c == QChar(';')))
			break;
		if(c->isSpace()) {
			skippedSpace = true;
			continue;
		}
		if(skippedSpace) {
			const QChar *p = cssValue.data() + cssValue.size() - 1;
			if(nonSepVal(p) && nonSepVal(c))
				cssValue += QChar::Space;
			skippedSpace = false;
		}
		if(copyCssChar(&cssValue, c))
			continue;
		if(*c == QChar('"') || *c == QChar('\'')) {
			const QChar ce = *c;
			while(!(++c)->isNull()) {
				if(copyCssChar(&cssValue, c))
					continue;
				if(*c == ce)
					break;
			}
			if(c->isNull())
				break;
		}
	}

	return cssValue;
}

RichCSS::RuleList
RichCSS::parseCssRules(const QChar **stylesheet)
{
	RuleList cssRules;
	const QChar *&c = *stylesheet;
	for(;;) {
		QString cssKey = parseCssKey(stylesheet);

		Q_ASSERT(c->isNull() || *c == QChar('}') || *c == QChar(':') || *c == QChar(';'));
		if(*c == QChar(';')) {
			qWarning() << "invalid css - rule-name:" << cssKey << " wihtout value, terminated by" << *c;
			c++;
			continue;
		}
		if(*c != QChar(':'))
			break;
		c++;

		QString cssValue = parseCssValue(stylesheet);

		if(!cssKey.isEmpty() && !cssValue.isEmpty()) {
			RuleList::iterator it = cssRules.begin();
			for(;;) {
				if(it == cssRules.end()) {
					// new rule
					cssRules.push_back(Rule{cssKey.toUtf8(), cssValue});
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

		Q_ASSERT(c->isNull() || *c == QChar('}') || *c == QChar(';'));
		if(*c != QChar(';'))
			break;
		c++;
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
