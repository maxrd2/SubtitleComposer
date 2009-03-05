#ifndef ERRORTRACKER_H
#define ERRORTRACKER_H

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

#include "../../core/subtitle.h"

namespace SubtitleComposer
{
	class SubtitleLine;

	class ErrorTracker : public QObject
	{
		Q_OBJECT

		public:

			explicit ErrorTracker( QObject* parent=0 );
			virtual ~ErrorTracker();

			bool isTracking() const;

		public slots:

			void setSubtitle( Subtitle* subtitle=0 );

		private:

			void connectSlots();
			void disconnectSlots();

			void updateLineErrors( SubtitleLine* line, int errorFlags ) const;

		private slots:

			void onLinePrimaryTextChanged( SubtitleLine* line );
			void onLineSecondaryTextChanged( SubtitleLine* line );
			void onLineTimesChanged( SubtitleLine* line );

			void onErrorsOptionChanged( const QString& optionName, const QString& value );

		private:

			Subtitle* m_subtitle;

			bool m_autoClearFixed;
			int m_minDuration;
			int m_maxDuration;
			int m_minDurationPerChar;
			int m_maxDurationPerChar;
			int m_maxCharacters;
			int m_maxLines;
	};
}

#endif
