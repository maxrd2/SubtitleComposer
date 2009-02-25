/***************************************************************************
 *   Copyright (C) 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#include "rangetest.h"
#include "../range.h"

#include <QtTest> // krazy:exclude=c++/includes
#include <QtCore> // krazy:exclude=c++/includes

using namespace SubtitleComposer;

void RangeTest::testConstructors()
{
	QVERIFY( Range::MaxIndex > 0 );

	Range fullRange = Range::full();
	QVERIFY( fullRange.start() == 0 );
	QVERIFY( fullRange.end() == Range::MaxIndex );

	Range lowerRange = Range::lower( 16 );
	QVERIFY( lowerRange.start() == 0 );
	QVERIFY( lowerRange.end() == 16 );

	Range upperRange = Range::upper( 16 );
	QVERIFY( upperRange.start() == 16 );
	QVERIFY( upperRange.end() == Range::MaxIndex );

	Range range( 13, 32 );
	QVERIFY( range.start() == 13 );
	QVERIFY( range.end() == 32 );
}

void RangeTest::testLimits()
{
	QVERIFY( Range::MaxIndex > 0 );

	Range range( 13, 99 );
	QVERIFY( ! range.contains( 12 ) );
	QVERIFY( range.contains( 13 ) );
	QVERIFY( range.contains( 99 ) );
	QVERIFY( ! range.contains( 100 ) );
}

void RangeTest::testOperators()
{
	QVERIFY( Range::MaxIndex > 0 );

	Range range( 13, 99 );
	QVERIFY( range == Range( 13, 99 ) );
	QVERIFY( range != Range( 12, 99 ) );
	QVERIFY( range != Range( 13, 100 ) );
	QVERIFY( range != Range( 13, 100 ) );

	QVERIFY( Range( 11, 15 ) > Range( 8, 10 ) );
	QVERIFY( ! (Range( 11, 15 ) > Range( 8, 12 )) );
	QVERIFY( ! (Range( 7, 8 ) > Range( 8, 12 )) );

	QVERIFY( Range( 8, 15 ) >= Range( 7, 10 ) );
	QVERIFY( Range( 10, 15 ) >= Range( 7, 10 ) );
	QVERIFY( Range( 11, 15 ) >= Range( 7, 10 ) );
	QVERIFY( Range( 7, 10 ) >= Range( 7, 10 ) );
	QVERIFY( ! (Range( 7, 9 ) >= Range( 7, 10 )) );
	QVERIFY( ! (Range( 6, 11 ) >= Range( 7, 10 )) );
	QVERIFY( ! (Range( 7, 9 ) >= Range( 6, 10 )) );

	QVERIFY( Range( 8, 10 ) < Range( 11, 15 ) );
	QVERIFY( ! (Range( 8, 12 ) < Range( 11, 15 )) );
	QVERIFY( ! (Range( 8, 12 ) < Range( 7, 8 )) );

	QVERIFY( Range( 7, 10 ) <= Range( 8, 15 ) );
	QVERIFY( Range( 7, 10 ) <= Range( 10, 15 ) );
	QVERIFY( Range( 7, 10 ) <= Range( 11, 15 ) );
	QVERIFY( Range( 7, 10 ) <= Range( 7, 10 ) );
	QVERIFY( ! (Range( 7, 10 ) <= Range( 7, 9 )) );
	QVERIFY( ! (Range( 7, 10 ) <= Range( 6, 11 )) );
	QVERIFY( ! (Range( 6, 10 ) <= Range( 7, 9 )) );
}

QTEST_MAIN( RangeTest );

#include "rangetest.moc"
