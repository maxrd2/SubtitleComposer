#ifndef AUDIOLEVELSWIDGET_H
#define AUDIOLEVELSWIDGET_H

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

#include "../core/time.h"
#include "../core/subtitle.h"
#include "../core/audiolevels.h"

#include <QtGui/QWidget>

class QRegion;
class QPolygon;

namespace SubtitleComposer
{
	class AudioLevelsWidget : public QWidget
	{
		Q_OBJECT

		public:

			AudioLevelsWidget( QWidget* parent );
			virtual ~AudioLevelsWidget();

			void loadConfig();
			void saveConfig();

			Time windowSize() const;
			void setWindowSize( const Time& size );

		public slots:

			void setSubtitle( Subtitle* subtitle=0 );
			void setAudioLevels( AudioLevels* audiolevels=0 );

		protected:

			virtual void paintEvent( QPaintEvent* );
			virtual void resizeEvent( QResizeEvent* e );

			void rebuildRegions();

		private slots:

			void onPlayerPositionChanged( double seconds );

		private:

			Subtitle* m_subtitle;
			AudioLevels* m_audiolevels;

			Time m_lowerPosition;
			Time m_playingPosition;
			Time m_upperPosition;

			QRegion* m_regions;
			QPolygon* m_points;
			int m_playingX;
	};

}

#endif
