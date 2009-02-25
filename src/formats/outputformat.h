#ifndef OUTPUTFORMAT_H
#define OUTPUTFORMAT_H

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

#include "format.h"
#include "../common/filesavehelper.h"

#include <QtCore/QFile>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>

namespace SubtitleComposer
{
	class OutputFormat : public Format
	{
		public:

			virtual ~OutputFormat() {}

			bool writeSubtitle( const Subtitle& subtitle, NewLine newLine, bool primary, const KUrl& url, QTextCodec* codec, bool overwrite ) const
			{
				FileSaveHelper fileSaveHelper( url, overwrite );

				if ( ! fileSaveHelper.open() )
					return false;

				QString data = dumpSubtitles( subtitle, primary );
				if ( newLine == Windows )
					data.replace( "\n", "\r\n" );
				else if ( newLine == Macintosh )
					data.replace( "\n", "\r" );

				QTextStream stream( fileSaveHelper.file() );
				stream.setCodec( codec );
				stream << data;

				return fileSaveHelper.close();
			}

			virtual QString dumpSubtitles( const Subtitle& subtitle, bool secondary ) const = 0;

		protected:

			OutputFormat( const QString& name, const QStringList& extensions ):
				Format( name, extensions ) {}
	};

}

#endif
