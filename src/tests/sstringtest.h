#ifndef SSTRINGTEST_H
#define SSTRINGTEST_H

/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QObject>

class SStringTest : public QObject
{
	Q_OBJECT

private slots:
	void testStyleFlags();
	void testLeftMidRight();
	void testInsert();
	void testReplace();
};

#endif
