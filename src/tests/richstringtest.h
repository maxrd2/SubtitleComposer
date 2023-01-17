/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RICHSTRINGTEST_H
#define RICHSTRINGTEST_H

#include <QObject>

class RichStringTest : public QObject
{
	Q_OBJECT

private slots:
	void testStyleFlags();
	void testLeftMidRight();
	void testInsert();
	void testReplace();
	void testStyleMerge();
};

#endif
