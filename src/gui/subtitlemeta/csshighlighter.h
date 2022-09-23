/*
    SPDX-FileCopyrightText: 2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CSSHIGHLIGHTER_H
#define CSSHIGHLIGHTER_H

#include <QColor>
#include <QPalette>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

QT_FORWARD_DECLARE_CLASS(QTextDocument);

namespace SubtitleComposer {

class CSSHighlighter : public QSyntaxHighlighter
{
	Q_OBJECT

public:
	CSSHighlighter(QTextDocument *parent = 0);

protected:
	void highlightBlock(const QString &text) override;

private:
	void onPaletteChanged(const QPalette &pal);

private:
	QTextCharFormat m_commentFormat;
	QTextCharFormat m_attributeFormat;
	QTextCharFormat m_styleFormat;
	QTextCharFormat m_stringFormat;
	QTextCharFormat m_valueFormat;
};

}
#endif // CSSHIGHLIGHTER_H
