/*
    SPDX-FileCopyrightText: 2020-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "richdocument.h"

#include "core/richtext/richdocumentlayout.h"
#include "core/richtext/richdom.h"
#include "helpers/common.h"

#include <QApplication>
#include <QPainter>
#include <QSharedPointer>
#include <QSet>
#include <QStyle>
#include <QStyleOptionViewItem>
#include <QTextDocumentFragment>
#include <QTextBlock>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
// QStringView use is unoptimized in Qt5, and some methods are missing pre 5.15
#include <QStringRef>
#define QStringView(x) QStringRef(&(x))
#define QStringView_ QStringRef
#define capturedView capturedRef
#else
#include <QStringView>
#define QStringView_ QStringView
#endif


#define DEBUG_CHANGES(...) //qDebug(__VA_ARGS__)

using namespace SubtitleComposer;

struct REStringCapture { int pos; int len; int no; };
Q_DECLARE_TYPEINFO(REStringCapture, Q_PRIMITIVE_TYPE);

struct REBackrefFragment {
	REBackrefFragment() : pos(-1), len(-1) {}
	int pos; int len; QSharedPointer<QTextDocumentFragment> frag;
};


enum EditType { None, ReplaceChar, ReplacePlain, ReplaceHtml, ReplaceFragment, Delete };
struct EditChange {
	EditChange() : type(None) {}
	EditChange(EditType t, int p, int l) : type(t), pos(p), len(l) {}
	EditChange(EditType t, int p, int l, const QChar &v) : type(t), pos(p), len(l), qchar(v.unicode()) {}
	EditChange(EditType t, int p, int l, QStringView_ v) : type(t), pos(p), len(l), qstr(v) {}
	EditChange(EditType t, int p, int l, const QSharedPointer<QTextDocumentFragment> v) : type(t), pos(p), len(l), qfrag(v) {}
	EditChange(EditType t, int p, int l, const QTextCursor &c) : type(t), pos(p), len(l), qfrag(new QTextDocumentFragment(c)) {}
	EditChange(EditType t, int p, int l, const QTextDocument &d) : type(t), pos(p), len(l), qfrag(new QTextDocumentFragment(&d)) {}
	EditType type;
	int pos;
	int len;
	ushort qchar = 0;
	QStringView_ qstr;
	QSharedPointer<QTextDocumentFragment> qfrag;
};


RichDocument::RichDocument(QObject *parent)
	: QTextDocument(parent),
	  m_undoableCursor(this),
	  m_stylesheet(nullptr),
	  m_domDirty(true),
	  m_dom(new RichDOM)
{
	setUndoRedoEnabled(true);

	QTextOption textOption;
	textOption.setAlignment(Qt::AlignCenter);
	textOption.setWrapMode(QTextOption::NoWrap);
	setDefaultTextOption(textOption);

	setDefaultStyleSheet($("p { display:block; white-space:pre; margin-top:0; margin-bottom:0; }"));

	setDocumentLayout(new RichDocumentLayout(this));

	connect(this, &RichDocument::contentsChanged, this, [&](){
		m_domDirty = true;
		emit domChanged();
	});
}

RichDocument::~RichDocument()
{
	delete m_dom;
}

void
RichDocument::markStylesheetDirty()
{
	m_undoableCursor.beginEditBlock();
	markContentsDirty(0, length());
	m_undoableCursor.endEditBlock();
}

void
RichDocument::setStylesheet(const RichCSS *css)
{
	if(m_stylesheet == css)
		return;
	if(m_stylesheet)
		disconnect(m_stylesheet, &RichCSS::changed, this, &RichDocument::markStylesheetDirty);
	m_stylesheet = css;
	if(m_stylesheet)
		connect(m_stylesheet, &RichCSS::changed, this, &RichDocument::markStylesheetDirty);
	markStylesheetDirty();
}

void
RichDocument::setRichText(const RichString &text, bool resetUndo)
{
	if(resetUndo)
		setUndoRedoEnabled(false);
	else
		m_undoableCursor.beginEditBlock();

	m_undoableCursor.select(QTextCursor::Document);
	m_undoableCursor.removeSelectedText();

	int currentStyleFlags = -1;
	QRgb currentStyleColor = 0;
	quint64 currentStyleClasses = 0;
	qint32 currentStyleVoice = -1;
	QTextCharFormat format;
	int prev = 0;
	for(int pos = 0, size = text.length(); pos < size; pos++) {
		const int posFlags = text.styleFlagsAt(pos);
		const QRgb posColor = text.styleColorAt(pos);
		const quint64 posClasses = text.styleClassesAt(pos);
		const qint32 posVoice = text.styleVoiceAt(pos);
		if(currentStyleFlags != posFlags || ((posFlags & SubtitleComposer::RichString::Color) && currentStyleColor != posColor)) {
			if(prev != pos) {
				m_undoableCursor.insertText(text.string().mid(prev, pos - prev), format);
				prev = pos;
			}
			currentStyleFlags = posFlags;
			currentStyleColor = posColor;
			format.setFontWeight(currentStyleFlags & SubtitleComposer::RichString::Bold ? QFont::Bold : QFont::Normal);
			format.setFontItalic(currentStyleFlags & SubtitleComposer::RichString::Italic);
			format.setFontUnderline(currentStyleFlags & SubtitleComposer::RichString::Underline);
			format.setFontStrikeOut(currentStyleFlags & SubtitleComposer::RichString::StrikeThrough);
			if((currentStyleFlags & SubtitleComposer::RichString::Color) == 0)
				format.setForeground(QBrush());
			else
				format.setForeground(QBrush(QColor(currentStyleColor)));
		}
		if(currentStyleClasses != posClasses) {
			if(prev != pos) {
				m_undoableCursor.insertText(text.string().mid(prev, pos - prev), format);
				prev = pos;
			}
			currentStyleClasses = posClasses;
			format.setProperty(RichDocument::Class, QVariant::fromValue(text.styleClassNamesAt(pos)));
		}
		if(currentStyleVoice != posVoice) {
			if(prev != pos) {
				m_undoableCursor.insertText(text.string().mid(prev, pos - prev), format);
				prev = pos;
			}
			currentStyleVoice = posVoice;
			format.setProperty(RichDocument::Voice, QVariant::fromValue(text.styleVoiceNameAt(pos)));
		}
	}
	if(prev != text.length())
		m_undoableCursor.insertText(text.string().mid(prev), format);

	if(resetUndo)
		setUndoRedoEnabled(true);
	else
		m_undoableCursor.endEditBlock();
}

QString
RichDocument::toHtml() const
{
	QString html;
	bool fB = false;
	bool fI = false;
	bool fU = false;
	bool fS = false;
	QRgb fC = 0;
	QSet<QString> fClass;
	QString fVoice; // <v:speaker name> - can't be nested... right? No need for QSet<QString>
	QTextBlock bi = begin();
	for(;;) {
		for(QTextBlock::iterator it = bi.begin(); !it.atEnd(); ++it) {
			const QTextFragment &f = it.fragment();
			if(!f.isValid())
				continue;
			const QTextCharFormat &format = f.charFormat();
			const QSet<QString> &cl = format.property(Class).value<QSet<QString>>();
			for(auto it = fClass.begin(); it != fClass.end();) {
				if(cl.contains(*it)) {
					++it;
					continue;
				}
				html.append($("</c.%1>").arg(*it));
				it = fClass.erase(it);
			}
			for(auto it = cl.cbegin(); it != cl.cend(); ++it) {
				if(fClass.contains(*it))
					continue;
				html.append($("<c.%1>").arg(*it));
				fClass.insert(*it);
			}
			const QString &vt = format.property(Voice).value<QString>();
			if(fVoice != vt) {
				fVoice = vt;
				html.append($("<v %1>").arg(fVoice));
			}
			if(fB != (format.fontWeight() == QFont::Bold))
				html.append((fB = !fB) ? $("<b>") : $("</b>"));
			if(fI != format.fontItalic())
				html.append((fI = !fI) ? $("<i>") : $("</i>"));
			if(fU != format.fontUnderline())
				html.append((fU = !fU) ? $("<u>") : $("</u>"));
			if(fS != format.fontStrikeOut())
				html.append((fS = !fS) ? $("<s>") : $("</s>"));
			const QRgb fg = format.foreground().style() != Qt::NoBrush ? format.foreground().color().toRgb().rgb() : 0;
			if(fC != fg) {
				if(fC) html.append($("</font>"));
				if((fC = fg)) html.append($("<font color=#%1>").arg(fC & 0xFFFFFF, 6, 16, QChar('0')));
			}
			html.append(f.text().replace(QChar::LineSeparator, $("<br>\n")));
		}
		bi = bi.next();
		if(bi == end()) {
			if(fB) html.append($("</b>"));
			if(fI) html.append($("</i>"));
			if(fU) html.append($("</u>"));
			if(fS) html.append($("</s>"));
			if(fC) html.append($("</font>"));
			for(const QString &cl: fClass)
				html.append($("</c.%1>").arg(cl));
			return html;
		}
		html.append($("<br>\n"));
	}
	// unreachable
}

void
RichDocument::linesToBlocks()
{
	for(QTextBlock bi = begin(); bi != end(); bi = bi.next()) {
		const QString &text = bi.text();
		for(int i = 0; i < text.length(); i++) {
			if(text.at(i).unicode() == QChar::LineSeparator) {
				m_undoableCursor.movePosition(QTextCursor::Start);
				m_undoableCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, bi.position() + i);
				m_undoableCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
				m_undoableCursor.insertBlock();
				break;
			}
		}
	}
}

void
RichDocument::setHtml(const QString &html, bool resetUndo)
{
	RichString s;
	s.setRichString(html);
	setRichText(s, resetUndo);
}

void
RichDocument::setPlainText(const QString &text, bool resetUndo)
{
	if(resetUndo)
		setUndoRedoEnabled(false);
	else
		m_undoableCursor.beginEditBlock();
	m_undoableCursor.select(QTextCursor::Document);
	m_undoableCursor.insertText(text);
	linesToBlocks();
	if(resetUndo)
		setUndoRedoEnabled(true);
	else
		m_undoableCursor.endEditBlock();
}

void
RichDocument::setDocument(const QTextDocument *doc, bool resetUndo)
{
	if(resetUndo)
		setUndoRedoEnabled(false);
	else
		m_undoableCursor.beginEditBlock();
	m_undoableCursor.select(QTextCursor::Document);
	QTextCursor cur(const_cast<QTextDocument *>(doc));
	cur.select(QTextCursor::Document);
	m_undoableCursor.insertFragment(cur.selection());
	linesToBlocks();
	if(resetUndo)
		setUndoRedoEnabled(true);
	else
		m_undoableCursor.endEditBlock();
}

void
RichDocument::clear(bool resetUndo)
{
	if(resetUndo)
		setUndoRedoEnabled(false);
	else
		m_undoableCursor.beginEditBlock();
	m_undoableCursor.select(QTextCursor::Document);
	m_undoableCursor.removeSelectedText();
	if(resetUndo)
		setUndoRedoEnabled(true);
	else
		m_undoableCursor.endEditBlock();
}

RichString
RichDocument::toRichText() const
{
	SubtitleComposer::RichString richText;

	for(QTextBlock bi = begin(); bi != end(); bi = bi.next()) {
		if(bi != begin())
			richText.append(QChar::LineFeed);
		for(QTextBlock::iterator it = bi.begin(); !it.atEnd(); ++it) {
			const QTextFragment &f = it.fragment();
			if(!f.isValid())
				continue;
			const QTextCharFormat &format = f.charFormat();
			int styleFlags = 0;
			QRgb styleColor;
			const QSet<QString> &styleClass = format.property(Class).value<QSet<QString>>();
			const QString &styleVoice = format.property(Voice).value<QString>();
			if(format.fontWeight() == QFont::Bold)
				styleFlags |= SubtitleComposer::RichString::Bold;
			if(format.fontItalic())
				styleFlags |= SubtitleComposer::RichString::Italic;
			if(format.fontUnderline())
				styleFlags |= SubtitleComposer::RichString::Underline;
			if(format.fontStrikeOut())
				styleFlags |= SubtitleComposer::RichString::StrikeThrough;
			if(format.foreground().style() != Qt::NoBrush) {
				styleFlags |= SubtitleComposer::RichString::Color;
				styleColor = format.foreground().color().toRgb().rgb();
			} else {
				styleColor = 0;
			}

			richText.append(RichString(f.text(), styleFlags, styleColor, styleClass, styleVoice));
		}
	}
	return richText;
}

void
RichDocument::replace(QChar before, QChar after, Qt::CaseSensitivity cs)
{
	const QString search(before);
	const FindFlags ff = cs == Qt::CaseSensitive ? FindCaseSensitively : FindFlags();

	m_undoableCursor.beginEditBlock();
	m_undoableCursor.movePosition(QTextCursor::Start);
	for(;;) {
		m_undoableCursor = find(search, m_undoableCursor, ff);
		if(m_undoableCursor.isNull())
			break;
		m_undoableCursor.insertText(after);
	}
	m_undoableCursor.endEditBlock();
}

int
RichDocument::indexOf(const QRegularExpression &re, int from)
{
	QTextCursor cursor = find(re, from);
	return cursor.isNull() ? -1 : cursor.position();
}

int
RichDocument::cummulativeStyleFlags() const
{
	int flags = 0;
	for(QTextBlock bi = begin(); bi != end(); bi = bi.next()) {
		for(QTextBlock::iterator it = bi.begin(); !it.atEnd(); ++it) {
			const QTextFragment &f = it.fragment();
			if(!f.isValid())
				continue;
			const QTextCharFormat &format = f.charFormat();
			// FIXME: consider classes/styles/css
			if(format.fontWeight() == QFont::Bold)
				flags |= SubtitleComposer::RichString::Bold;
			if(format.fontItalic())
				flags |= SubtitleComposer::RichString::Italic;
			if(format.fontUnderline())
				flags |= SubtitleComposer::RichString::Underline;
			if(format.fontStrikeOut())
				flags |= SubtitleComposer::RichString::StrikeThrough;
			if(format.foreground().style() != Qt::NoBrush)
				flags |= SubtitleComposer::RichString::Color;
		}
	}
	return flags;
}

QRgb
RichDocument::styleColorAt(int index) const
{
	QTextCursor c(const_cast<RichDocument *>(this));
	c.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, index);
	return c.charFormat().foreground().color().rgba();
}

void
RichDocument::applyChanges(const void *data)
{
	auto changeList = reinterpret_cast<const QVector<EditChange> *>(data);
	m_undoableCursor.beginEditBlock();
	DEBUG_CHANGES("** BEGIN '%s'", toPlainText().toUtf8().constData());
	for(auto it = changeList->crbegin(); it != changeList->crend(); ++it) {
		m_undoableCursor.movePosition(QTextCursor::Start);
		if(it->pos)
			m_undoableCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, it->pos);
		if(it->len)
			m_undoableCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, it->len);
		switch(it->type) {
		case Delete:
			DEBUG_CHANGES("Delete %d-%d '%s'", it->pos, it->pos + it->len,
						  m_undoableCursor.selectedText().toUtf8().constData());
			m_undoableCursor.removeSelectedText();
			break;
		case ReplaceChar:
			DEBUG_CHANGES("ReplaceChar %d-%d '%s' <- '%s'", it->pos, it->pos + it->len,
						  m_undoableCursor.selectedText().toUtf8().constData(),
						  QString(QChar(it->qchar)).toUtf8().constData());
			m_undoableCursor.insertText(QString(QChar(it->qchar)));
			break;
		case ReplacePlain:
			DEBUG_CHANGES("ReplacePlain %d-%d '%s' <- '%s'", it->pos, it->pos + it->len,
						  m_undoableCursor.selectedText().toUtf8().constData(),
						  it->qstr.toUtf8().constData());
			m_undoableCursor.insertText(it->qstr.toString());
			break;
		case ReplaceHtml:
			DEBUG_CHANGES("ReplaceHtml %d-%d '%s' <- '%s'", it->pos, it->pos + it->len,
						  m_undoableCursor.selectedText().toUtf8().constData(),
						  it->qstr.toUtf8().constData());
			m_undoableCursor.insertHtml(it->qstr.toString());
			break;
		case ReplaceFragment:
			DEBUG_CHANGES("ReplaceFragment %d-%d '%s' <- '%s'", it->pos, it->pos + it->len,
						  m_undoableCursor.selectedText().toUtf8().constData(),
						  it->qfrag->toPlainText().toUtf8().constData());
			m_undoableCursor.insertFragment(*it->qfrag);
			break;
		default:
			Q_ASSERT(false);
		}
	}
	DEBUG_CHANGES("** END '%s'", toPlainText().toUtf8().constData());
	m_undoableCursor.endEditBlock();
}

void
RichDocument::cleanupSpaces()
{
	QVector<EditChange> cl;

	int pos = 0;
	for(QTextBlock bi = begin(); bi != end(); bi = bi.next()) {
		const QString &text = bi.text();
		const int textLen = text.length();
		int usefulLen = textLen;

		// ignore space at the end of the line
		while(usefulLen > 0 && text.at(usefulLen - 1).isSpace())
			usefulLen--;

		if(!usefulLen) { // remove empty line
			cl.push_back(EditChange(Delete, pos ? pos - 1 : pos, textLen + 1));
			pos += textLen + 1;
			continue;
		}

		bool lastWasSpace = true;
		for(int i = 0; i < usefulLen; i++) {
			const QChar &cc = text.at(i);
			const bool thisIsSpace = cc.isSpace();
			if(lastWasSpace && thisIsSpace) { // remove consecutive spaces and spaces at the start of the line
				cl.push_back(EditChange(Delete, pos + i, 1));
				continue;
			}
			if(thisIsSpace && cc.unicode() != QChar::Space) { // tabs etc to space
				cl.push_back(EditChange(ReplaceChar, pos + i, 1, QChar(QChar::Space)));
				lastWasSpace = true;
				continue;
			}
			lastWasSpace = thisIsSpace;
		}

		// remove space at the end of the line
		if(usefulLen != textLen)
			cl.push_back(EditChange(Delete, pos + usefulLen, textLen - usefulLen));

		pos += textLen + 1;
	}

	applyChanges(&cl);
}

void
RichDocument::fixPunctuation(bool spaces, bool quotes, bool englishI, bool ellipsis, bool *cont, bool testOnly)
{
	if(isEmpty())
		return;

	if(testOnly) {
		if(!cont)
			return;
		RichDocument tmp;
		tmp.setDocument(this, true);
		tmp.fixPunctuation(spaces, quotes, englishI, ellipsis, cont, false);
		return;
	}

	if(spaces)
		cleanupSpaces();

	if(quotes) { // quotes and double quotes
		staticRE$(reQ1, "`|´|\u0092", REs | REu);
		replace(reQ1, $("'"));
		staticRE$(reQ2, "''|«|»", REs | REu);
		replace(reQ2, $("\""));
	}

	if(spaces) {
		// remove spaces after " or ' at the beginning of line
		staticRE$(reS1, "^([\"'])\\s", REs | REu);
		replace(reS1, $("\\1"));

		// remove space before " or ' at the end of line
		staticRE$(reS2, "\\s([\"'])$", REs | REu);
		replace(reS2, $("\\1"));

		// if not present, add space after '?', '!', ',', ';', ':', ')' and ']'
		staticRE$(reS3, "([\\?!,;:\\)\\]])([^\\s\"'])", REs | REu);
		replace(reS3, $("\\1 \\2"));

		// if not present, add space after '.'
		staticRE$(reS4, "(\\.)([^\\s\\.\"'])", REs | REu);
		replace(reS4, $("\\1 \\2"));

		// remove space after '¿', '¡', '(' and '['
		staticRE$(reS5, "([¿¡\\(\\[])\\s", REs | REu);
		replace(reS5, $("\\1"));

		// remove space before '?', '!', ',', ';', ':', '.', ')' and ']'
		staticRE$(reS6, "\\s([\\?!,;:\\.\\)\\]])", REs | REu);
		replace(reS6, $("\\1"));

		// remove space after ... at the beginning of sentence
		staticRE$(reS7, "^\\.\\.\\.?\\s", REs | REu);
		replace(reS7, $("..."));
	}

	if(englishI) {
		// fix english I pronoun capitalization
		staticRE$(reI, "([\\s\"'\\(\\[])i([\\s'\",;:\\.\\?!\\]\\)]|$)" , REs | REu);
		replace(reI, $("\\1I\\2"));
	}

	if(ellipsis) {
		// fix ellipsis
		staticRE$(reE1, "[,;]?\\.{2,}", REs | REu);
		staticRE$(reE2, "[,;]\\s*$", REs | REu);
		staticRE$(reE3, "[\\.:?!\\)\\]'\\\"]$", REs | REu);
		replace(reE1, $("..."));
		replace(reE2, $("..."));

		if(indexOf(reE3) == -1) {
			undoableCursor()->movePosition(QTextCursor::End);
			undoableCursor()->insertText($("..."));
		}

		if(cont) {
			staticRE$(reE4, "^\\s*\\.{3}[^\\.]?", REs | REu);
			staticRE$(reE5, "^\\s*\\.*\\s*", REs | REu);
			staticRE$(reE6, "\\.{3,3}\\s*$", REs | REu);
			if(*cont && indexOf(reE4) == -1)
				replace(reE5, $("..."));

			*cont = indexOf(reE6) != -1;
		}
	} else {
		if(cont) {
			staticRE$(reC1, "[?!\\)\\]'\\\"]\\s*$", REs | REu);
			staticRE$(reC2, "[^\\.]?\\.\\s*$", REs | REu);
			*cont = indexOf(reC1) == -1;
			if(!*cont)
				*cont = indexOf(reC2) == -1;
		}
	}
}

void
RichDocument::toLower()
{
	m_undoableCursor.beginEditBlock();
	for(QTextBlock bi = begin(); bi != end(); bi = bi.next()) {
		const QString &text = bi.text();
		for(int i = 0; i < text.length(); i++) {
			if(text.at(i).isUpper()) {
				m_undoableCursor.movePosition(QTextCursor::Start);
				m_undoableCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, bi.position() + i);
				m_undoableCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
				m_undoableCursor.insertText(QString(text.at(i).toLower()));
			}
		}
	}
	m_undoableCursor.endEditBlock();
}

void
RichDocument::toUpper()
{
	m_undoableCursor.beginEditBlock();
	for(QTextBlock bi = begin(); bi != end(); bi = bi.next()) {
		const QString &text = bi.text();
		for(int i = 0; i < text.length(); i++) {
			if(text.at(i).isLower()) {
				m_undoableCursor.movePosition(QTextCursor::Start);
				m_undoableCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, bi.position() + i);
				m_undoableCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
				m_undoableCursor.insertText(QString(text.at(i).toUpper()));
			}
		}
	}
	m_undoableCursor.endEditBlock();
}

void
RichDocument::toSentenceCase(bool *isSentenceStart, bool convertLowerCase, bool titleCase, bool testOnly)
{
	if(!testOnly)
		m_undoableCursor.beginEditBlock();
	for(QTextBlock bi = begin(); bi != end(); bi = bi.next()) {
		const QString &text = bi.text();
		bool wordStart = true;
		for(int i = 0; i < text.length(); i++) {
			const QChar &ch = text.at(i);
			const bool isSpace = ch.isSpace();
			const bool isEndPunct = !isSpace && (ch == QChar('.') || ch == QChar('?') || ch == QChar('!') || ch == QChar(ushort(0xbf)/*¿*/));
			if(!testOnly) {
				if(titleCase ? wordStart : *isSentenceStart) {
					if(ch.isLower()) {
						m_undoableCursor.movePosition(QTextCursor::Start);
						m_undoableCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, bi.position() + i);
						m_undoableCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
						m_undoableCursor.insertText(QString(ch.toUpper()));
					}
				} else if(convertLowerCase) {
					if(ch.isUpper()) {
						m_undoableCursor.movePosition(QTextCursor::Start);
						m_undoableCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, bi.position() + i);
						m_undoableCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
						m_undoableCursor.insertText(QString(ch.toLower()));
					}
				}
			}
			if(isSentenceStart) {
				if(isEndPunct)
					*isSentenceStart = true;
				else if(*isSentenceStart && !isSpace)
					*isSentenceStart = false;
			}
			wordStart = isSpace || isEndPunct
					|| (ch != QChar('-') && ch != QChar('_') && ch != QChar('\'') && ch.isPunct());
		}
	}
	if(!testOnly)
		m_undoableCursor.endEditBlock();
}

void
RichDocument::breakText(int minBreakLength)
{
	Q_ASSERT(minBreakLength >= 0);

	if(length() <= minBreakLength)
		return;

	const double center = double(length()) / 2.;
	double brkD = std::numeric_limits<double>::infinity();

	m_undoableCursor.beginEditBlock();
	while(blockCount() > 1) {
		m_undoableCursor.movePosition(QTextCursor::Start);
		m_undoableCursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
		m_undoableCursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
		m_undoableCursor.insertText($(" "));
	}
	const QString &text = firstBlock().text();
	for(int i = 0; i < text.length(); i++) {
		if(!text.at(i).isSpace())
			continue;
		const double nd = double(i) - center;
		if(qAbs(nd) < qAbs(brkD))
			brkD = nd;
	}
	if(qIsFinite(brkD)) {
		m_undoableCursor.movePosition(QTextCursor::Start);
		m_undoableCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, int(center + brkD));
		m_undoableCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
		m_undoableCursor.insertText($("\n"));
	}
	m_undoableCursor.endEditBlock();
}

void
RichDocument::replace(const QRegularExpression &search, const QString &replacement, bool replacementIsHtml)
{
	if(!search.isValid()) {
		qWarning("RichDocument::replace(): invalid QRegularExpression object");
		return;
	}

	const QString copy(toPlainText());
	QRegularExpressionMatchIterator matches = search.globalMatch(copy);
	if(!matches.hasNext()) // no matches at all
		return;

	QTextCursor readCur(this);
	QVector<EditChange> cl;

	const int numCaptures = search.captureCount();

	QVector<REBackrefFragment> backRefFrags(numCaptures);

	// build the backreferences vector with offsets in the replacement string
	QVector<REStringCapture> backRefs;
	// build replacement string fragments, so html is properly preserved
	QVector<QSharedPointer<QTextDocumentFragment>> repFrags;
	{
		const int al = replacement.length();
		const QChar *ac = replacement.unicode();
		RichDocument repDoc;
		if(replacementIsHtml)
			repDoc.setHtml(replacement);
		else
			repDoc.setPlainText(replacement);
		QTextCursor repCur(&repDoc);
		for(int i = 0; i < al - 1; i++) {
			if(ac[i] == QLatin1Char('\\')) {
				int no = ac[i + 1].digitValue();
				if(no > 0 && no <= numCaptures) {
					REStringCapture ref;
					ref.pos = i;
					ref.len = 2;

					if(i < al - 2) {
						const int secondDigit = ac[i + 2].digitValue();
						if(secondDigit != -1) {
							const int d = no * 10 + secondDigit;
							if(d <= numCaptures) {
								no = d;
								++ref.len;
							}
						}
					}

					ref.no = no;
					backRefs.append(ref);

					repCur.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, ref.pos - repCur.position());
					repFrags.push_back(QSharedPointer<QTextDocumentFragment>(new QTextDocumentFragment(repCur)));
					repCur.clearSelection();
					repCur.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, ref.len);
				}
			}
		}
		repCur.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
		repFrags.push_back(QSharedPointer<QTextDocumentFragment>(new QTextDocumentFragment(repCur)));
	}

	// iterate over matches
	while(matches.hasNext()) {
		QRegularExpressionMatch match = matches.next();
		const int matchStart = match.capturedStart();
		const int matchLen = match.capturedLength();
		int len;
		int repFrag = 0;
		// add the replacement string, with backreferences replaced
		for(const REStringCapture &backRef: qAsConst(backRefs)) {
			// part of the replacement string before the backreference
			if(!repFrags.at(repFrag)->isEmpty())
				cl.push_back(EditChange(ReplaceFragment, matchStart, 0, repFrags.at(repFrag)));
			repFrag++;

			// backreference inside the replacement string
			if((len = match.capturedLength(backRef.no))) {
				REBackrefFragment *brF = &backRefFrags[backRef.no - 1];
				const int pos = match.capturedStart(backRef.no);
				if(brF->frag.isNull() || pos != brF->pos || len != brF->len) {
					readCur.movePosition(QTextCursor::Start);
					readCur.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, pos);
					readCur.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, len);
					brF->frag.reset(new QTextDocumentFragment(readCur));
				}
				cl.push_back(EditChange(ReplaceFragment, matchStart, 0, brF->frag));
			}
		}

		// changes are applied in reverse, so match is overwritten last
		if(!repFrags.at(repFrag)->isEmpty()) {
			// last part of the replacement string
			cl.push_back(EditChange(ReplaceFragment, matchStart, matchLen, repFrags.at(repFrag)));
		} else if(matchLen) {
			// erase the matched part
			cl.push_back(EditChange(Delete, matchStart, matchLen));
		}
	}

	applyChanges(&cl);
}

void
RichDocument::replace(int index, int len, const QString &replacement)
{
	int oldLength = length();

	if(index < 0 || index >= oldLength)
		return;

	len = length(index, len);

	if(len == 0 && replacement.length() == 0)
		return;

	m_undoableCursor.beginEditBlock();
	m_undoableCursor.movePosition(QTextCursor::Start);
	m_undoableCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, index);
	m_undoableCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, len);
	m_undoableCursor.insertText(replacement);
	m_undoableCursor.endEditBlock();
}

RichDOM *
RichDocument::dom()
{
	if(m_domDirty) {
		m_dom->update(this);
		m_domDirty = false;
	}
	return m_dom;
}

RichDOM::Node *
RichDocument::nodeAt(quint32 pos, RichDOM::Node *root)
{
	RichDOM::Node *n = (root ? root : dom()->m_root)->children;
	while(n) {
		if(pos >= n->nodeStart && pos < n->nodeEnd) {
			RichDOM::Node *s;
			if(n->children && (s = nodeAt(pos, n)))
				return s;
			break;
		}
		n = n->next;
	}
	return n;
}

QString
RichDocument::crumbAt(RichDOM::Node *n)
{
	if(!n)
		return QString();

	QString crumb = n->cssSel();
	while(n->parent) {
		n = n->parent;
		crumb = n->cssSel() + $(" > ") + crumb;
	}
	return crumb;
}
