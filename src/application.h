/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef APPLICATION_H
#define APPLICATION_H

#include "mainwindow.h"
#include "core/subtitle.h"
#include "formats/format.h"
#include "gui/treeview/lineswidget.h"
#include "scconfig.h"

#include <QApplication>
#include <QAction>
#include <QExplicitlySharedDataPointer>
#include <QMap>
#include <QString>
#include <QKeySequence>
#include <QUrl>

#include <sonnet/configwidget.h>

#include <KActionCollection>

QT_FORWARD_DECLARE_CLASS(QAction)
QT_FORWARD_DECLARE_CLASS(QUndoStack)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QTextCodec)

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
class CurrentLineWidget;

class ConfigDialog;

class Finder;
class Replacer;
class ErrorFinder;
class Speller;
class ErrorTracker;

class ScriptsManager;

class UndoStack;

class Application : public QApplication
{
	Q_OBJECT

public:
	Application(int &argc, char **argv);
	virtual ~Application();

	inline MainWindow * mainWindow() const { return m_mainWindow; }
	inline LinesWidget * linesWidget() const { return m_mainWindow->m_linesWidget; }
	inline const SpeechProcessor * speechProcessor() const { return m_speechProcessor; }

	inline bool translationMode() const { return m_translationMode; }
	inline bool showingLinesContextMenu() const { return m_mainWindow->m_linesWidget->showingContextMenu(); }

	void loadConfig();
	void saveConfig();

	inline QAction * action(const char *actionName) const {
		return m_mainWindow->actionCollection()->action(actionName);
	}

	/**
	 * @brief triggerAction
	 * @param keySequence
	 * @return true if an action was triggered
	 */
	bool triggerAction(const QKeySequence &keySequence);

	inline const QUrl & lastSubtitleDirectory() const { return m_lastSubtitleUrl; }

	const QStringList & availableEncodingNames() const;
	static const QString & buildSubtitleFilesFilter(bool openFileFilter = true);
	static const QString & buildMediaFilesFilter();

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

	void detectErrors();
	void clearErrors();
	void selectNextError();
	void selectPreviousError();

	void retrocedeCurrentLine();
	void advanceCurrentLine();

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
	void seekToCurrentLine();
	void seekToNextLine();

	void playrateIncrease();
	void playrateDecrease();

	void setCurrentLineShowTimeFromVideo();
	void setCurrentLineHideTimeFromVideo();

	void anchorToggle();
	void anchorRemoveAll();

	void shiftToVideoPosition();

signals:
	void subtitleOpened(Subtitle *subtitle);
	void subtitleClosed();

	void translationModeChanged(bool value);
	void fullScreenModeChanged(bool value);

	void actionsReady();

private:
	void processSubtitleOpened(QTextCodec *codec, const QString &subtitleFormat);
	void processTranslationOpened(QTextCodec *codec, const QString &subtitleFormat);

	QTextCodec * codecForEncoding(const QString &encoding);

	bool acceptClashingUrls(const QUrl &subtitleUrl, const QUrl &subtitleTrUrl);

	QUrl saveSplitSubtitle(const Subtitle &subtitle, const QUrl &srcUrl, QString encoding, QString format, bool primary);

	void setupActions();

	Time videoPosition(bool compensate = false);

	void updateActionTexts();

private slots:
	void updateTitle();

	void onWaveformDoubleClicked(Time time);
	void onWaveformMiddleMouse(Time time);

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
	QUrl m_subtitleUrl;
	QString m_subtitleFileName;
	QString m_subtitleEncoding;
	QString m_subtitleFormat;

	bool m_translationMode;
	QUrl m_subtitleTrUrl;
	QString m_subtitleTrFileName;
	QString m_subtitleTrEncoding;
	QString m_subtitleTrFormat;

	TextDemux *m_textDemux;
	SpeechProcessor *m_speechProcessor;

	SubtitleLine *m_lastFoundLine;

	MainWindow *m_mainWindow;

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

	QLabel *m_labSubFormat = nullptr;
	QLabel *m_labSubEncoding = nullptr;

	ScriptsManager *m_scriptsManager;

	QUrl m_lastVideoUrl;
	bool m_linkCurrentLineToPosition;
	KRecentFilesAction *m_recentVideosAction;
};

}

#endif
