/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TIMETEST_H
#define TIMETEST_H

#include <QObject>

class TimeTest : public QObject
{
	Q_OBJECT

private slots:
	void testConstructors();
	void testSetters();
	void testOperators();
};

#endif
