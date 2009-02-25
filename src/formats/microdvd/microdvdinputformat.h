#ifndef MICRODVDINPUTFORMAT_H
#define MICRODVDINPUTFORMAT_H

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
	class MicroDVDInputFormat : public InputFormat
	{
		friend class FormatManager;

		public:

			virtual ~MicroDVDInputFormat() {}

			virtual bool parseSubtitles( Subtitle& subtitle, const QString& data ) const
			{
				if ( m_lineRegExp.indexIn( data, 0 ) == -1 )
					return false; // couldn't find first line (content or FPS)

				int offset = 0;

				// if present, the FPS must by indicated by the first entry with both initial and final frames at 1
				bool ok;
				double framesPerSecond = m_lineRegExp.cap( 3 ).toDouble( &ok );
				if ( ok && m_lineRegExp.cap( 1 ) == "1" && m_lineRegExp.cap( 2 ) == "1" )
				{
					 // first line contained the frames per second
					subtitle.setFramesPerSecond( framesPerSecond );

					offset += m_lineRegExp.matchedLength();
					if ( m_lineRegExp.indexIn( data, offset ) == -1 )
						return false; // couldn't find first line with content
				}
				else // first line doesn't contain the FPS, use the value loaded by default
					framesPerSecond = subtitle.framesPerSecond();


				unsigned readLines = 0;

				do
				{
					offset += m_lineRegExp.matchedLength();

					Time showTime( (long)((m_lineRegExp.cap( 1 ).toLong() / framesPerSecond)*1000) );
					Time hideTime( (long)((m_lineRegExp.cap( 2 ).toLong() / framesPerSecond)*1000) );

					int styleFlags = 0;
					QString text = m_lineRegExp.cap( 3 ).replace( "|", "\n" );
					if ( m_styleRegExp.indexIn( text ) != -1 )
					{
						QString styleText( m_styleRegExp.cap( 1 ) );
						if ( styleText.contains( 'b', Qt::CaseInsensitive ) )
							styleFlags |= SString::Bold;
						if ( styleText.contains( 'i', Qt::CaseInsensitive ) )
							styleFlags |= SString::Italic;
						if ( styleText.contains( 'u', Qt::CaseInsensitive ) )
							styleFlags |= SString::Underline;

						text.remove( m_styleRegExp );
					}
					text.replace( m_unsupportedFormatRegExp, "" );

					subtitle.insertLine( new SubtitleLine( SString( text, styleFlags ), showTime, hideTime ) );

					readLines++;
				}
				while ( m_lineRegExp.indexIn( data, offset ) != -1 );

				return readLines > 0;
			}

		protected:

			MicroDVDInputFormat():
				InputFormat( "MicroDVD", QString( "sub:txt" ).split( ":" ) ),
				m_lineRegExp( "\\{(\\d+)\\}\\{(\\d+)\\}([^\n]+)\n", Qt::CaseInsensitive ),
				m_styleRegExp( "(\\{y:[ubi]+\\})", Qt::CaseInsensitive ),
				m_unsupportedFormatRegExp( "\\{C:\\$[0-9a-f]{6,6}\\}", Qt::CaseInsensitive )
			{
			}

			mutable QRegExp m_lineRegExp;
			mutable QRegExp m_styleRegExp;
			mutable QRegExp m_unsupportedFormatRegExp;
	};
}

#endif

