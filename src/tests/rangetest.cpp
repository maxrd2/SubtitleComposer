/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rangetest.h"
#include "core/range.h"

#include <QTest>                               // krazy:exclude=c++/includes

using namespace SubtitleComposer;

void
RangeTest::testConstructors()
{
	QVERIFY(Range::MaxIndex > 0);

	Range fullRange = Range::full();
	QVERIFY(fullRange.start() == 0);
	QVERIFY(fullRange.end() == Range::MaxIndex);

	Range lowerRange = Range::lower(16);
	QVERIFY(lowerRange.start() == 0);
	QVERIFY(lowerRange.end() == 16);

	Range upperRange = Range::upper(16);
	QVERIFY(upperRange.start() == 16);
	QVERIFY(upperRange.end() == Range::MaxIndex);

	Range range(13, 32);
	QVERIFY(range.start() == 13);
	QVERIFY(range.end() == 32);
}

void
RangeTest::testLimits()
{
	QVERIFY(Range::MaxIndex > 0);

	Range range(13, 99);
	QVERIFY(!range.contains(12));
	QVERIFY(range.contains(13));
	QVERIFY(range.contains(99));
	QVERIFY(!range.contains(100));
}

void
RangeTest::testOperators()
{
	QVERIFY(Range::MaxIndex > 0);

	Range range(13, 99);
	QVERIFY(range == Range(13, 99));
	QVERIFY(range != Range(12, 99));
	QVERIFY(range != Range(13, 100));
	QVERIFY(range != Range(13, 100));

	QVERIFY(Range(11, 15) > Range(8, 10));
	QVERIFY(!(Range(11, 15) > Range(8, 12)));
	QVERIFY(!(Range(7, 8) > Range(8, 12)));

	QVERIFY(Range(8, 15) >= Range(7, 10));
	QVERIFY(Range(10, 15) >= Range(7, 10));
	QVERIFY(Range(11, 15) >= Range(7, 10));
	QVERIFY(Range(7, 10) >= Range(7, 10));
	QVERIFY(!(Range(7, 9) >= Range(7, 10)));
	QVERIFY(!(Range(6, 11) >= Range(7, 10)));
	QVERIFY(!(Range(7, 9) >= Range(6, 10)));

	QVERIFY(Range(8, 10) < Range(11, 15));
	QVERIFY(!(Range(8, 12) < Range(11, 15)));
	QVERIFY(!(Range(8, 12) < Range(7, 8)));

	QVERIFY(Range(7, 10) <= Range(8, 15));
	QVERIFY(Range(7, 10) <= Range(10, 15));
	QVERIFY(Range(7, 10) <= Range(11, 15));
	QVERIFY(Range(7, 10) <= Range(7, 10));
	QVERIFY(!(Range(7, 10) <= Range(7, 9)));
	QVERIFY(!(Range(7, 10) <= Range(6, 11)));
	QVERIFY(!(Range(6, 10) <= Range(7, 9)));
}

QTEST_GUILESS_MAIN(RangeTest);
