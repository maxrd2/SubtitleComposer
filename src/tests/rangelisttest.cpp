/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "rangelisttest.h"
#include "core/rangelist.h"

#include <QTest>                               // krazy:exclude=c++/includes

using namespace SubtitleComposer;

void
RangeListTest::testConstructors()
{
	RangeList ranges;
	QVERIFY(ranges.isEmpty() && ranges.rangesCount() == 0);

	ranges << Range(1, 2);
	QVERIFY(ranges.firstIndex() == 1 && ranges.lastIndex() == 2 && ranges.indexesCount() == 2);

	RangeList ranges2(Range(1, 2));
	QVERIFY(ranges == ranges2);

	ranges2 = ranges;
	QVERIFY(ranges == ranges2);

	ranges << Range(7, 9);
	QVERIFY(ranges.firstIndex() == 1 && ranges.lastIndex() == 9 && ranges.indexesCount() == 5);

	RangeList ranges3(ranges);
	QVERIFY(ranges == ranges3);

	ranges3 = ranges;
	QVERIFY(ranges == ranges3);

	RangeList complementRanges = ranges.complement();
	QVERIFY(complementRanges.firstIndex() == 0);
	QVERIFY(complementRanges.lastIndex() == Range::MaxIndex);

	RangeList::ConstIterator complementRangesIt = complementRanges.begin();
	QVERIFY(*(complementRangesIt++) == Range(0, 0));
	QVERIFY(*(complementRangesIt++) == Range(3, 6));
	QVERIFY(*(complementRangesIt++) == Range(10, Range::MaxIndex));
}

void
RangeListTest::testJoinAndTrim()
{
	RangeList ranges;

	ranges << Range(1, 4);
	QVERIFY(ranges.rangesCount() == 1 && ranges.indexesCount() == 4);

	ranges << Range(3, 5);
	QVERIFY(ranges.rangesCount() == 1 && ranges.indexesCount() == 5);

	ranges << Range(7, 7);
	QVERIFY(ranges.rangesCount() == 2 && ranges.indexesCount() == 6);

	ranges << Range(13, 16);
	QVERIFY(ranges.rangesCount() == 3 && ranges.indexesCount() == 10);

	ranges << Range(6, 15);
	QVERIFY(ranges.rangesCount() == 1 && ranges.indexesCount() == 16);

	ranges.trimToRange(Range(0, 17));
	QVERIFY(ranges.rangesCount() == 1 && ranges.indexesCount() == 16);

	ranges.trimToRange(Range(0, 16));
	QVERIFY(ranges.rangesCount() == 1 && ranges.indexesCount() == 16);

	ranges.trimToRange(Range(0, 15));
	QVERIFY(ranges.rangesCount() == 1 && ranges.indexesCount() == 15);

	ranges.trimToRange(Range(0, 5));
	QVERIFY(ranges.rangesCount() == 1 && ranges.indexesCount() == 5);
}

QTEST_GUILESS_MAIN(RangeListTest);
