/*
    SPDX-FileCopyrightText: 2021-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RICHCSSTEST_H
#define RICHCSSTEST_H

#include "core/richtext/richcss.h"

#include <QObject>

class RichCssTest : public QObject
{
	Q_OBJECT

	SubtitleComposer::RichCSS css;

private slots:
	void testParseSelector_data();
	void testParseSelector();

	void testParseRules_data();
	void testParseRules();
};

#endif // RICHCSSTEST_H
