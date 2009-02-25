#ifndef SUBSTATIONALPHAOUTPUTFORMAT_H
#define SUBSTATIONALPHAOUTPUTFORMAT_H

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

#include "../outputformat.h"
#include "../../core/formatdata.h"
#include "../../core/subtitleiterator.h"

namespace SubtitleComposer
{
	class SubStationAlphaOutputFormat : public OutputFormat
	{
		friend class FormatManager;

		public:

			virtual ~SubStationAlphaOutputFormat() {}

			QString fromSString( const SString& string ) const
			{
				QString ret( string.richString() );

				ret.replace( "<b>", "{\\b1}" );
				ret.replace( "</b>", "{\\b0}" );
				ret.replace( "<i>", "{\\i1}" );
				ret.replace( "</i>", "{\\i0}" );
				ret.replace( "<u>", "{\\u1}" );
				ret.replace( "</u>", "{\\u0}" );
				ret.replace( "&lt;", "<" );
				ret.replace( "&gt;", ">" );
				ret.replace( "\n", "\\N" );

				return ret;
			}

			virtual QString dumpSubtitles( const Subtitle& subtitle, bool primary ) const
			{
				FormatData* formatData = this->formatData( subtitle );

				QString builder( "%1\n%2\n%3" );
	
				QString ret = builder
					.arg( formatData ? formatData->value( "ScriptInfo" ) : m_defaultScriptInfo )
					.arg( formatData ? formatData->value( "Styles" ) : m_defaultStyles )
					.arg( m_events );

				for ( SubtitleIterator it( subtitle ); it.current(); ++it )
				{
					const SubtitleLine* line = it.current();

					Time showTime = line->showTime();
					QString showTimeArg = builder.sprintf(
						"%01d:%02d:%02d.%02d",
						showTime.hours(),
						showTime.minutes(),
						showTime.seconds(),
						(showTime.mseconds() + 5)/10
					);

					Time hideTime = line->hideTime();
					QString hideTimeArg = builder.sprintf(
						"%01d:%02d:%02d.%02d",
						hideTime.hours(),
						hideTime.minutes(),
						hideTime.seconds(),
						(hideTime.mseconds() + 5)/10
					);

					formatData = this->formatData( line );

					ret += QString( formatData ? formatData->value( "Dialogue" ) : m_dialogueBuilder )
						.arg( showTimeArg )
						.arg( hideTimeArg )
						.arg( fromSString( primary ? line->primaryText() : line->secondaryText() ) );
				}

				return ret;
			}

		protected:

			SubStationAlphaOutputFormat():
				OutputFormat( "SubStation Alpha", QStringList( "ssa" ) ),
				m_defaultScriptInfo( s_defaultScriptInfo ),
				m_defaultStyles( s_defaultStyles ),
				m_events( s_events ),
				m_dialogueBuilder( s_dialogueBuilder )
			{
			}

			SubStationAlphaOutputFormat( const QString& name, const QStringList& extensions, const QString& scriptInfo, const QString& styles, const QString& events, const QString& dialogueBuilder ):
				OutputFormat( name, extensions ),
				m_defaultScriptInfo( scriptInfo ),
				m_defaultStyles( styles ),
				m_events( events ),
				m_dialogueBuilder( dialogueBuilder )
			{
			}

			const QString m_defaultScriptInfo;
			const QString m_defaultStyles;
			const QString m_events;
			const QString m_dialogueBuilder;
			const QString m_textBuilder;

			static const char* s_defaultScriptInfo;
			static const char* s_defaultStyles;
			static const char* s_events;
			static const char* s_dialogueBuilder;
	};

	const char* SubStationAlphaOutputFormat::s_defaultScriptInfo =
		"[Script Info]\n"
		"Title: Untitled\n"
		"ScriptType: V4.00\n"
		"Collisions: Normal\n"
		"Timer: 100\n"
		"WrapStyle: 0\n";

	const char* SubStationAlphaOutputFormat::s_defaultStyles =
		"[V4 Styles]\n"
		"Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, TertiaryColour, BackColour, "
		"Bold, Italic, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, AlphaLevel, Encoding\n"
		"Style: Default, Sans, 24, 16777215, 16777215, 16777215, 12632256, -1, 0, 1, 1, 1, 6, 30, 30, 415, 0, 0\n";

	const char* SubStationAlphaOutputFormat::s_events =
		"[Events]\n"
		"Format: Marked, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n";

	const char* SubStationAlphaOutputFormat::s_dialogueBuilder = "Dialogue: Marked=0,%1,%2,Default,,0000,0000,0000,,%3\n";

	class AdvancedSubStationAlphaOutputFormat : public SubStationAlphaOutputFormat
	{
		friend class FormatManager;

		protected:

			AdvancedSubStationAlphaOutputFormat():
				SubStationAlphaOutputFormat( "Advanced SubStation Alpha", QStringList( "ass" ), s_defaultScriptInfo, s_defaultStyles, s_events, s_dialogueBuilder )
			{
			}

			static const char* s_defaultScriptInfo;
			static const char* s_defaultStyles;
			static const char* s_events;
			static const char* s_dialogueBuilder;
	};

	const char* AdvancedSubStationAlphaOutputFormat::s_defaultScriptInfo =
		"[Script Info]\n"
		"Title: Untitled\n"
		"ScriptType: V4.00+\n"
		"Collisions: Normal\n"
		"Timer: 100\n"
		"WrapStyle: 0\n";

	const char* AdvancedSubStationAlphaOutputFormat::s_defaultStyles =
		"[V4+ Styles]\n"
		"Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeThrough, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding\n"
		"Style: Default, Sans, 16, &H00FFFFFF, &H00FFFFFF, &H00674436, &HFFFFFFF8, -1, 0, 0, 0, 100, 90, 0, 0, 1, 2.1, 0.5, 2, 15, 15, 15, 0\n";

	const char* AdvancedSubStationAlphaOutputFormat::s_events =
		"[Events]\n"
		"Format: Layer, Start, End, Style, Actor, MarginL, MarginR, MarginV, Effect, Text\n";

	const char* AdvancedSubStationAlphaOutputFormat::s_dialogueBuilder = "Dialogue: 0,%1,%2,Default,,0000,0000,0000,,%3\n";
}

#endif
