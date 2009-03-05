#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

#include "../core/subtitle.h"

#include <KXmlGuiWindow>

namespace SubtitleComposer
{
	class PlayerWidget;
	class LinesWidget;
	class CurrentLineWidget;
	class AudioLevelsWidget;

	class MainWindow : public KXmlGuiWindow
	{
		Q_OBJECT

		friend class Application;

		public:

			MainWindow();
			virtual ~MainWindow();

			void loadConfig();
			void saveConfig();

		public slots:

			void setSubtitle( Subtitle* subtitle=0 );

		protected:

			virtual bool queryClose();

		protected:

			//AudioLevelsWidget* m_audiolevelsWidget;
			PlayerWidget* m_playerWidget;
			LinesWidget* m_linesWidget;
			CurrentLineWidget* m_curLineWidget;
	};
}

#endif
