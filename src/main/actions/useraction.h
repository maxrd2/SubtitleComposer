#ifndef USERACTION_H
#define USERACTION_H

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

#include <QtCore/QList>

#include <KAction>

namespace SubtitleComposer
{
	class Subtitle;
	class AudioLevels;
	class LinesWidget;
	class CurrentLineWidget;

	class UserAction : public QObject
	{
		Q_OBJECT

		public:

			typedef enum {
				SubClosed =				0x1,		// Subtitle is not opened
				SubOpened =				0x2,		// Subtitle is opened
				SubTrClosed =			0x4,		// Subtitle is not opened
				SubTrOpened =			0x8,		// Subtitle is opened
				SubHasLine =			0x10,		// Subtitle opened with, at least, one line
				SubHasLines =			0x20,		// Subtitle opened with, at least, two lines
				SubPDirty =				0x40,		// Subtitle opened and has unsaved changes
				SubPClean =				0x80,		// Subtitle opened or closed without unsaved changes
				SubSDirty =				0x100,		// Subtitle opened and has unsaved changes
				SubSClean =				0x200,		// Subtitle opened or closed without unsaved changes
				SubHasUndo =			0x400,		// An action can be undone
				SubHasRedo =			0x800,		// An action can be redone
				HasSelection =			0x1000,		// Subtitle opened with, at least, one line selected
				VideoClosed =			0x2000,
				VideoOpened =			0x4000,
				VideoStopped =			0x8000,
				VideoPlaying =			0x10000,
				AudioLevelsClosed =		0x20000,
				AudioLevelsOpened =		0x40000,
				FullScreenOn =			0x80000,
				FullScreenOff =			0x100000,

				SubtitleMask =  		SubClosed|SubOpened|SubTrClosed|SubTrOpened|
										SubPDirty|SubPClean|SubSDirty|SubSClean|
										SubHasLine|SubHasLines|
										SubHasUndo|SubHasRedo,
				SelectionMask =			HasSelection,
				VideoMask =				VideoClosed|VideoOpened|VideoStopped|VideoPlaying,
				AudioLevelsMask = 		AudioLevelsClosed|AudioLevelsOpened,
				FullScreenMask =		FullScreenOn|FullScreenOff,
				AllMask =				SubtitleMask|SelectionMask|VideoMask|AudioLevelsMask|FullScreenMask

			} EnableFlag;

			explicit UserAction( QAction* action, int enableFlags=SubOpened );

			QAction* action();
			int enableFlags();

			bool isEnabled();
			void setActionEnabled( bool enabled );
			void setContextFlags( int contextFlags );

		private:

			void updateEnabledState();

		private slots:

			void onActionChanged();

		private:

			QAction* m_action;
			int m_enableFlags;

			bool m_actionEnabled;
			bool m_contextEnabled;

			bool m_ignoreActionEnabledSignal;
	};

	class Player;

	class UserActionManager : public QObject
	{
		Q_OBJECT

		public:

			static UserActionManager* instance();

			void addAction( QAction* action, int enableFlags=UserAction::SubOpened );
			void addAction( UserAction* actionSpec );
			void removeAction( UserAction* actionSpec );

		public slots:

			void setSubtitle( Subtitle* subtitle=0 );
			void setLinesWidget( LinesWidget* linesWidget=0 );
			void setAudioLevels( AudioLevels* audiolevels=0 );
			void setPlayer( Player* player=0 );
			void setTranslationMode( bool translationMode );
			void setFullScreenMode( bool fullScreenMode );

		private:

			UserActionManager();

			void updateActionsContext( int contextFlags );

		private slots:

			void onSubtitleLinesChanged();
			void onPrimaryDirtyStateChanged( bool dirty );
			void onSecondaryDirtyStateChanged( bool dirty );
			void onUndoRedoStateChanged();
			void onLinesWidgetSelectionChanged();
			void onPlayerStateChanged();

		private:

			QList<UserAction*> m_actionSpecs;

			const Subtitle* m_subtitle;
			const LinesWidget* m_linesWidget;
			const Player* m_player;
			bool m_translationMode;

			int m_contextFlags;
	};
}

#endif
