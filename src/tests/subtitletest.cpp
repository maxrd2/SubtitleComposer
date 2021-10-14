/*
    SPDX-FileCopyrightText: 2021 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "subtitletest.h"

#include <QTest>

#include "application.h"
#include "core/richdocument.h"

using namespace SubtitleComposer;

void
SubtitleTest::testSort_data()
{
	QTest::addColumn<QVector<int>>("lines");

	QTest::newRow("ordered")
			<< (QVector<int>() << 1 << 2 << 3 << 4 << 5 << 6);
	QTest::newRow("inverse")
			<< (QVector<int>() << 6 << 5 << 4 << 3 << 2 << 1);
	QTest::newRow("random1")
			<< (QVector<int>() << 3 << 4 << 2 << 1 << 5 << 6);
	QTest::newRow("random2")
			<< (QVector<int>() << 3 << 4 << 1 << 2 << 6 << 5);
}

void
SubtitleTest::testSort()
{
	QFETCH(QVector<int>, lines);

	sub->removeLines(RangeList(Range::full()), SubtitleTarget::Both);

	for(int n: lines) {
		SubtitleLine *l = new SubtitleLine(n * 1000, n * 1000 + 500);
		l->primaryDoc()->setPlainText(QString::number(n));
		sub->insertLine(l);
	}

	QVERIFY(sub->count() == lines.size());
	for(int i = 0; i < sub->count(); i++)
		QVERIFY(qRound(sub->at(i)->showTime().toSeconds()) == i + 1);
}

#define QCoreApplication Application
QTEST_GUILESS_MAIN(SubtitleTest);
