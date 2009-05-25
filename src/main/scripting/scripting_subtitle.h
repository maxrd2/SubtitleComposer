#ifndef SCRIPTING_SUBTITLE_H
#define SCRIPTING_SUBTITLE_H

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

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include "scripting_subtitleline.h"
#include "../../core/subtitle.h"
#include "../../core/rangelist.h"

#include <QtCore/QObject>

namespace SubtitleComposer
{
	class Subtitle;

	namespace Scripting
	{
		class Subtitle : public QObject
		{
			Q_OBJECT

			public slots:

				/// NOTE: target defaults to an invalid value, which means using and operation
				/// defined default value, generaly dependant on translationMode value.

				double framesPerSecond() const;
				void setFramesPerSecond( double framesPerSecond );

				bool isEmpty() const;
				int linesCount() const;
				int lastIndex() const;

				QObject* firstLine();
				QObject* lastLine();
				QObject* line( int index );

				void changeFramesPerSecond( double toFramesPerSecond, double fromFramesPerSecond=-1.0 );

				SubtitleLine* insertNewLine( int index, bool timeAfter, int target=-1 );
				void removeLine( int index, int target=-1 );
				void removeLines( const QObject* ranges, int target=-1 );

				void swapTexts( const QObject* ranges );

				void splitLines( const QObject* ranges );
				void joinLines( const QObject* ranges );

				void shiftLines( const QObject* ranges, int msecs );
				void adjustLines( const QObject* range, int firstTime, int lastTime );
				void sortLines( const QObject* range );

				void applyDurationLimits( const QObject* ranges, int minDuration, int maxDuration, bool canOverlap );
				void setMaximumDurations( const QObject* ranges );
				void setAutoDurations( const QObject* ranges, int msecsPerChar, int msecsPerWord, int msecsPerLine, bool canOverlap, int calculationTarget=-1 );

				void fixOverlappingLines( const QObject* ranges, int minInterval=100 );

				void fixPunctuation(const QObject* ranges, bool spaces, bool quotes, bool englishI, bool ellipsis, int target=-1);

				void lowerCase( const QObject* ranges, int target=-1 );
				void upperCase( const QObject* ranges, int target=-1 );
				void titleCase( const QObject* ranges, bool lowerFirst, int target=-1 );
				void sentenceCase( const QObject* ranges, bool lowerFirst, int target=-1 );

				void breakLines( const QObject* ranges, int minLengthForLineBreak, int target=-1 );
				void unbreakTexts( const QObject* ranges, int target=-1 );
				void simplifyTextWhiteSpace( const QObject* ranges, int target=-1 );

				void setMarked( const QObject* ranges, bool value );

				void clearErrors( const QObject* ranges, int errorFlags );

				void checkErrors( const QObject* ranges, int errorFlags, int minDuration, int maxDuration,
								  int minDurationPerChar, int maxDurationPerChar, int maxChars, int maxLines );

				void recheckErrors( const QObject* ranges, int minDuration, int maxDuration,
									int minDurationPerChar, int maxDurationPerChar, int maxChars, int maxLines );

			private:

				static SubtitleComposer::RangeList toRangesList( const QObject* object );

				friend class SubtitleModule;

				Subtitle( SubtitleComposer::Subtitle* backend, QObject* parent );

				SubtitleComposer::Subtitle* m_backend;
		};
	}
}

#endif
