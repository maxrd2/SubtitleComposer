/*
    SPDX-FileCopyrightText: 2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "csshighlighter.h"

#include <QDebug>
#include <QGuiApplication>
#include <QPalette>

using namespace SubtitleComposer;

union BlockState {
	int state;
	struct {
		quint16 indent;
		bool commentStart : 1;
		bool attributeStart : 1;
		bool styleStart : 1;
		bool stringStart : 1;
		bool valueStart : 1;
	};
};

CSSHighlighter::CSSHighlighter(QTextDocument *parent)
	: QSyntaxHighlighter(parent)
{
	onPaletteChanged();
}

static QColor
correctedColor(const QColor &color)
{
	int h, s, v;
	color.getHsv(&h, &s, &v);
	return QColor::fromHsv(h, s, qMin(v * 5 / 2, 255));
}

bool
CSSHighlighter::event(QEvent *ev)
{
	if(ev->type() == QEvent::ApplicationPaletteChange || ev->type() == QEvent::PaletteChange) {
		onPaletteChanged();
	}

	return QSyntaxHighlighter::event(ev);
}

void
CSSHighlighter::onPaletteChanged()
{
	const QPalette pal = QGuiApplication::palette();
	bool dark = pal.color(QPalette::Window).value() < pal.color(QPalette::WindowText).value();
	m_commentFormat.setForeground(dark ? correctedColor(Qt::cyan) : Qt::cyan);
	m_attributeFormat.setForeground(dark ? correctedColor(Qt::green) : Qt::green);
	m_styleFormat.setForeground(dark ? correctedColor(Qt::yellow) : Qt::yellow);
	m_stringFormat.setForeground(dark ? correctedColor(Qt::red) : Qt::red);
	m_valueFormat.setForeground(dark ? correctedColor(Qt::white) : Qt::white);
}

#define FMT_START(x) { x ## Start = i; }
#define FMT_END(x, pos) { setFormat(x ## Start, pos - x ## Start, m_ ## x ## Format); x ## Start = -1; }

void
CSSHighlighter::highlightBlock(const QString &text)
{
	// init state
	BlockState state = { .state = qMax(0, previousBlockState()) };
	int commentStart = state.commentStart ? 0 : -1;
	int attributeStart = state.attributeStart ? 0 : -1;
	int styleStart = state.styleStart ? 0 : -1;
	int stringStart = state.stringStart ? 0 : -1;
	int valueStart = state.valueStart ? 0 : -1;
	quint16 indent = state.indent;

	// process
	const int last = text.size() - 1;
	for(int i = 0; i <= last; i++) {
		const QChar &ch = text.at(i);
		if(commentStart != -1) {
			if(ch == QChar('*') && i != last && text.at(i + 1) == QChar('/')) {
				i++;
				FMT_END(comment, i + 1);
			}
		} else if(ch == QChar('/') && i != last && text.at(i + 1) == QChar('*')) {
			FMT_START(comment)
			i++;
		} else if(ch == QChar('{') && stringStart == -1) {
			indent++;
			if(attributeStart != -1)
				FMT_END(attribute, i);
			styleStart = -1;
			if(valueStart != -1)
				FMT_END(value, i);
		} else if(ch == QChar('}') && stringStart == -1) {
			indent--;
			styleStart = -1;
			if(valueStart != -1)
				FMT_END(value, i);
		} else if(indent == 0) {
			if(attributeStart == -1) {
				if(ch != QChar(','))
					FMT_START(attribute);
			} else {
				if(ch.isSpace() || ch == QChar(','))
					FMT_END(attribute, i);
			}
		} else {
			if(stringStart != -1) {
				if(ch == QChar('"')) {
					FMT_END(string, i + 1);
					if(valueStart != -1)
						valueStart = i != last ? i + 1 : -1;
				}
			} else if(valueStart != -1) {
				if(ch == QChar(';')) {
					FMT_END(value, i + 1);
				} else if(ch == QChar('"')) {
					FMT_START(string);
				}
			} else if(styleStart == -1) {
				if(!ch.isSpace() && ch != QChar(':'))
					FMT_START(style);
			} else {
				if(ch.isSpace() || ch == QChar(';')) {
					styleStart = -1;
				} else if(ch == QChar(':')) {
					FMT_END(style, i);
					while(i != last) {
						const QChar &ch = text.at(++i);
						if(ch.isSpace())
							continue;
						FMT_START(value);
						if(ch == QChar('"'))
							FMT_START(string);
						break;
					}
				}
			}

		}
	}

	// cleanup
	if(commentStart != -1)
		FMT_END(comment, last + 1);
	if(attributeStart != -1)
		setFormat(attributeStart, last - attributeStart + 1, m_attributeFormat);
	if(styleStart != -1)
		setFormat(styleStart, last - styleStart + 1, m_styleFormat);
	if(stringStart != -1)
		setFormat(stringStart, last - stringStart + 1, m_stringFormat);
	else if(valueStart != -1)
		setFormat(valueStart, last - valueStart + 1, m_valueFormat);

	// store state
	state.commentStart = commentStart != -1;
	state.attributeStart = attributeStart != -1;
	state.styleStart = styleStart != -1;
	state.stringStart = stringStart != -1;
	state.valueStart = valueStart != -1;
	state.indent = indent;
	setCurrentBlockState(state.state);
}
