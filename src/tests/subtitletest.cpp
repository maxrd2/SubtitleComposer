/*
 * Copyright (C) 2021 Mladen Milinkovic <max@smoothware.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "subtitletest.h"

#include <QTest>

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

	sub.removeLines(RangeList(Range::full()), SubtitleTarget::Both);

	for(int n: lines) {
		SubtitleLine *l = new SubtitleLine(n * 1000, n * 1000 + 500);
		l->primaryDoc()->setPlainText(QString::number(n));
		sub.insertLine(l);
	}

	QVERIFY(sub.count() == lines.size());
	for(int i = 0; i < sub.count(); i++)
		QVERIFY(qRound(sub.at(i)->showTime().toSeconds()) == i + 1);
}

QTEST_GUILESS_MAIN(SubtitleTest);
