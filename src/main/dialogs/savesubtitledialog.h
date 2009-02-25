#ifndef SAVESUBTITLEDIALOG_H
#define SAVESUBTITLEDIALOG_H

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

#include "../formats/format.h"

#include <KEncodingFileDialog>

class KComboBox;

namespace SubtitleComposer
{
	class SaveSubtitleDialog : public KFileDialog
	{
		public:

			explicit SaveSubtitleDialog( bool primary, const QString& startDir=QString(), const QString& encoding=QString(), int newLine=Format::CurrentOS, const QString& format=QString(), QWidget* parent=0 );

			QString selectedEncoding() const;
			QString selectedFormat() const;

			Format::NewLine selectedNewLine() const;

			static QString outputFormatsFilter();

		public slots:

			int exec();

		private:

			void setCurrentFilter( const QString& formatName );

			KComboBox* m_encodingComboBox;
			KComboBox* m_newLineComboBox;
	};
}

#endif
