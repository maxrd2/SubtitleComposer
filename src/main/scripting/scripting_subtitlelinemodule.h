#ifndef SCRIPTING_SUBTITLELINEMODULE_H
#define SCRIPTING_SUBTITLELINEMODULE_H

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
	}
}

#endif
