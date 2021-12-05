/*
    SPDX-FileCopyrightText: 2020-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RICHDOCUMENTTEST_H
#define RICHDOCUMENTTEST_H

#include "core/richtext/richdocument.h"

class RichDocumentTest : public QObject
{
	Q_OBJECT

	SubtitleComposer::RichDocument doc;

private slots:
	void testCursor();

	void testHtml_data();
	void testHtml();

	void testRegExpReplace_data();
	void testRegExpReplace();

	void testIndexReplace_data();
	void testIndexReplace();

	void testCleanupSpaces_data();
	void testCleanupSpaces();

	void testUpperLower();

	void testSentence_data();
	void testSentence();

	void testTitle_data();
	void testTitle();
};

#endif // RICHDOCUMENTTEST_H
