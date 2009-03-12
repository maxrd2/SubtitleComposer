#ifndef SCRIPTING_SUBTITLELINE_H
#define SCRIPTING_SUBTITLELINE_H

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

#include "../../core/subtitleline.h"

#include <QtCore/QObject>
#include <QtCore/QString>

namespace SubtitleComposer
{
	namespace Scripting
	{
		class SubtitleLineModule : public QObject
		{
			Q_OBJECT

			Q_ENUMS( TextTarget )
			Q_ENUMS( ErrorFlag )

			public:

				typedef enum {
					Primary =		SubtitleComposer::SubtitleLine::Primary,
					Secondary =		SubtitleComposer::SubtitleLine::Secondary,
					Both =			SubtitleComposer::SubtitleLine::Both
				} TextTarget;

				typedef enum {
					EmptyPrimaryText =					SubtitleComposer::SubtitleLine::EmptyPrimaryText,
					EmptySecondaryText =				SubtitleComposer::SubtitleLine::EmptySecondaryText,
					MaxPrimaryChars =					SubtitleComposer::SubtitleLine::MaxPrimaryChars,
					MaxSecondaryChars =					SubtitleComposer::SubtitleLine::MaxSecondaryChars,
					MaxPrimaryLines =					SubtitleComposer::SubtitleLine::MaxPrimaryLines,
					MaxSecondaryLines =					SubtitleComposer::SubtitleLine::MaxSecondaryLines,
					PrimaryUnneededSpaces =				SubtitleComposer::SubtitleLine::PrimaryUnneededSpaces,
					SecondaryUnneededSpaces =			SubtitleComposer::SubtitleLine::SecondaryUnneededSpaces,
					PrimaryUnneededDash =				SubtitleComposer::SubtitleLine::PrimaryUnneededDash,
					SecondaryUnneededDash =				SubtitleComposer::SubtitleLine::SecondaryUnneededDash,
					PrimaryCapitalAfterEllipsis =		SubtitleComposer::SubtitleLine::PrimaryCapitalAfterEllipsis,
					SecondaryCapitalAfterEllipsis =		SubtitleComposer::SubtitleLine::SecondaryCapitalAfterEllipsis,
					MinDurationPerPrimaryChar =			SubtitleComposer::SubtitleLine::MinDurationPerPrimaryChar,
					MinDurationPerSecondaryChar =		SubtitleComposer::SubtitleLine::MinDurationPerSecondaryChar,
					MaxDurationPerPrimaryChar =			SubtitleComposer::SubtitleLine::MaxDurationPerPrimaryChar,
					MaxDurationPerSecondaryChar =		SubtitleComposer::SubtitleLine::MaxDurationPerSecondaryChar,
					MinDuration =						SubtitleComposer::SubtitleLine::MinDuration,
					MaxDuration =						SubtitleComposer::SubtitleLine::MaxDuration,
					OverlapsWithNext =					SubtitleComposer::SubtitleLine::OverlapsWithNext,
					UntranslatedText = 					SubtitleComposer::SubtitleLine::UntranslatedText,
					UserMark = 							SubtitleComposer::SubtitleLine::UserMark,
					PrimaryOnlyErrors =					SubtitleComposer::SubtitleLine::PrimaryOnlyErrors,
					SecondaryOnlyErrors =				SubtitleComposer::SubtitleLine::SecondaryOnlyErrors,
					SharedErrors =						SubtitleComposer::SubtitleLine::SharedErrors,
					AllErrors =							SubtitleComposer::SubtitleLine::AllErrors,
					TimesErrors =						SubtitleComposer::SubtitleLine::TimesErrors,
					TextErrors =						SubtitleComposer::SubtitleLine::TextErrors,
				} ErrorFlag;

				SubtitleLineModule( QObject* parent=0 );
		};

		class SubtitleLine : public QObject
		{
			Q_OBJECT

			Q_PROPERTY( int number READ number )
			Q_PROPERTY( int index READ index )

			Q_PROPERTY( int primaryCharacters READ primaryCharacters )
			Q_PROPERTY( int primaryWords READ primaryWords )
			Q_PROPERTY( int primaryLines READ primaryLines )

			Q_PROPERTY( QObject* primaryText READ primaryText WRITE setPrimaryText )
			Q_PROPERTY( QString plainPrimaryText READ plainPrimaryText WRITE setPlainPrimaryText )
			Q_PROPERTY( QString richPrimaryText READ richPrimaryText WRITE setRichPrimaryText )

			Q_PROPERTY( int secondaryCharacters READ primaryCharacters )
			Q_PROPERTY( int secondaryWords READ secondaryWords )
			Q_PROPERTY( int secondaryLines READ secondaryLines )

			Q_PROPERTY( QObject* secondaryText READ secondaryText WRITE setSecondaryText )
			Q_PROPERTY( QString plainSecondaryText READ plainSecondaryText WRITE setPlainSecondaryText )
			Q_PROPERTY( QString richSecondaryText READ richSecondaryText WRITE setRichSecondaryText )

			Q_PROPERTY( int showTime READ showTime WRITE setShowTime )
			Q_PROPERTY( int hideTime READ hideTime WRITE setHideTime )
			Q_PROPERTY( int durationTime READ durationTime WRITE setDurationTime )

			Q_PROPERTY( int errorCount READ errorCount )
			Q_PROPERTY( int errorFlags READ errorFlags WRITE setErrorFlags )

			public slots:

				/// NOTE: target defaults to an invalid value, which means using and operation
				/// defined default value, generaly dependant on translationMode value.
				/// Also, setSecondaryText is a nop is ! translationMode

				int number() const;
				int index() const;

				QObject* prevLine() const;
				QObject* nextLine() const;

				int primaryCharacters() const;
				int primaryWords() const;
				int primaryLines() const;

				QObject* primaryText() const;
				void setPrimaryText( const QObject* text );
				QString plainPrimaryText() const;
				void setPlainPrimaryText( const QString& plainText );
				QString richPrimaryText() const;
				void setRichPrimaryText( const QString& richText );

				int secondaryCharacters() const;
				int secondaryWords() const;
				int secondaryLines() const;

				QObject* secondaryText() const;
				void setSecondaryText( const QObject* text );
				QString plainSecondaryText() const;
				void setPlainSecondaryText( const QString& plainText );
				QString richSecondaryText() const;
				void setRichSecondaryText( const QString& richText );

				void breakText( int minLengthForBreak, int target=-1 );
				void unbreakText( int target=-1 );
				void simplifyTextWhiteSpace( int target=-1 );

				/// all times or durations are specified in milliseconds

				int showTime() const;
				void setShowTime( int showTime );

				int hideTime() const;
				void setHideTime( int hideTime );

				int durationTime() const;
				void setDurationTime( int durationTime );

				int autoDuration( int msecsPerChar, int msecsPerWord, int msecsPerLine, int calculationTarget=-1 );

				void shiftTimes( int mseconds );
				void adjustTimes( int shiftMseconds, double scaleFactor );

				int errorCount() const;
				int errorFlags() const;
				void setErrorFlags( int errorFlags );
				void setErrorFlags( int errorFlags, bool value );

				bool checkEmptyPrimaryText( bool update=true );
				bool checkEmptySecondaryText( bool update=true );
				bool checkUntranslatedText( bool update=true );
				bool checkOverlapsWithNext( bool update=true );

				bool checkMinDuration( int minMsecs, bool update=true );
				bool checkMaxDuration( int maxMsecs, bool update=true );

				bool checkMinDurationPerPrimaryChar( int minMsecsPerChar, bool update=true );
				bool checkMinDurationPerSecondaryChar( int minMsecsPerChar, bool update=true );
				bool checkMaxDurationPerPrimaryChar( int maxMsecsPerChar, bool update=true );
				bool checkMaxDurationPerSecondaryChar( int maxMsecsPerChar, bool update=true );

				bool checkMaxPrimaryChars( int maxCharacters, bool update=true );
				bool checkMaxSecondaryChars( int maxCharacters, bool update=true );
				bool checkMaxPrimaryLines( int maxLines, bool update=true );
				bool checkMaxSecondaryLines( int maxLines, bool update=true );

				bool checkPrimaryUnneededSpaces( bool update=true );
				bool checkSecondaryUnneededSpaces( bool update=true );
				bool checkPrimaryCapitalAfterEllipsis( bool update=true );
				bool checkSecondaryCapitalAfterEllipsis( bool update=true );
				bool checkPrimaryUnneededDash( bool update=true );
				bool checkSecondaryUnneededDash( bool update=true );

				int check( int errorFlagsToCheck, int minDuration, int maxDuration,
						   int minDurationPerChar, int maxDurationPerChar,
						   int maxChars, int maxLines, bool update=true );

			private:

				friend class Subtitle;

				SubtitleLine( SubtitleComposer::SubtitleLine* backend, QObject* parent=0 );

				SubtitleComposer::SubtitleLine* m_backend;
		};
	}
}

#endif
