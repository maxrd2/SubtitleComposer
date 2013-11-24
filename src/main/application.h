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
#include "../config/appconfig.h"
#include "../formats/format.h"

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtGui/QKeySequence>

#include <KApplication>
#include <KAction>
#include <KUrl>
#include <kencodingdetector.h>

class KComboBox;
class KAction;
class KToggleAction;
class KRecentFilesActionExt;
class KCodecActionExt;

namespace SubtitleComposer {
class Player;
class Decoder;

class PlayerWidget;
class LinesWidget;
class CurrentLineWidget;
class StatusBar2;

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

	static Application * instance();

	Subtitle * subtitle() const;

	MainWindow * mainWindow() const;
	LinesWidget * linesWidget() const;

	bool translationMode() const;
	bool showingLinesContextMenu() const;

	void loadConfig();
	void saveConfig();

	GeneralConfig * generalConfig() { return static_cast<GeneralConfig *>(m_config.group("General")); }
	SpellingConfig * spellingConfig() { return static_cast<SpellingConfig *>(m_config.group("Spelling")); }
	ErrorsConfig * errorsConfig() { return static_cast<ErrorsConfig *>(m_config.group("Errors")); }
	PlayerConfig * playerConfig() { return static_cast<PlayerConfig *>(m_config.group("Player")); }

	QAction * action(const char *actionName);

	/**
	 * @brief triggerAction
	 * @param keySequence
	 * @return true if an action was triggered
	 */
	bool triggerAction(const QKeySequence &keySequence);

	const QStringList & availableEncodingNames() const;

	const KUrl & lastSubtitleDirectory() const;

public slots:
	void undo();
	void redo();

	void newSubtitle();
	void openSubtitle();
	void reopenSubtitleWithCodec(QTextCodec *codec);
	void reopenSubtitleWithDetectScript(KEncodingDetector::AutoDetectScript autodetectScript);
	void reopenSubtitleWithCodecOrDetectScript(QTextCodec *codec, KEncodingDetector::AutoDetectScript autodetectScript);
	void openSubtitle(const KUrl &url, bool warnClashingUrls = true);
	bool saveSubtitle();
	bool saveSubtitleAs();
	bool closeSubtitle();

	void newSubtitleTr();
	void openSubtitleTr();
	void reopenSubtitleTrWithCodec(QTextCodec *codec);
	void reopenSubtitleTrWithDetectScript(KEncodingDetector::AutoDetectScript autodetectScript);
	void reopenSubtitleTrWithCodecOrDetectScript(QTextCodec *codec, KEncodingDetector::AutoDetectScript autodetectScript);

	void openSubtitleTr(const KUrl &url, bool warnClashingUrls = true);
	bool saveSubtitleTr();
	bool saveSubtitleTrAs();
	bool closeSubtitleTr();

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
	void clearSelectedErrors(bool includeMarks = false);
	void clearSelectedMarks();

	void showErrors();
	void showErrorsConfig();

	void toggleSelectedLinesMark();
	void toggleSelectedLinesBold();
	void toggleSelectedLinesItalic();
	void toggleSelectedLinesUnderline();
	void toggleSelectedLinesStrikeThrough();
	void changeSelectedLinesColor();

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
	void openVideo(const KUrl &url);

	void toggleFullScreenMode();
	void setFullScreenMode(bool enabled);

	void seekBackwards();
	void seekForwards();
	void seekToPrevLine();
	void seekToNextLine();

	void setCurrentLineShowTimeFromVideo();
	void setCurrentLineHideTimeFromVideo();

	void shiftToVideoPosition();
	void adjustToVideoPositionAnchorLast();
	void adjustToVideoPositionAnchorFirst();

	void extractVideoAudio();

	void openAudioLevels();
	void openAudioLevels(const KUrl &url);
	void saveAudioLevelsAs();
	void closeAudioLevels();

	void increaseAudioLevelsVZoom();
	void decreaseAudioLevelsVZoom();
	void increaseAudioLevelsHZoom();
	void decreaseAudioLevelsHZoom();

signals:
	void subtitleOpened(Subtitle *subtitle);
	void subtitleClosed();

	void translationModeChanged(bool value);
	void fullScreenModeChanged(bool value);

private:
	QTextCodec * codecForUrl(const KUrl &url, bool useRecentFiles, bool useDefault);
	QTextCodec * codecForEncoding(const QString &encoding, bool useDefault);

	bool acceptClashingUrls(const KUrl &subtitleUrl, const KUrl &subtitleTrUrl);

	KUrl saveSplitSubtitle(const Subtitle &subtitle, const KUrl &srcUrl, QString encoding, QString format, bool primary);

	void setupActions();

	Time videoPosition(bool compensate = false);

	static const QString & buildMediaFilesFilter();
	static const QString & buildLevelsFilesFilter();

	bool applyTranslation(RangeList ranges, bool primary, int inputLanguage, int outputLanguage, int textTargets);

private slots:
	void updateTitle();
	void updateUndoRedoToolTips();

	void onLineDoubleClicked(SubtitleLine *line);
	void onHighlightLine(SubtitleLine *line, bool primary = true, int firstIndex = -1, int lastIndex = -1);
	void onPlayingLineChanged(SubtitleLine *line);
	void onLinkCurrentLineToVideoToggled(bool value);

	void onPlayerFileOpened(const QString &filePath);
	void onPlayerPlaying();
	void onPlayerPaused();
	void onPlayerStopped();
	void onPlayerAudioStreamsChanged(const QStringList &audioStreams);
	void onPlayerActiveAudioStreamChanged(int audioStream);
	void onPlayerMuteChanged(bool muted);
	void onPlayerOptionChanged(const QString &option, const QString &value);

	void onDecodingError(const QString &errorMessage);

	void onGeneralOptionChanged(const QString &option, const QString &value);

	void setActiveSubtitleStream(int subtitleStream);

	void updateConfigFromDialog();

private:
	void toggleFullScreen(bool on);

	AppConfig m_config;

	Subtitle *m_subtitle;
	KUrl m_subtitleUrl;
	QString m_subtitleFileName;
	QString m_subtitleEncoding;
	Format::NewLine m_subtitleEOL;
	QString m_subtitleFormat;

	bool m_translationMode;
	KUrl m_subtitleTrUrl;
	QString m_subtitleTrFileName;
	QString m_subtitleTrEncoding;
	Format::NewLine m_subtitleTrEOL;
	QString m_subtitleTrFormat;

	Player *m_player;
	Decoder *m_decoder;

	SubtitleLine *m_lastFoundLine;

	MainWindow *m_mainWindow;
	PlayerWidget *m_playerWidget;
	LinesWidget *m_linesWidget;
	CurrentLineWidget *m_curLineWidget;
	StatusBar2 *m_statusBar;

	ConfigDialog *m_configDialog;
	ErrorsDialog *m_errorsDialog;

	ErrorsWidget *m_errorsWidget;

	KUrl m_lastSubtitleUrl;
	KRecentFilesActionExt *m_recentSubtitlesAction;
	KRecentFilesActionExt *m_recentSubtitlesTrAction;

	KCodecActionExt *m_reopenSubtitleAsAction;
	KCodecActionExt *m_reopenSubtitleTrAsAction;

	Finder *m_finder;
	Replacer *m_replacer;
	ErrorFinder *m_errorFinder;
	Speller *m_speller;

	ErrorTracker *m_errorTracker;

	ScriptsManager *m_scriptsManager;

	KUrl m_lastVideoUrl;
	bool m_linkCurrentLineToPosition;
	KRecentFilesActionExt *m_recentVideosAction;

	KUrl m_lastAudioLevelsUrl;
	KRecentFilesActionExt *m_recentAudioLevelsAction;
};

Application * app();
}

#endif
