#ifndef MICRODVDOUTPUTFORMAT_H
#define MICRODVDOUTPUTFORMAT_H

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
#include "../../core/subtitleiterator.h"

namespace SubtitleComposer
{
	class MicroDVDOutputFormat : public OutputFormat
	{
		friend class FormatManager;

		public:

			virtual ~MicroDVDOutputFormat() {}

			virtual QString dumpSubtitles( const Subtitle& subtitle, bool primary ) const
			{
				QString ret;

				double framesPerSecond = subtitle.framesPerSecond();
				ret += m_lineBuilder.arg( 1 ).arg( 1 ).arg( "" ).arg( QString::number( framesPerSecond, 'f', 3 ) );

				for ( SubtitleIterator it( subtitle ); it.current(); ++it )
				{
					const SubtitleLine* line = it.current();

					const SString& text = primary ? line->primaryText() : line->secondaryText();

					ret += m_lineBuilder
						.arg( (long)((line->showTime().toMillis()/1000.0)*framesPerSecond + 0.5) )
						.arg( (long)((line->hideTime().toMillis()/1000.0)*framesPerSecond + 0.5) )
						.arg( m_stylesMap[text.cummulativeStyleFlags()] )
						.arg( text.string().replace( '\n', '|' ) );
				}

				return ret;
			}

		protected:

			MicroDVDOutputFormat():
				OutputFormat( "MicroDVD", QString( "sub:txt" ).split( ":" ) ),
				m_lineBuilder( "{%1}{%2}%3%4\n" ),
				m_stylesMap()
			{
				m_stylesMap[SString::Bold] = "{Y:b}";
				m_stylesMap[SString::Italic] = "{Y:i}";
				m_stylesMap[SString::Underline] = "{Y:u}";
				m_stylesMap[SString::Bold|SString::Italic] = "{Y:bi}";
				m_stylesMap[SString::Bold|SString::Underline] = "{Y:ub}";
				m_stylesMap[SString::Italic|SString::Underline] = "{Y:ui}";
				m_stylesMap[SString::Bold|SString::Italic|SString::Underline] = "{Y:ubi}";
			}

			const QString m_lineBuilder;
			mutable QMap<int,QString> m_stylesMap;
	};
}

#endif
