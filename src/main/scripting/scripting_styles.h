#ifndef SCRIPTING_STYLES_H
#define SCRIPTING_STYLES_H

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

#include "../../core/sstring.h"

#include <QtCore/QObject>

namespace SubtitleComposer
{
	namespace Scripting
	{

		class StylesModule : public QObject
		{
			Q_OBJECT

			Q_ENUMS( StyleFlag )

			public:

				typedef enum {
					Bold =				SubtitleComposer::SString::Bold,
					Italic =			SubtitleComposer::SString::Italic,
					Underline = 		SubtitleComposer::SString::Underline,
					StrikeThrough = 	SubtitleComposer::SString::StrikeThrough,
				} StyleFlag;

				StylesModule( QObject* parent=0 );

			public slots:

				int cummulativeFlags( const QString& text ) const;

				QString setFlags( const QString& text, int styleFlags ) const;
				QString setFlags( const QString& text, int styleFlags, bool on ) const;
				QString setFlags( const QString& text, int index, int len, int styleFlags ) const;
				QString setFlags( const QString& text, int index, int len, int styleFlags, bool on ) const;
		};
	}
}

#endif
