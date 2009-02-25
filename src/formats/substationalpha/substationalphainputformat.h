#ifndef SUBSTATIONALPHAINPUTFORMAT_H
#define SUBSTATIONALPHAINPUTFORMAT_H

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

#include "../inputformat.h"

#include <QtCore/QRegExp>

namespace SubtitleComposer
{
	class SubStationAlphaInputFormat : public InputFormat
	{
		friend class FormatManager;
		friend class AdvancedSubStationAlphaInputFormat;

		public:

			virtual ~SubStationAlphaInputFormat() {}

			SString toSString( QString string ) const
			{
				static QRegExp cmdRegExp( "\\{([^\\}]+)\\}" );
				static QRegExp tagCmdRegExp( "\\[ibu][01]" );

				SString ret;

				string.replace( "\\N", "\n" );
				string.replace( "\\n", "\n" );

				int currentStyle = 0;

				int offsetPos = 0, matchedPos;
				while ( (matchedPos = cmdRegExp.indexIn( string, offsetPos ) ) != -1 )
				{
					int newStyleFlags = currentStyle;

					QString commands( cmdRegExp.cap( 1 ) );
					QStringList commandsList( commands.split( '\\' ) );
					for ( QStringList::ConstIterator it = commandsList.begin(), end = commandsList.end(); it != end; ++it )
					{
						if ( *it == "\\il" )
							newStyleFlags |= SString::Italic;
						else if ( *it == "\\bl" )
							newStyleFlags |= SString::Bold;
						else if ( *it == "\\ul" )
							newStyleFlags |= SString::Underline;
						else if ( *it == "\\i0" )
							newStyleFlags &= ~SString::Italic;
						else if ( *it == "\\b0" )
							newStyleFlags &= ~SString::Bold;
						else if ( *it == "\\u0" )
							newStyleFlags &= ~SString::Underline;
					}
	
					commands.replace( tagCmdRegExp, "" );

					QString token( string.mid( offsetPos, matchedPos - offsetPos ) );
					if ( commands.length() )
						token += "{" + commands + "}";

					ret.append( SString( token, currentStyle ) );
					currentStyle = newStyleFlags;

					offsetPos = matchedPos + cmdRegExp.matchedLength();
				}

				ret.append( SString( string.mid( offsetPos, matchedPos - offsetPos ), currentStyle ) );

				return ret;
			}

			virtual bool parseSubtitles( Subtitle& subtitle, const QString& data ) const
			{
				if ( m_scriptInfoRegExp.indexIn( data ) == -1 )
					return false;

				int stylesStart = m_stylesRegExp.indexIn( data );
				if ( stylesStart == -1 )
					return false;

				FormatData formatData = createFormatData();

				formatData.setValue( "ScriptInfo", data.mid( 0, stylesStart - 1 ) );

				int eventsStart = m_eventsRegExp.indexIn( data, stylesStart );
				if ( eventsStart == -1 )
					return false;

				formatData.setValue( "Styles", data.mid( stylesStart, eventsStart - stylesStart ) );

				if ( m_formatRegExp.indexIn( data, eventsStart ) == -1 )
					return false;

				setFormatData( subtitle, formatData );
				formatData.clear();

				unsigned readLines = 0;

				int offset = m_formatRegExp.pos() + m_formatRegExp.matchedLength();
				for ( ; m_dialogueRegExp.indexIn( data, offset ) != -1; offset += m_dialogueRegExp.matchedLength() )
				{
					if ( m_timeRegExp.indexIn( m_dialogueRegExp.cap( 1 ) ) == -1 )
						continue;

					Time showTime(
						m_timeRegExp.cap( 1 ).toInt(),
						m_timeRegExp.cap( 2 ).toInt(),
						m_timeRegExp.cap( 3 ).toInt(),
						m_timeRegExp.cap( 4 ).toInt() * 10
					);

					if ( m_timeRegExp.indexIn( m_dialogueRegExp.cap( 2 ) ) == -1 )
						continue;

					Time hideTime(
						m_timeRegExp.cap( 1 ).toInt(),
						m_timeRegExp.cap( 2 ).toInt(),
						m_timeRegExp.cap( 3 ).toInt(),
						m_timeRegExp.cap( 4 ).toInt() * 10
					);

					SubtitleLine* line = new SubtitleLine( toSString( m_dialogueRegExp.cap( 3 ) ), showTime, hideTime );

					formatData.setValue(
						"Dialogue",
						m_dialogueRegExp.cap( 0 ).replace( m_dialogue2RegExp, "\\1%1\\2%2\\3%3\n" )
					);

					setFormatData( line, formatData );

					subtitle.insertLine( line );

					readLines++;
				}

				return readLines > 0;
			}

		protected:

			SubStationAlphaInputFormat():
				InputFormat( "SubStation Alpha", QStringList( "ssa" ) ),
				m_scriptInfoRegExp( "^ *\\[Script Info\\] *\n" ),
				m_stylesRegExp( s_stylesRegExp ),
				m_eventsRegExp( "\n\\ *\\[Events\\] *\n" ),
				m_formatRegExp( s_formatRegExp ),
				m_dialogueRegExp( s_dialogueRegExp ),
				m_dialogue2RegExp( s_dialogue2RegExp ),
				m_timeRegExp( "([0-9]):([0-5][0-9]):([0-5][0-9]).([0-9][0-9])" )
			{
			}

			SubStationAlphaInputFormat( const QString& name, const QStringList& extensions, const QString& stylesRegExp, const QString& formatRegExp ):
				InputFormat( name, extensions ),
				m_scriptInfoRegExp( "^ *\\[Script Info\\] *\n" ),
				m_stylesRegExp( stylesRegExp ),
				m_eventsRegExp( "\n\\ *\\[Events\\] *\n" ),
				m_formatRegExp( formatRegExp ),
				m_dialogueRegExp( s_dialogueRegExp ),
				m_dialogue2RegExp( s_dialogue2RegExp ),
				m_timeRegExp( "([0-9]):([0-5][0-9]):([0-5][0-9]).([0-9][0-9])" )
			{
			}

			mutable QRegExp m_scriptInfoRegExp;
			mutable QRegExp m_stylesRegExp;
			mutable QRegExp m_eventsRegExp;
			mutable QRegExp m_formatRegExp;
			mutable QRegExp m_dialogueRegExp;
			mutable QRegExp m_dialogue2RegExp;
			mutable QRegExp m_timeRegExp;

			static const char* s_stylesRegExp;
			static const char* s_formatRegExp;
			static const char* s_dialogueRegExp;
			static const char* s_dialogue2RegExp;
	};

	const char* SubStationAlphaInputFormat::s_stylesRegExp = "\n\\ *\\[V4 Styles\\] *\n";

	const char* SubStationAlphaInputFormat::s_formatRegExp =
		" *Format: *Marked, *Start, *End, *Style, *Name, *MarginL, *MarginR, *MarginV, *Effect, *Text *\n";

	const char* SubStationAlphaInputFormat::s_dialogueRegExp =
		" *Dialogue: *[^,]+, *([^,]+), *([^,]+), *[^,]+, *[^,]*, *[0-9]{4,4}, *[0-9]{4,4}, *[0-9]{4,4}, *[^,]*, *([^\n]*)\n?";

	const char* SubStationAlphaInputFormat::s_dialogue2RegExp =
		" *(Dialogue: *[^,]+, *)[^,]+(, *)[^,]+(, *[^,]+, *[^,]*, *[0-9]{4,4}, *[0-9]{4,4}, *[0-9]{4,4}, *[^,]*, *).*";

	class AdvancedSubStationAlphaInputFormat : public SubStationAlphaInputFormat
	{
		friend class FormatManager;

		protected:

			AdvancedSubStationAlphaInputFormat():
				SubStationAlphaInputFormat( "Advanced SubStation Alpha", QStringList( "ass" ), s_stylesRegExp, s_formatRegExp )
			{
			}

			static const char* s_stylesRegExp;
			static const char* s_formatRegExp;
	};

	const char* AdvancedSubStationAlphaInputFormat::s_stylesRegExp = "\n\\ *\\[V4\\+ Styles\\] *\n";

	const char* AdvancedSubStationAlphaInputFormat::s_formatRegExp =
		" *Format: *Layer, *Start, *End, *Style, *(Actor|Name), *MarginL, *MarginR, *MarginV, *Effect, *Text *\n";
// Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text
}

#endif

