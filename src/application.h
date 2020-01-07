#ifndef APPLICATION_H
#define APPLICATION_H

/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2019 Mladen Milinkovic <max@smoothware.net>
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

#include "mainwindow.h"
#include "core/subtitle.h"
#include "formats/format.h"
#include "scconfig.h"

#include <QMap>
#include <QString>
#include <QKeySequence>

#include <QApplication>
#include <QAction>
#include <QUrl>
#include <sonnet/configwidget.h>

QT_FORWARD_DECLARE_CLASS(QAction)
QT_FORWARD_DECLARE_CLASS(QUndoStack)

class KComboBox;
class KToggleAction;
class KRecentFilesAction;
class KRecentFilesActionExt;
class KCodecActionExt;

namespace SubtitleComposer {
class VideoPlayer;
class TextDemux;
class SpeechProcessor;

class PlayerWidget;
class LinesWidget;
class CurrentLineWidget;
class StatusBar2;

class ConfigDialog;

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

	inline QUndoStack * undoStack() const { return m_undoStack; }

	MainWindow * mainWindow() const;
	LinesWidget * linesWidget() const;

	bool translationMode() const;
	bool showingLinesContextMenu() const;

	void loadConfig();
	void saveConfig();

	QAction * action(const char *actionName) const;

	/**
	 * @brief triggerAction
	 * @param keySequence
	 * @return true if an action was triggered
	 */
	bool triggerAction(const QKeySequence &keySequence);

	const QStringList & availableEncodingNames() const;

	const QUrl & lastSubtitleDirectory() const;

	inline const SpeechProcessor * speechProcessor() const { return m_speechProcessor; }

public slots:
	void newSubtitle();
	void openSubtitle();
	void reopenSubtitleWithCodec(QTextCodec *codec = nullptr);
	void demuxTextStream(int textStreamIndex);
	void openSubtitle(const QUrl &url, bool warnClashingUrls = true);
	bool saveSubtitle(QTextCodec *codec = nullptr);
	bool saveSubtitleAs(QTextCodec *codec = nullptr);
	bool closeSubtitle();

	void speechImportAudioStream(int audioStreamIndex);

	void newSubtitleTr();
	void openSubtitleTr();
	void reopenSubtitleTrWithCodec(QTextCodec *codec = nullptr);

	void openSubtitleTr(const QUrl &url, bool warnClashingUrls = true);
	bool saveSubtitleTr(QTextCodec *codec = nullptr);
	bool saveSubtitleTrAs(QTextCodec *codec = nullptr);
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

	void seekBackward();
	void seekForward();
	void stepBackward();
	void stepForward();
	void playOnlyCurrentLine();
	void seekToPrevLine();
	void seekToNextLine();

	void playrateIncrease();
	void playrateDecrease();

	void setCurrentLineShowTimeFromVideo();
	void setCurrentLineHideTimeFromVideo();

	void anchorToggle();
	void anchorRemoveAll();

	void shiftToVideoPosition();
	void adjustToVideoPositionAnchorLast();
	void adjustToVideoPositionAnchorFirst();

	static const QString & buildSubtitleFilesFilter(bool openFileFilter = true);
	static const QString & buildMediaFilesFilter();

signals:
	void subtitleOpened(Subtitle *subtitle);
	void subtitleClosed();

	void translationModeChanged(bool value);
	void fullScreenModeChanged(bool value);

private:
	void processSubtitleOpened(QTextCodec *codec, const QString &subtitleFormat);
	void processTranslationOpened(QTextCodec *codec, const QString &subtitleFormat);

	QTextCodec * codecForEncoding(const QString &encoding);

	bool acceptClashingUrls(const QUrl &subtitleUrl, const QUrl &subtitleTrUrl);

	QUrl saveSplitSubtitle(const Subtitle &subtitle, const QUrl &srcUrl, QString encoding, QString format, bool primary);

	void setupActions();

	Time videoPosition(bool compensate = false);

	bool applyTranslation(RangeList ranges, bool primary, int inputLanguage, int outputLanguage, int textTargets);

	void updateActionTexts();

private slots:
	void updateTitle();

	void onWaveformDoubleClicked(Time time);
	void onWaveformMiddleMouseDown(Time time);
	void onWaveformMiddleMouseUp(Time time);

	void onLineDoubleClicked(SubtitleLine *line);
	void onHighlightLine(SubtitleLine *line, bool primary = true, int firstIndex = -1, int lastIndex = -1);
	void onPlayingLineChanged(SubtitleLine *line);
	void onLinkCurrentLineToVideoToggled(bool value);

	void onPlayerFileOpened(const QString &filePath);
	void onPlayerPlaying();
	void onPlayerPaused();
	void onPlayerStopped();
	void onPlayerTextStreamsChanged(const QStringList &textStreams);
	void onPlayerAudioStreamsChanged(const QStringList &audioStreams);
	void onPlayerActiveAudioStreamChanged(int audioStream);
	void onPlayerMuteChanged(bool muted);

	void onConfigChanged();

	void setActiveSubtitleStream(int subtitleStream);

	void showPreferences();

private:
	Subtitle *m_subtitle;
	QUrl m_subtitleUrl;
	QString m_subtitleFileName;
	QString m_subtitleEncoding;
	QString m_subtitleFormat;

	bool m_translationMode;
	QUrl m_subtitleTrUrl;
	QString m_subtitleTrFileName;
	QString m_subtitleTrEncoding;
	QString m_subtitleTrFormat;

	VideoPlayer *m_player;

	TextDemux *m_textDemux;
	SpeechProcessor *m_speechProcessor;

	SubtitleLine *m_lastFoundLine;

	MainWindow *m_mainWindow;
	PlayerWidget *m_playerWidget;
	LinesWidget *m_linesWidget;
	CurrentLineWidget *m_curLineWidget;

	QUrl m_lastSubtitleUrl;
	KRecentFilesActionExt *m_recentSubtitlesAction;
	KRecentFilesActionExt *m_recentSubtitlesTrAction;

	KCodecActionExt *m_reopenSubtitleAsAction;
	KCodecActionExt *m_saveSubtitleAsAction;
	KCodecActionExt *m_reopenSubtitleTrAsAction;
	KCodecActionExt *m_saveSubtitleTrAsAction;

	Finder *m_finder;
	Replacer *m_replacer;
	ErrorFinder *m_errorFinder;
	Speller *m_speller;

	ErrorTracker *m_errorTracker;

	ScriptsManager *m_scriptsManager;

	QUrl m_lastVideoUrl;
	bool m_linkCurrentLineToPosition;
	KRecentFilesAction *m_recentVideosAction;

	QUndoStack *m_undoStack;
};

inline Application * app() { return static_cast<Application *>(QApplication::instance()); }


}

#endif
