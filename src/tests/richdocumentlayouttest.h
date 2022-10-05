/*
    SPDX-FileCopyrightText: 2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RICHDOCUMENTLAYOUTTEST_H
#define RICHDOCUMENTLAYOUTTEST_H

#include "core/richtext/richdocumentlayout.h"

#include <QObject>

class RichDocumentLayoutTest : public QObject
{
	Q_OBJECT

	SubtitleComposer::RichDocumentLayout richLayout;

public:
	RichDocumentLayoutTest();

private slots:
	void testMerge_data();
	void testMerge();
};

#endif // RICHDOCUMENTLAYOUTTEST_H
