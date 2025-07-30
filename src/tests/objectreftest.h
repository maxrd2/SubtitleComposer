/*
    SPDX-FileCopyrightText: 2025 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef OBJECTREFTEST_H
#define OBJECTREFTEST_H

#include <QObject>

class ObjectRefTest : public QObject
{
	Q_OBJECT

public:
	ObjectRefTest();
	virtual ~ObjectRefTest();

private slots:
	void test();
};

#endif // OBJECTREFTEST_H
