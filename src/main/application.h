#ifndef APPLICATION_H
#define APPLICATION_H

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

#include "mainwindow.h"
#include "configs/generalconfig.h"
#include "configs/spellingconfig.h"
#include "configs/errorsconfig.h"
#include "configs/playerconfig.h"
#include "../core/subtitle.h"
#include "../core/audiolevels.h"
#include "../config/appconfig.h"

#include <QtCore/QMap>
#include <QtGui/QKeySequence>

#include <KApplication>
#include <KAction>

class KComboBox;
class KAction;
class KToggleAction;
class KRecentFilesActionExt;
class KCodecAction;
class KSelectAction;

namespace SubtitleComposer
{
	class Player;
	class AudioLevels;

	class AudioLevelsWidget;
	class PlayerWidget;
	class LinesWidget;
	class CurrentLineWidget;
	class ConfigDialog;
	class ErrorsDialog;
	class ErrorsWidget;

	class Finder;
	class Replacer;
	class ErrorFinder;
	class Speller;
	class ErrorTracker;

	class ScriptsManager;

	class Application : public KApplication
	{
		Q_OBJECT

		public:

			Application();
			virtual ~Application();

			static Application* instance();

			Subtitle* subtitle() const;

			MainWindow* mainWindow() const;
			LinesWidget* linesWidget() const;

			bool translationMode() const;
			bool showingLinesContextMenu() const;

			void loadConfig();
			void saveConfig();

			GeneralConfig* generalConfig() { return static_cast<GeneralConfig*>( m_config.group( "General" ) ); }
			SpellingConfig* spellingConfig() { return static_cast<SpellingConfig*>( m_config.group( "Spelling" ) ); }
			ErrorsConfig* errorsConfig() { return static_cast<ErrorsConfig*>( m_config.group( "Errors" ) ); }
			PlayerConfig* playerConfig() { return static_cast<PlayerConfig*>( m_config.group( "Player" ) ); }

			QAction* action( const char* actionName );
			void triggerAction( const QKeySequence& keySequence );

			const QStringList& availableEncodingNames() const;

			const KUrl& lastSubtitleDirectory() const;

		public slots:

			/// BEGIN ACTIONS HANDLERS

			void undo();
			void redo();

			void newSubtitle();
			void openSubtitle();
			void openSubtitle( const KUrl& url, bool warnClashingUrls=true );
			bool saveSubtitle();
			bool saveSubtitleAs();
			bool closeSubtitle();

			void newTrSubtitle();
			void openTrSubtitle();
			void openTrSubtitle( const KUrl& url, bool warnClashingUrls=true );
			bool saveTrSubtitle();
			bool saveTrSubtitleAs();
			bool closeTrSubtitle();

			void openSubtitleWithDefaultEncoding();
			void changeSubtitlesEncoding( const QString& encoding );

			void joinSubtitles();
			void splitSubtitle();

			void insertBeforeCurrentLine();
			void insertAfterCurrentLine();
			void removeSelectedLines();

			void joinSelectedLines();
			void splitSelectedLines();

			void selectAllLines();
			void gotoLine();

			void find();
			void findNext();
			void findPrevious();
			void replace();

			void spellCheck();

			void findError();
			void findNextError();
			void findPreviousError();

			void retrocedeCurrentLine();
			void advanceCurrentLine();

			void checkErrors();
			void recheckAllErrors();
			void recheckSelectedErrors();
			void clearErrors();
			void clearSelectedErrors( bool includeMarks=false );
			void clearSelectedMarks();

			void showErrors();
			void showErrorsConfig();

			void toggleSelectedLinesMark();
			void toggleSelectedLinesBold();
			void toggleSelectedLinesItalic();
			void toggleSelectedLinesUnderline();
			void toggleSelectedLinesStrikeThrough();

			void shiftLines();
			void shiftSelectedLinesForwards();
			void shiftSelectedLinesBackwards();
			void adjustLines();

			void sortLines();

			void changeFrameRate();
			void enforceDurationLimits();
			void setAutoDurations();
			void maximizeDurations();
			void fixOverlappingLines();
			void syncWithSubtitle();

			void breakLines();
			void unbreakTexts();
			void simplifySpaces();
			void changeCase();
			void fixPunctuation();
			void translate();

			void openVideo();
			void openVideo( const KUrl& url );

			void toggleFullScreenMode();
			void setFullScreenMode( bool enabled );

			void seekBackwards();
			void seekForwards();
			void seekToPrevLine();
			void seekToNextLine();

			void setCurrentLineShowTimeFromVideo();
			void setCurrentLineHideTimeFromVideo();

			void shiftToVideoPosition();
			void adjustToVideoPositionAnchorLast();
			void adjustToVideoPositionAnchorFirst();

			void openAudioLevels();
			void openAudioLevels( const KUrl& url );
			void saveAudioLevelsAs();
			void closeAudioLevels();

			void increaseAudioLevelsVZoom();
			void decreaseAudioLevelsVZoom();
			void increaseAudioLevelsHZoom();
			void decreaseAudioLevelsHZoom();

			/// END ACTIONS HANDLERS

		signals:

			void subtitleOpened( Subtitle* subtitle );
			void subtitleClosed();

			void audiolevelsOpened( AudioLevels* audiolevels );
			void audiolevelsClosed();

			void translationModeChanged( bool value );
			void fullScreenModeChanged( bool value );

		private:

			QString encodingForUrl( const KUrl& url );

			bool acceptClashingUrls( const KUrl& subtitleUrl, const KUrl& subtitleTrUrl );

			KUrl saveSplittedSubtitle( const Subtitle& subtitle, const KUrl& srcUrl, QString encoding, QString format, bool primary );

			void setupActions();

			Time videoPosition( bool compensate=false );

			static const QString& buildMediaFilesFilter();
			static const QString& buildLevelsFilesFilter();

			bool applyTranslation( RangeList ranges, bool primary, int inputLanguage, int outputLanguage, int textTargets );

		private slots:

			void updateTitle();
			void updateUndoRedoToolTips();

			void onLineDoubleClicked( SubtitleLine* line );
			void onHighlightLine( SubtitleLine* line, bool primary=true, int firstIndex=-1, int lastIndex=-1 );
			void onPlayingLineChanged( SubtitleLine* line );
			void onLinkCurrentLineToVideoToggled( bool value );

			void onPlayerFileOpened( const QString& filePath );
			void onPlayerPlaying();
			void onPlayerPaused();
			void onPlayerStopped();
			void onPlayerAudioStreamsChanged( const QStringList& audioStreams );
			void onPlayerActiveAudioStreamChanged( int audioStream );
			void onPlayerMuteChanged( bool muted );
			void onPlayerOptionChanged( const QString& option, const QString& value );
			void onGeneralOptionChanged( const QString& option, const QString& value );

			void setActiveSubtitleStream( int subtitleStream );

			void updateConfigFromDialog();

		private:

			void toggleFullScreen( bool on );

			AppConfig m_config;

			Subtitle* m_subtitle;
			KUrl m_subtitleUrl;
			QString m_subtitleFileName;
			QString m_subtitleEncoding;
			int m_subtitleEOL;
			QString m_subtitleFormat;

			bool m_translationMode;
			KUrl m_subtitleTrUrl;
			QString m_subtitleTrFileName;
			QString m_subtitleTrEncoding;
			int m_subtitleTrEOL;
			QString m_subtitleTrFormat;

			Player* m_player;

			SubtitleLine* m_lastFoundLine;

			//AudioLevels* m_audiolevels; // FIXME audio levels

			MainWindow* m_mainWindow;
			//AudioLevelsWidget* m_audiolevelsWidget; // FIXME audio levels
			PlayerWidget* m_playerWidget;
			LinesWidget* m_linesWidget;
			CurrentLineWidget* m_curLineWidget;

			ConfigDialog* m_configDialog;
			ErrorsDialog* m_errorsDialog;

			ErrorsWidget* m_errorsWidget;

			KUrl m_lastSubtitleUrl;
			KRecentFilesActionExt* m_recentSubtitlesAction;
			KRecentFilesActionExt* m_recentTrSubtitlesAction;

			KCodecAction* m_reloadSubtitleAsAction;
			KSelectAction* m_quickReloadSubtitleAsAction;

			Finder* m_finder;
			Replacer* m_replacer;
			ErrorFinder* m_errorFinder;
			Speller* m_speller;

			ErrorTracker* m_errorTracker;

			ScriptsManager* m_scriptsManager;

			KUrl m_lastVideoUrl;
			bool m_linkCurrentLineToPosition;
			KRecentFilesActionExt* m_recentVideosAction;

			KUrl m_lastAudioLevelsUrl;
			KRecentFilesActionExt* m_recentAudioLevelsAction;
	};

	Application* app();
}

#endif
