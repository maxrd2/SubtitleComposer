#ifndef APPLICATION_H
#define APPLICATION_H

/**
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2015 Mladen Milinkovic <max@smoothware.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "mainwindow.h"
#include "../core/subtitle.h"
#include "../formats/format.h"
#include "scconfig.h"

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QKeySequence>

#include <QApplication>
#include <KConfigDialog>
#include <QAction>
#include <QUrl>
#include <kencodingdetector.h>

class KComboBox;
class QAction;
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

class Application : public QApplication
{
	Q_OBJECT

public:
	Application(int &argc, char **argv);
	virtual ~Application();

	void init();

	static Application * instance();

	Subtitle * subtitle() const;

	MainWindow * mainWindow() const;
	LinesWidget * linesWidget() const;

	bool translationMode() const;
	bool showingLinesContextMenu() const;

	void loadConfig();
	void saveConfig();

	QAction * action(const char *actionName);

	/**
	 * @brief triggerAction
	 * @param keySequence
	 * @return true if an action was triggered
	 */
	bool triggerAction(const QKeySequence &keySequence);

	const QStringList & availableEncodingNames() const;

	const QUrl & lastSubtitleDirectory() const;

public slots:
	void undo();
	void redo();

	void newSubtitle();
	void openSubtitle();
	void reopenSubtitleWithCodec(QTextCodec *codec);
	void reopenSubtitleWithDetectScript();
	void reopenSubtitleWithCodecOrDetectScript(QTextCodec *codec);
	void openSubtitle(const QUrl &url, bool warnClashingUrls = true);
	bool saveSubtitle();
	bool saveSubtitleAs();
	bool closeSubtitle();

	void newSubtitleTr();
	void openSubtitleTr();
	void reopenSubtitleTrWithCodec(QTextCodec *codec);
	void reopenSubtitleTrWithDetectScript();
	void reopenSubtitleTrWithCodecOrDetectScript(QTextCodec *codec);

	void openSubtitleTr(const QUrl &url, bool warnClashingUrls = true);
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

	void checqCriticals();
	void recheckAllErrors();
	void recheckSelectedErrors();
	void clearErrors();
	void clearSelectedErrors(bool includeMarks = false);
	void clearSelectedMarks();

	void showErrors();

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
	void openVideo(const QUrl &url);

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
	void openAudioLevels(const QUrl &url);
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
	QTextCodec * codecForUrl(const QUrl &url, bool useRecentFiles, bool useDefault);
	QTextCodec * codecForEncoding(const QString &encoding, bool useDefault);

	bool acceptClashingUrls(const QUrl &subtitleUrl, const QUrl &subtitleTrUrl);

	QUrl saveSplitSubtitle(const Subtitle &subtitle, const QUrl &srcUrl, QString encoding, QString format, bool primary);

	void setupActions();

	Time videoPosition(bool compensate = false);

	static const QString & buildMediaFilesFilter();
	static const QString & buildLevelsFilesFilter();

	bool applyTranslation(RangeList ranges, bool primary, int inputLanguage, int outputLanguage, int textTargets);

	void updateActionTexts();

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

	void onDecodingError(const QString &errorMessage);

	void onConfigChanged();

	void setActiveSubtitleStream(int subtitleStream);

	void showPreferences();

private:
	void toggleFullScreen(bool on);

	Subtitle *m_subtitle;
	QUrl m_subtitleUrl;
	QString m_subtitleFileName;
	QString m_subtitleEncoding;
	Format::NewLine m_subtitleEOL;
	QString m_subtitleFormat;

	bool m_translationMode;
	QUrl m_subtitleTrUrl;
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

	KConfigDialog *m_configDialog;

	QUrl m_lastSubtitleUrl;
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

	QUrl m_lastVideoUrl;
	bool m_linkCurrentLineToPosition;
	KRecentFilesActionExt *m_recentVideosAction;

	QUrl m_lastAudioLevelsUrl;
	KRecentFilesActionExt *m_recentAudioLevelsAction;
};

Application * app();
}

#endif
