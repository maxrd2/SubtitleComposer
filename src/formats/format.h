#ifndef FORMAT_H
#define FORMAT_H

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

#include "../core/formatdata.h"
#include "../core/subtitle.h"
#include "../core/subtitleline.h"

#include <QtCore/QString>
#include <QtCore/QStringList>

namespace SubtitleComposer
{
	class Format
	{
		public:

			friend class FormatManager;

			typedef enum {
				UNIX = 0,
				Windows,
				Macintosh,
#ifdef Q_OS_UNIX // krazy:exclude=c++/cpp
				CurrentOS = UNIX
#else
	#ifdef Q_OS_WIN // krazy:exclude=c++/cpp
				CurrentOS = Windows
	#else
				CurrentOS = Macintosh
	#endif
#endif
			} NewLine;

			Format( const QString& name, const QStringList& extensions ):
				m_name( name ),
				m_extensions( extensions ) {}
			virtual ~Format() {}

			const QString& name() const
			{
				return m_name;
			}

			const QStringList& extensions() const
			{
				return m_extensions;
			}

			bool knowsExtension( const QString& extension ) const
			{
				QString ext = extension.toLower();
				for ( QStringList::ConstIterator it = m_extensions.begin(), end = m_extensions.end(); it != end; ++it )
				{
					if ( *it == ext && *it != "*" )
						return true;
				}
				return false;
			}

		protected:

			FormatData createFormatData() const
			{
				return FormatData( m_name );
			}

			FormatData* formatData( const Subtitle& subtitle ) const
			{
				FormatData* formatData = subtitle.formatData();
				return formatData && formatData->formatName() == m_name ? formatData : 0;
			}

			void setFormatData( Subtitle& subtitle, FormatData* formatData ) const
			{
				subtitle.setFormatData( formatData );
			}

			void setFormatData( Subtitle& subtitle, FormatData& formatData ) const
			{
				subtitle.setFormatData( &formatData );
			}

			FormatData* formatData( const SubtitleLine* line ) const
			{
				FormatData* formatData = line->formatData();
				return formatData && formatData->formatName() == m_name ? formatData : 0;
			}

			void setFormatData( SubtitleLine* line, FormatData* formatData ) const
			{
				line->setFormatData( formatData );
			}

			void setFormatData( SubtitleLine* line, FormatData& formatData ) const
			{
				line->setFormatData( &formatData );
			}

			QString m_name;
			QStringList m_extensions;
	};
}

#endif
