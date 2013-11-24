#ifndef MAIN_TESTS_H
#define MAIN_TESTS_H

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

#include "../core/range.h"
#include "../core/rangelist.h"
#include "../core/sstring.h"
#include "../core/subtitle.h"
#include "../core/subtitleline.h"
#include "../core/subtitleiterator.h"
#include "../core/audiolevels.h"

#include <QGlobal>
#include <QtCore/QString>

#include <KDebug>

using namespace SubtitleComposer;

void
showRanges(const RangeList &ranges)
{
	QString aux;
	for(RangeList::ConstIterator it = ranges.begin(), end = ranges.end(); it != end; ++it)
		aux += QString(" [%1,%2]").arg((*it).start()).arg((*it).end());

	kDebug() << QString("Showing ranges: %1").arg(aux.trimmed());
}

void
showSubtitle(const Subtitle &subtitle)
{
	kDebug() << "Showing subtitle";
	for(int index = 0, size = subtitle.linesCount(); index < size; ++index)
		kDebug() << QString("Line: %1").arg(subtitle.line(index)->primaryText().richString());
	kDebug() << "--------------------------";
}

void
iterateSubtitle(const Subtitle &subtitle, const RangeList &ranges)
{
	showRanges(ranges);

//  for ( int idx=0; idx < 3; ++idx )
	{
		kDebug() << "Iterating subtitle forwards from ranges";
		for(SubtitleIterator it(subtitle, ranges); it.current(); ++it)
			kDebug() << QString("Line: %1").arg(it.current()->primaryText().richString());

		kDebug() << "Iterating subtitle backwards from ranges";
		for(SubtitleIterator it(subtitle, ranges, true); it.current(); --it)
			kDebug() << QString("Line: %1").arg(it.current()->primaryText().richString());
	}

	kDebug() << "--------------------------";
}

void
testSubtitleIterator()
{
	Subtitle subtitle;

	for(int index = 0; index < 20; ++index)
		subtitle.insertLine(new SubtitleLine(QString("Line %1").arg(index)));

	showSubtitle(subtitle);

	iterateSubtitle(subtitle, Range::full());
	iterateSubtitle(subtitle, Range::lower(10));
	iterateSubtitle(subtitle, Range::upper(10));
	iterateSubtitle(subtitle, Range::lower(50));
	iterateSubtitle(subtitle, Range::upper(50));
	RangeList ranges;
	ranges << Range(1, 3);
	ranges << Range(5, 5);
	ranges << Range(11, 17);
	iterateSubtitle(subtitle, ranges);

//  SubtitleIterator it( subtitle, ranges, true );
//  for ( int idx = 0; idx < 100; ++idx )
//      ++it;
//  for ( int idx = 0; idx < 200; ++idx )
//      --it;

//  SubtitleIterator it( subtitle, Range( 1, 50 ), true );
//  for ( int idx = 0; idx < 100; ++idx )
//      ++it;
//  for ( int idx = 0; idx < 200; ++idx )
//      --it;

	/*QTime time;
	   time.start();

	   for ( SubtitleIterator it( subtitle, Range::full(), true ); it.current(); --it )
	   it.current()->text();

	   kDebug() << time.elapsed();

	   for ( SubtitleIterator it( subtitle, Range::full(), true ); it.current(); --it )
	   it.current()->text();

	   kDebug() << time.elapsed(); */
}

void
testAudioLevels()
{
/*	AudioLevels audiolevels;
        audiolevels.load( "/home/sergio/Desktop/scomp-trunk/SAMPLES/chasing.amy.cd1.wf" );
        audiolevels.save( "/home/sergio/Desktop/scomp-trunk/SAMPLES/chasing.amy.cd1.2.wf", true );*/
}

int
main(int, char **)
{
	// testSubtitleIterator();
	// testAudioLevels();
}

#endif
