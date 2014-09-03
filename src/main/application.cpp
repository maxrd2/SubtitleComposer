/***************************************************************************
 *   Copyright (C) 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Sn; either version 2 of the License, or     *
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

#include "application.h"
#include "mainwindow.h"
#include "audiolevelswidget.h"
#include "playerwidget.h"
#include "lineswidget.h"
#include "currentlinewidget.h"
#include "statusbar.h"
#include "configdialog.h"
#include "errorswidget.h"
#include "errorsdialog.h"
#include "actions/useraction.h"
#include "actions/useractionnames.h"
#include "actions/kcodecactionext.h"
#include "actions/krecentfilesactionext.h"
#include "configs/generalconfig.h"
#include "configs/spellingconfig.h"
#include "configs/playerconfig.h"
#include "configs/errorsconfig.h"
#include "dialogs/opensubtitledialog.h"
#include "dialogs/savesubtitledialog.h"
#include "dialogs/joinsubtitlesdialog.h"
#include "dialogs/splitsubtitledialog.h"
#include "dialogs/actionwithtargetdialog.h"
#include "dialogs/shifttimesdialog.h"
#include "dialogs/adjusttimesdialog.h"
#include "dialogs/durationlimitsdialog.h"
#include "dialogs/autodurationsdialog.h"
#include "dialogs/changetextscasedialog.h"
#include "dialogs/fixoverlappingtimesdialog.h"
#include "dialogs/fixpunctuationdialog.h"
#include "dialogs/translatedialog.h"
#include "dialogs/smarttextsadjustdialog.h"
#include "dialogs/changeframeratedialog.h"
#include "dialogs/syncsubtitlesdialog.h"
#include "dialogs/checkerrorsdialog.h"
#include "dialogs/clearerrorsdialog.h"
#include "dialogs/insertlinedialog.h"
#include "dialogs/removelinesdialog.h"
#include "dialogs/inputdialog.h"
#include "dialogs/subtitlecolordialog.h"
#include "utils/finder.h"
#include "utils/replacer.h"
#include "utils/errorfinder.h"
#include "utils/speller.h"
#include "utils/errortracker.h"
#include "utils/translator.h"
#include "scripting/scriptsmanager.h"
#include "../common/commondefs.h"
#include "../common/fileloadhelper.h"
#include "../common/filesavehelper.h"
#include "../common/qxtsignalwaiter.h"
#include "../core/subtitleiterator.h"
#include "../formats/formatmanager.h"
#include "../services/player.h"
#include "../services/playerbackend.h"
#include "../services/decoder.h"
#include "../services/decoderbackend.h"
#include "../profiler.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QTextCodec>
#include <QtCore/QProcess>
#include <QtGui/QGridLayout>
#include <QtGui/QMenu>

#include <KAboutData>
#include <KCmdLineArgs>
#include <KGlobal>
#include <KConfig>
#include <KCharsets>
#include <KStandardDirs>
#include <KLocale>
#include <KStandardShortcut>
#include <KAction>
#include <KStandardAction>
#include <KToggleAction>
#include <kcodecaction.h>
#include <KSelectAction>
#include <KActionCollection>
#include <KMessageBox>
#include <KComboBox>

using namespace SubtitleComposer;

Application *
SubtitleComposer::app()
{
	return Application::instance();
}

Application::Application() :
	KApplication(),
	m_config(),
	m_subtitle(0),
	m_subtitleUrl(),
	m_subtitleFileName(),
	m_subtitleEncoding(),
	m_subtitleEOL(Format::CurrentOS),
	m_subtitleFormat(),
	m_translationMode(false),
	m_subtitleTrUrl(),
	m_subtitleTrFileName(),
	m_subtitleTrEncoding(),
	m_subtitleTrEOL(Format::CurrentOS),
	m_subtitleTrFormat(),
	m_player(Player::instance()),
	m_decoder(Decoder::instance()),
	m_lastFoundLine(0),
//  m_audiolevels( 0 ), // FIXME audio levels
	m_lastSubtitleUrl(QDir::homePath()),
	m_lastVideoUrl(QDir::homePath()),
	m_linkCurrentLineToPosition(false)
{
	QStringList playerBackendNames(m_player->backendNames());
	QStringList decoderBackendNames(m_decoder->backendNames());

	// The config object has to be filled first with all the groups and default data...
	m_config.setGroup(new GeneralConfig());
	m_config.setGroup(new SpellingConfig());
	m_config.setGroup(new ErrorsConfig());
	m_config.setGroup(new PlayerConfig());
	for(QStringList::ConstIterator it = playerBackendNames.begin(), end = playerBackendNames.end(); it != end; ++it)
		m_config.setGroup(m_player->backend(*it)->config()->clone());

	// ...only then can be loaded
	m_config.readFrom(KGlobal::config().constData());

	// and feed to the player and decoder backends
	for(QStringList::ConstIterator it = playerBackendNames.begin(), end = playerBackendNames.end(); it != end; ++it)
		m_player->backend(*it)->setConfig(m_config.group(*it));
	for(QStringList::ConstIterator it = decoderBackendNames.begin(), end = decoderBackendNames.end(); it != end; ++it)
		m_decoder->backend(*it)->setConfig(m_config.group(*it));

	// NOTE the player is initialized by PlayerWidget because it requires the parent widget
	m_decoder->initialize(0, app()->playerConfig()->decoderBackend());

	m_mainWindow = new MainWindow();

	m_playerWidget = m_mainWindow->m_playerWidget;
	m_linesWidget = m_mainWindow->m_linesWidget;
	m_curLineWidget = m_mainWindow->m_curLineWidget;
	m_statusBar = m_mainWindow->m_statusBar;

	m_configDialog = new ConfigDialog(m_config, m_mainWindow);
	m_errorsDialog = new ErrorsDialog(m_mainWindow);

	m_errorsWidget = m_errorsDialog->errorsWidget();

	m_finder = new Finder(m_linesWidget);
	m_replacer = new Replacer(m_linesWidget);
	m_errorFinder = new ErrorFinder(m_linesWidget);
	m_speller = new Speller(m_linesWidget);

	m_errorTracker = new ErrorTracker(this);

	m_scriptsManager = new ScriptsManager(this);

	UserActionManager *actionManager = UserActionManager::instance();

	connect(playerConfig(), SIGNAL(optionChanged(const QString &, const QString &)), this, SLOT(onPlayerOptionChanged(const QString &, const QString &)));
	connect(generalConfig(), SIGNAL(optionChanged(const QString &, const QString &)), this, SLOT(onGeneralOptionChanged(const QString &, const QString &)));

	connect(m_player, SIGNAL(fileOpened(const QString &)), this, SLOT(onPlayerFileOpened(const QString &)));
	connect(m_player, SIGNAL(playing()), this, SLOT(onPlayerPlaying()));
	connect(m_player, SIGNAL(paused()), this, SLOT(onPlayerPaused()));
	connect(m_player, SIGNAL(stopped()), this, SLOT(onPlayerStopped()));
	connect(m_player, SIGNAL(audioStreamsChanged(const QStringList &)), this, SLOT(onPlayerAudioStreamsChanged(const QStringList &)));
	connect(m_player, SIGNAL(activeAudioStreamChanged(int)), this, SLOT(onPlayerActiveAudioStreamChanged(int)));
	connect(m_player, SIGNAL(muteChanged(bool)), this, SLOT(onPlayerMuteChanged(bool)));

	connect(m_decoder, SIGNAL(decoding()), m_statusBar, SLOT(initDecoding()));
	connect(m_decoder, SIGNAL(positionChanged(double)), m_statusBar, SLOT(setDecodingPosition(double)));
	connect(m_decoder, SIGNAL(lengthChanged(double)), m_statusBar, SLOT(setDecodingLength(double)));
	connect(m_decoder, SIGNAL(stopped()), m_statusBar, SLOT(endDecoding()));
	connect(m_decoder, SIGNAL(decodingError(const QString &)), this, SLOT(onDecodingError(const QString &)));

	QList<QObject *> listeners;
	listeners << actionManager << m_mainWindow << m_playerWidget << m_linesWidget << m_curLineWidget << m_statusBar << m_errorsWidget << m_finder << m_replacer << m_errorFinder << m_speller << m_errorTracker << m_scriptsManager;
	for(QList<QObject *>::ConstIterator it = listeners.begin(), end = listeners.end(); it != end; ++it) {
		connect(this, SIGNAL(subtitleOpened(Subtitle *)), *it, SLOT(setSubtitle(Subtitle *)));
		connect(this, SIGNAL(subtitleClosed()), *it, SLOT(setSubtitle()));
	}

	listeners.clear();
	listeners << actionManager << m_playerWidget << m_linesWidget << m_curLineWidget << m_finder << m_replacer << m_errorFinder << m_speller;
	for(QList<QObject *>::ConstIterator it = listeners.begin(), end = listeners.end(); it != end; ++it)
		connect(this, SIGNAL(translationModeChanged(bool)), *it, SLOT(setTranslationMode(bool)));

	connect(this, SIGNAL(fullScreenModeChanged(bool)), actionManager, SLOT(setFullScreenMode(bool)));

	connect(m_configDialog, SIGNAL(accepted()), this, SLOT(updateConfigFromDialog()));

	connect(m_linesWidget, SIGNAL(currentLineChanged(SubtitleLine *)), m_curLineWidget, SLOT(setCurrentLine(SubtitleLine *)));
	connect(m_linesWidget, SIGNAL(lineDoubleClicked(SubtitleLine *)), this, SLOT(onLineDoubleClicked(SubtitleLine *)));
	connect(m_linesWidget, SIGNAL(currentLineChanged(SubtitleLine *)), m_errorsWidget, SLOT(setCurrentLine(SubtitleLine *)));

	connect(m_errorsWidget, SIGNAL(lineDoubleClicked(SubtitleLine *)), m_linesWidget, SLOT(setCurrentLine(SubtitleLine *)));

	connect(m_playerWidget, SIGNAL(playingLineChanged(SubtitleLine *)), this, SLOT(onPlayingLineChanged(SubtitleLine *)));

	connect(m_finder, SIGNAL(found(SubtitleLine *, bool, int, int)), this, SLOT(onHighlightLine(SubtitleLine *, bool, int, int)));
	connect(m_replacer, SIGNAL(found(SubtitleLine *, bool, int, int)), this, SLOT(onHighlightLine(SubtitleLine *, bool, int, int)));
	connect(m_errorFinder, SIGNAL(found(SubtitleLine *)), this, SLOT(onHighlightLine(SubtitleLine *)));
	connect(m_speller, SIGNAL(misspelled(SubtitleLine *, bool, int, int)), this, SLOT(onHighlightLine(SubtitleLine *, bool, int, int)));

	actionManager->setLinesWidget(m_linesWidget);
	actionManager->setPlayer(m_player);
	actionManager->setDecoder(m_decoder);
	actionManager->setFullScreenMode(false);

	setupActions();

	m_playerWidget->plugActions();
	m_statusBar->plugActions();
	m_curLineWidget->setupActions();

	m_mainWindow->setupGUI();

	m_scriptsManager->reloadScripts();

	loadConfig();
}

Application::~Application()
{
	// NOTE: The Application destructor is called after all widgets are destroyed
	// (NOT BEFORE). Therefore is not possible to save the program settings (nor do
	// pretty much anything) at this point.

	// delete m_mainWindow; the windows is destroyed when it's closed

	delete m_subtitle;
//  delete m_audiolevels; // FIXME audio levels
}

Application *
Application::instance()
{
	return static_cast<Application *>(kapp);
}

Subtitle *
Application::subtitle() const
{
	return m_subtitle;
}

MainWindow *
Application::mainWindow() const
{
	return m_mainWindow;
}

LinesWidget *
Application::linesWidget() const
{
	return m_linesWidget;
}

bool
Application::translationMode() const
{
	return m_translationMode;
}

bool
Application::showingLinesContextMenu() const
{
	return m_linesWidget->showingContextMenu();
}

void
Application::loadConfig()
{
	KConfigGroup group(KGlobal::config()->group("Application Settings"));

	m_lastSubtitleUrl = KUrl(group.readPathEntry("LastSubtitleUrl", QDir::homePath()));
	m_recentSubtitlesAction->loadEntries(KGlobal::config()->group("Recent Subtitles"));
	m_recentSubtitlesTrAction->loadEntries(KGlobal::config()->group("Recent Translation Subtitles"));

	m_lastAudioLevelsUrl = KUrl(group.readPathEntry("LastAudioLevelsUrl", QDir::homePath()));
//  m_recentAudioLevelsAction->loadEntries( group, "Recent Audio Levels" ); // FIXME audio levels

	m_lastVideoUrl = KUrl(group.readPathEntry("LastVideoUrl", QDir::homePath()));
	m_recentVideosAction->loadEntries(KGlobal::config()->group("Recent Videos"));

	m_player->setMuted(group.readEntry<bool>("Muted", false));
	m_player->setVolume(group.readEntry<double>("Volume", 100.0));

	((KToggleAction *)action(ACT_TOGGLE_MUTED))->setChecked(m_player->isMuted());

	m_mainWindow->loadConfig();
//  m_audiolevelsWidget->loadConfig(); // FIXME audio levels
	m_playerWidget->loadConfig();
	m_linesWidget->loadConfig();
	m_curLineWidget->loadConfig();
	m_errorsDialog->loadConfig();
}

void
Application::saveConfig()
{
	m_config.writeTo(KGlobal::config().data());

	KConfigGroup group(KGlobal::config()->group("Application Settings"));

	group.writePathEntry("LastSubtitleUrl", m_lastSubtitleUrl.prettyUrl());
	m_recentSubtitlesAction->saveEntries(KGlobal::config()->group("Recent Subtitles"));
	m_recentSubtitlesTrAction->saveEntries(KGlobal::config()->group("Recent Translation Subtitles"));

	group.writePathEntry("LastAudioLevelsUrl", m_lastAudioLevelsUrl.prettyUrl());
//  m_recentAudioLevelsAction->saveEntries( KGlobal::config()->group( "Recent Audio Levels" ) ); // FIXME audio levels

	group.writePathEntry("LastVideoUrl", m_lastVideoUrl.prettyUrl());
	m_recentVideosAction->saveEntries(KGlobal::config()->group("Recent Videos"));

	group.writeEntry("Muted", m_player->isMuted());
	group.writeEntry("Volume", m_player->volume());

	m_mainWindow->saveConfig();
//  m_audiolevelsWidget->saveConfig(); // FIXME audio levels
	m_playerWidget->saveConfig();
	m_linesWidget->saveConfig();
	m_curLineWidget->saveConfig();
	m_errorsDialog->saveConfig();
}

QAction *
Application::action(const char *actionName)
{
	return m_mainWindow->actionCollection()->action(actionName);
}

bool
Application::triggerAction(const QKeySequence &keySequence)
{
	QList<QAction *> actions = m_mainWindow->actionCollection()->actions();

	for(QList<QAction *>::ConstIterator it = actions.begin(), end = actions.end(); it != end; ++it) {
		if((*it)->isEnabled()) {
			if(KAction * action = qobject_cast<KAction *>(*it)) {
				KShortcut shortcut = action->shortcut();
				if(shortcut.primary() == keySequence || shortcut.alternate() == keySequence) {
					action->trigger();
					return true;
				}
			} else {
				if((*it)->shortcut() == keySequence) {
					(*it)->trigger();
					return true;
				}
			}
		}
	}

	return false;
}

const QStringList &
Application::availableEncodingNames() const
{
	static QStringList encodingNames;

	if(encodingNames.empty()) {
		QStringList encodings(KGlobal::charsets()->availableEncodingNames());
		for(QStringList::Iterator it = encodings.begin(); it != encodings.end(); ++it) {
			bool found = false;
			QTextCodec *codec = KGlobal::charsets()->codecForName(*it, found);
			if(found)
				encodingNames.append(codec->name().toUpper());
		}
		encodingNames.sort();
	}

	return encodingNames;
}

const KUrl &
Application::lastSubtitleDirectory() const
{
	return m_lastSubtitleUrl;
}

const QString &
Application::buildMediaFilesFilter()
{
	static QString filter;

	if(filter.isEmpty()) {
		QString mediaExtensions;

		QString videoExtensions;
		QStringList videoExts(QString("avi flv mkv mov mpg mpeg mp4 wmv ogm ogv rmvb ts vob").split(' '));
		for(QStringList::ConstIterator it = videoExts.begin(), end = videoExts.end(); it != end; ++it)
//          videoExtensions += " *." + *it /*+ " *." + (*it).toUpper()*/;
			videoExtensions += " *." + *it + " *." + (*it).toUpper();
		mediaExtensions += videoExtensions;
		filter += '\n' + videoExtensions.trimmed() + '|' + i18n("Video Files");

		QString audioExtensions;
		QStringList audioExts(QString("aac ac3 ape flac la m4a mac mp2 mp3 mp4 mp+ mpc mpp ofr oga ogg pac ra spx tta wav wma wv").split(' '));
		for(QStringList::ConstIterator it = audioExts.begin(), end = audioExts.end(); it != end; ++it)
//          audioExtensions += " *." + *it /*+ " *." + (*it).toUpper()*/;
			audioExtensions += " *." + *it + " *." + (*it).toUpper();
		mediaExtensions += audioExtensions;
		filter += '\n' + audioExtensions.trimmed() + '|' + i18n("Audio Files");

		filter = mediaExtensions + '|' + i18n("Media Files") + filter;
		filter += "\n*|" + i18n("All Files");
	}

	return filter;
}

const QString &
Application::buildLevelsFilesFilter()
{
	static QString filter;

	if(filter.isEmpty()) {
		QString levelsExtensions;
		QStringList videoExts(QString("wf").split(' '));
		for(QStringList::ConstIterator it = videoExts.begin(), end = videoExts.end(); it != end; ++it)
			levelsExtensions += " *." + *it + " *." + (*it).toUpper();
		filter += '\n' + levelsExtensions.trimmed() + '|' + i18n("Audio Levels Files");

		filter += buildMediaFilesFilter();
	}

	return filter;
}

void
Application::setupActions()
{
	KActionCollection *actionCollection = m_mainWindow->actionCollection();
	UserActionManager *actionManager = UserActionManager::instance();

	KAction *quitAction = KStandardAction::quit(m_mainWindow, SLOT(close()), actionCollection);
	quitAction->setStatusTip(i18n("Exit the application"));

	KAction *prefsAction = KStandardAction::preferences(m_configDialog, SLOT(show()), actionCollection);
	prefsAction->setStatusTip(i18n("Configure Subtitle Composer"));

	KAction *newSubtitleAction = new KAction(actionCollection);
	newSubtitleAction->setIcon(KIcon("document-new"));
	newSubtitleAction->setText(i18nc("@action:inmenu Create a new subtitle", "New"));
	newSubtitleAction->setStatusTip(i18n("Create an empty subtitle"));
	newSubtitleAction->setShortcut(KStandardShortcut::openNew(), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(newSubtitleAction, SIGNAL(triggered()), this, SLOT(newSubtitle()));
	actionCollection->addAction(ACT_NEW_SUBTITLE, newSubtitleAction);
	actionManager->addAction(newSubtitleAction, UserAction::FullScreenOff);

	KAction *openSubtitleAction = new KAction(actionCollection);
	openSubtitleAction->setIcon(KIcon("document-open"));
	openSubtitleAction->setText(i18nc("@action:inmenu Open subtitle file", "Open..."));
	openSubtitleAction->setStatusTip(i18n("Open subtitle file"));
	openSubtitleAction->setShortcut(KStandardShortcut::open(), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(openSubtitleAction, SIGNAL(triggered()), this, SLOT(openSubtitle()));
	actionCollection->addAction(ACT_OPEN_SUBTITLE, openSubtitleAction);
	actionManager->addAction(openSubtitleAction, 0);

	m_reopenSubtitleAsAction = new KCodecActionExt(actionCollection, true);
	m_reopenSubtitleAsAction->setIcon(KIcon("view-refresh"));
	m_reopenSubtitleAsAction->setText(i18n("Reload As..."));
	m_reopenSubtitleAsAction->setStatusTip(i18n("Reload opened file with a different encoding"));
	connect(m_reopenSubtitleAsAction, SIGNAL(triggered(QTextCodec *)), this, SLOT(reopenSubtitleWithCodec(QTextCodec *)));
	connect(m_reopenSubtitleAsAction, SIGNAL(triggered(KEncodingDetector::AutoDetectScript)), this, SLOT(reopenSubtitleWithDetectScript(KEncodingDetector::AutoDetectScript)));
	actionCollection->addAction(ACT_REOPEN_SUBTITLE_AS, m_reopenSubtitleAsAction);
	actionManager->addAction(m_reopenSubtitleAsAction, UserAction::SubOpened | UserAction::SubPClean | UserAction::FullScreenOff);

	m_recentSubtitlesAction = new KRecentFilesActionExt(actionCollection);
	m_recentSubtitlesAction->setIcon(KIcon("document-open"));
	m_recentSubtitlesAction->setText(i18nc("@action:inmenu Open rencently used subtitle file", "Open &Recent"));
	m_recentSubtitlesAction->setStatusTip(i18n("Open subtitle file"));
	connect(m_recentSubtitlesAction, SIGNAL(urlSelected(const KUrl &)), this, SLOT(openSubtitle(const KUrl &)));
	actionCollection->addAction(ACT_RECENT_SUBTITLES, m_recentSubtitlesAction);

	KAction *saveSubtitleAction = new KAction(actionCollection);
	saveSubtitleAction->setIcon(KIcon("document-save"));
	saveSubtitleAction->setText(i18n("Save"));
	saveSubtitleAction->setStatusTip(i18n("Save opened subtitle"));
	saveSubtitleAction->setShortcut(KStandardShortcut::save(), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(saveSubtitleAction, SIGNAL(triggered()), this, SLOT(saveSubtitle()));
	actionCollection->addAction(ACT_SAVE_SUBTITLE, saveSubtitleAction);
	actionManager->addAction(saveSubtitleAction, UserAction::SubPDirty | UserAction::FullScreenOff);

	KAction *saveSubtitleAsAction = new KAction(actionCollection);
	saveSubtitleAsAction->setIcon(KIcon("document-save-as"));
	saveSubtitleAsAction->setText(i18n("Save As..."));
	saveSubtitleAsAction->setStatusTip(i18n("Save opened subtitle with different settings"));
	connect(saveSubtitleAsAction, SIGNAL(triggered()), this, SLOT(saveSubtitleAs()));
	actionCollection->addAction(ACT_SAVE_SUBTITLE_AS, saveSubtitleAsAction);
	actionManager->addAction(saveSubtitleAsAction, UserAction::SubOpened | UserAction::FullScreenOff);

	KAction *closeSubtitleAction = new KAction(actionCollection);
	closeSubtitleAction->setIcon(KIcon("window-close"));
	closeSubtitleAction->setText(i18n("Close"));
	closeSubtitleAction->setStatusTip(i18n("Close opened subtitle"));
	closeSubtitleAction->setShortcut(KStandardShortcut::close(), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(closeSubtitleAction, SIGNAL(triggered()), this, SLOT(closeSubtitle()));
	actionCollection->addAction(ACT_CLOSE_SUBTITLE, closeSubtitleAction);
	actionManager->addAction(closeSubtitleAction, UserAction::SubOpened | UserAction::FullScreenOff);

	KAction *newSubtitleTrAction = new KAction(actionCollection);
	newSubtitleTrAction->setIcon(KIcon("document-new"));
	newSubtitleTrAction->setText(i18n("New Translation"));
	newSubtitleTrAction->setStatusTip(i18n("Create an empty translation subtitle"));
	newSubtitleTrAction->setShortcut(KShortcut("Ctrl+Shift+N"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(newSubtitleTrAction, SIGNAL(triggered()), this, SLOT(newSubtitleTr()));
	actionCollection->addAction(ACT_NEW_SUBTITLE_TR, newSubtitleTrAction);
	actionManager->addAction(newSubtitleTrAction, UserAction::SubOpened | UserAction::FullScreenOff);

	KAction *openSubtitleTrAction = new KAction(actionCollection);
	openSubtitleTrAction->setIcon(KIcon("document-open"));
	openSubtitleTrAction->setText(i18n("Open Translation..."));
	openSubtitleTrAction->setStatusTip(i18n("Open translation subtitle file"));
	openSubtitleTrAction->setShortcut(KShortcut("Ctrl+Shift+O"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(openSubtitleTrAction, SIGNAL(triggered()), this, SLOT(openSubtitleTr()));
	actionCollection->addAction(ACT_OPEN_SUBTITLE_TR, openSubtitleTrAction);
	actionManager->addAction(openSubtitleTrAction, UserAction::SubOpened);

	m_reopenSubtitleTrAsAction = new KCodecActionExt(actionCollection, true);
	m_reopenSubtitleTrAsAction->setIcon(KIcon("view-refresh"));
	m_reopenSubtitleTrAsAction->setText(i18n("Reload Translation As..."));
	m_reopenSubtitleTrAsAction->setStatusTip(i18n("Reload opened translation file with a different encoding"));
	connect(m_reopenSubtitleTrAsAction, SIGNAL(triggered(QTextCodec *)), this, SLOT(reopenSubtitleTrWithCodec(QTextCodec *)));
	connect(m_reopenSubtitleTrAsAction, SIGNAL(triggered(KEncodingDetector::AutoDetectScript)), this, SLOT(reopenSubtitleTrWithDetectScript(KEncodingDetector::AutoDetectScript)));
	actionCollection->addAction(ACT_REOPEN_SUBTITLE_TR_AS, m_reopenSubtitleTrAsAction);
	actionManager->addAction(m_reopenSubtitleTrAsAction, UserAction::SubTrOpened | UserAction::SubSClean | UserAction::FullScreenOff);

	m_recentSubtitlesTrAction = new KRecentFilesActionExt(actionCollection);
	m_recentSubtitlesTrAction->setIcon(KIcon("document-open"));
	m_recentSubtitlesTrAction->setText(i18n("Open &Recent Translation"));
	m_recentSubtitlesTrAction->setStatusTip(i18n("Open translation subtitle file"));
	connect(m_recentSubtitlesTrAction, SIGNAL(urlSelected(const KUrl &)), this, SLOT(openSubtitleTr(const KUrl &)));
	actionCollection->addAction(ACT_RECENT_SUBTITLES_TR, m_recentSubtitlesTrAction);
	actionManager->addAction(m_recentSubtitlesTrAction, UserAction::SubOpened | UserAction::FullScreenOff);

	KAction *saveSubtitleTrAction = new KAction(actionCollection);
	saveSubtitleTrAction->setIcon(KIcon("document-save"));
	saveSubtitleTrAction->setText(i18n("Save Translation"));
	saveSubtitleTrAction->setStatusTip(i18n("Save opened translation subtitle"));
	saveSubtitleTrAction->setShortcut(KShortcut("Ctrl+Shift+S"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(saveSubtitleTrAction, SIGNAL(triggered()), this, SLOT(saveSubtitleTr()));
	actionCollection->addAction(ACT_SAVE_SUBTITLE_TR, saveSubtitleTrAction);
	actionManager->addAction(saveSubtitleTrAction, UserAction::SubSDirty | UserAction::FullScreenOff);

	KAction *saveSubtitleTrAsAction = new KAction(actionCollection);
	saveSubtitleTrAsAction->setIcon(KIcon("document-save-as"));
	saveSubtitleTrAsAction->setText(i18n("Save Translation As..."));
	saveSubtitleTrAsAction->setStatusTip(i18n("Save opened translation subtitle with different settings"));
	connect(saveSubtitleTrAsAction, SIGNAL(triggered()), this, SLOT(saveSubtitleTrAs()));
	actionCollection->addAction(ACT_SAVE_SUBTITLE_TR_AS, saveSubtitleTrAsAction);
	actionManager->addAction(saveSubtitleTrAsAction, UserAction::SubTrOpened | UserAction::FullScreenOff);

	KAction *closeSubtitleTrAction = new KAction(actionCollection);
	closeSubtitleTrAction->setIcon(KIcon("window-close"));
	closeSubtitleTrAction->setText(i18n("Close Translation"));
	closeSubtitleTrAction->setStatusTip(i18n("Close opened translation subtitle"));
	closeSubtitleTrAction->setShortcut(KShortcut("Ctrl+Shift+F4; Ctrl+Shift+W"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(closeSubtitleTrAction, SIGNAL(triggered()), this, SLOT(closeSubtitleTr()));
	actionCollection->addAction(ACT_CLOSE_SUBTITLE_TR, closeSubtitleTrAction);
	actionManager->addAction(closeSubtitleTrAction, UserAction::SubTrOpened | UserAction::FullScreenOff);

	KAction *undoAction = new KAction(actionCollection);
	undoAction->setIcon(KIcon("edit-undo"));
	undoAction->setText(i18n("Undo"));
	undoAction->setStatusTip(i18n("Undo"));
	undoAction->setShortcut(KStandardShortcut::undo(), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(undoAction, SIGNAL(triggered()), this, SLOT(undo()));
	actionCollection->addAction(ACT_UNDO, undoAction);
	actionManager->addAction(undoAction, UserAction::SubHasUndo);

	KAction *redoAction = new KAction(actionCollection);
	redoAction->setIcon(KIcon("edit-redo"));
	redoAction->setText(i18n("Redo"));
	redoAction->setStatusTip(i18n("Redo"));
	redoAction->setShortcut(KStandardShortcut::redo(), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(redoAction, SIGNAL(triggered()), this, SLOT(redo()));
	actionCollection->addAction(ACT_REDO, redoAction);
	actionManager->addAction(redoAction, UserAction::SubHasRedo);

	KAction *splitSubtitleAction = new KAction(actionCollection);
	splitSubtitleAction->setText(i18n("Split Subtitle..."));
	splitSubtitleAction->setStatusTip(i18n("Split the opened subtitle in two parts"));
	connect(splitSubtitleAction, SIGNAL(triggered()), this, SLOT(splitSubtitle()));
	actionCollection->addAction(ACT_SPLIT_SUBTITLE, splitSubtitleAction);
	actionManager->addAction(splitSubtitleAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	KAction *joinSubtitlesAction = new KAction(actionCollection);
	joinSubtitlesAction->setText(i18n("Join Subtitles..."));
	joinSubtitlesAction->setStatusTip(i18n("Append to the opened subtitle another one"));
	connect(joinSubtitlesAction, SIGNAL(triggered()), this, SLOT(joinSubtitles()));
	actionCollection->addAction(ACT_JOIN_SUBTITLES, joinSubtitlesAction);
	actionManager->addAction(joinSubtitlesAction, UserAction::SubOpened | UserAction::FullScreenOff);

	KAction *insertBeforeCurrentLineAction = new KAction(actionCollection);
	insertBeforeCurrentLineAction->setText(i18n("Insert Before"));
	insertBeforeCurrentLineAction->setStatusTip(i18n("Insert empty line before current one"));
	insertBeforeCurrentLineAction->setShortcut(KShortcut("Ctrl+Insert"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(insertBeforeCurrentLineAction, SIGNAL(triggered()), this, SLOT(insertBeforeCurrentLine()));
	actionCollection->addAction(ACT_INSERT_BEFORE_CURRENT_LINE, insertBeforeCurrentLineAction);
	actionManager->addAction(insertBeforeCurrentLineAction, UserAction::SubOpened | UserAction::FullScreenOff);

	KAction *insertAfterCurrentLineAction = new KAction(actionCollection);
	insertAfterCurrentLineAction->setText(i18n("Insert After"));
	insertAfterCurrentLineAction->setStatusTip(i18n("Insert empty line after current one"));
	insertAfterCurrentLineAction->setShortcut(KShortcut("Insert"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(insertAfterCurrentLineAction, SIGNAL(triggered()), this, SLOT(insertAfterCurrentLine()));
	actionCollection->addAction(ACT_INSERT_AFTER_CURRENT_LINE, insertAfterCurrentLineAction);
	actionManager->addAction(insertAfterCurrentLineAction, UserAction::SubOpened | UserAction::FullScreenOff);

	KAction *removeSelectedLinesAction = new KAction(actionCollection);
	removeSelectedLinesAction->setText(i18n("Remove"));
	removeSelectedLinesAction->setStatusTip(i18n("Remove selected lines"));
	removeSelectedLinesAction->setShortcut(KShortcut("Delete"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(removeSelectedLinesAction, SIGNAL(triggered()), this, SLOT(removeSelectedLines()));
	actionCollection->addAction(ACT_REMOVE_SELECTED_LINES, removeSelectedLinesAction);
	actionManager->addAction(removeSelectedLinesAction, UserAction::HasSelection | UserAction::FullScreenOff);

	KAction *splitSelectedLinesAction = new KAction(actionCollection);
	splitSelectedLinesAction->setText(i18n("Split Lines"));
	splitSelectedLinesAction->setStatusTip(i18n("Split selected lines"));
	connect(splitSelectedLinesAction, SIGNAL(triggered()), this, SLOT(splitSelectedLines()));
	actionCollection->addAction(ACT_SPLIT_SELECTED_LINES, splitSelectedLinesAction);
	actionManager->addAction(splitSelectedLinesAction, UserAction::HasSelection | UserAction::FullScreenOff);

	KAction *joinSelectedLinesAction = new KAction(actionCollection);
	joinSelectedLinesAction->setText(i18n("Join Lines"));
	joinSelectedLinesAction->setStatusTip(i18n("Join selected lines"));
	connect(joinSelectedLinesAction, SIGNAL(triggered()), this, SLOT(joinSelectedLines()));
	actionCollection->addAction(ACT_JOIN_SELECTED_LINES, joinSelectedLinesAction);
	actionManager->addAction(joinSelectedLinesAction, UserAction::HasSelection | UserAction::FullScreenOff);

	KAction *selectAllLinesAction = new KAction(actionCollection);
	selectAllLinesAction->setText(i18n("Select All"));
	selectAllLinesAction->setStatusTip(i18n("Select all lines"));
	selectAllLinesAction->setShortcut(KStandardShortcut::selectAll(), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(selectAllLinesAction, SIGNAL(triggered()), this, SLOT(selectAllLines()));
	actionCollection->addAction(ACT_SELECT_ALL_LINES, selectAllLinesAction);
	actionManager->addAction(selectAllLinesAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	KAction *gotoLineAction = new KAction(actionCollection);
	gotoLineAction->setText(i18n("Go to Line..."));
	gotoLineAction->setStatusTip(i18n("Go to specified line"));
	gotoLineAction->setShortcut(KStandardShortcut::gotoLine(), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(gotoLineAction, SIGNAL(triggered()), this, SLOT(gotoLine()));
	actionCollection->addAction(ACT_GOTO_LINE, gotoLineAction);
	actionManager->addAction(gotoLineAction, UserAction::SubHasLines);

	KAction *findAction = new KAction(actionCollection);
	findAction->setIcon(KIcon("edit-find"));
	findAction->setText(i18n("Find..."));
	findAction->setStatusTip(i18n("Find occurrences of strings or regular expressions"));
	findAction->setShortcut(KStandardShortcut::find(), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(findAction, SIGNAL(triggered()), this, SLOT(find()));
	actionCollection->addAction(ACT_FIND, findAction);
	actionManager->addAction(findAction, UserAction::SubHasLine);

	KAction *findNextAction = new KAction(actionCollection);
//  findNextAction->setIcon( KIcon( "go-down" ) );
	findNextAction->setIcon(KIcon("go-down-search"));
	findNextAction->setText(i18n("Find Next"));
	findNextAction->setStatusTip(i18n("Find next occurrence of string or regular expression"));
	findNextAction->setShortcut(KStandardShortcut::findNext(), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(findNextAction, SIGNAL(triggered()), this, SLOT(findNext()));
	actionCollection->addAction(ACT_FIND_NEXT, findNextAction);
	actionManager->addAction(findNextAction, UserAction::SubHasLine);

	KAction *findPreviousAction = new KAction(actionCollection);
//  findPreviousAction->setIcon( KIcon( "go-up" ) );
	findPreviousAction->setIcon(KIcon("go-up-search"));
	findPreviousAction->setText(i18n("Find Previous"));
	findPreviousAction->setStatusTip(i18n("Find previous occurrence of string or regular expression"));
	findPreviousAction->setShortcut(KStandardShortcut::findPrev(), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(findPreviousAction, SIGNAL(triggered()), this, SLOT(findPrevious()));
	actionCollection->addAction(ACT_FIND_PREVIOUS, findPreviousAction);
	actionManager->addAction(findPreviousAction, UserAction::SubHasLine);

	KAction *replaceAction = new KAction(actionCollection);
	replaceAction->setText(i18n("Replace..."));
	replaceAction->setStatusTip(i18n("Replace occurrences of strings or regular expressions"));
	replaceAction->setShortcut(KStandardShortcut::replace(), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(replaceAction, SIGNAL(triggered()), this, SLOT(replace()));
	actionCollection->addAction(ACT_REPLACE, replaceAction);
	actionManager->addAction(replaceAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	KAction *retrocedeCurrentLineAction = new KAction(actionCollection);
	retrocedeCurrentLineAction->setIcon(KIcon("go-down"));
	retrocedeCurrentLineAction->setText(i18n("Retrocede current line"));
	retrocedeCurrentLineAction->setStatusTip(i18n("Makes the line before the current one active"));
	retrocedeCurrentLineAction->setShortcut(KShortcut("Alt+Up"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(retrocedeCurrentLineAction, SIGNAL(triggered()), this, SLOT(retrocedeCurrentLine()));
	actionCollection->addAction(ACT_RETROCEDE_CURRENT_LINE, retrocedeCurrentLineAction);
	actionManager->addAction(retrocedeCurrentLineAction, UserAction::SubHasLines | UserAction::HasSelection);

	KAction *advanceCurrentLineAction = new KAction(actionCollection);
	advanceCurrentLineAction->setIcon(KIcon("go-down"));
	advanceCurrentLineAction->setText(i18n("Advance current line"));
	advanceCurrentLineAction->setStatusTip(i18n("Makes the line after the current one active"));
	advanceCurrentLineAction->setShortcut(KShortcut("Alt+Down"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(advanceCurrentLineAction, SIGNAL(triggered()), this, SLOT(advanceCurrentLine()));
	actionCollection->addAction(ACT_ADVANCE_CURRENT_LINE, advanceCurrentLineAction);
	actionManager->addAction(advanceCurrentLineAction, UserAction::SubHasLines | UserAction::HasSelection);

	KAction *checkErrorsAction = new KAction(actionCollection);
	checkErrorsAction->setText(i18n("Check Errors..."));
	checkErrorsAction->setStatusTip(i18n("Check for errors in the current subtitle"));
	checkErrorsAction->setShortcut(KShortcut("Ctrl+E"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(checkErrorsAction, SIGNAL(triggered()), this, SLOT(checkErrors()));
	actionCollection->addAction(ACT_CHECK_ERRORS, checkErrorsAction);
	actionManager->addAction(checkErrorsAction, UserAction::HasSelection | UserAction::FullScreenOff);

	KAction *clearErrorsAction = new KAction(actionCollection);
	clearErrorsAction->setText(i18n("Clear Errors..."));
	clearErrorsAction->setStatusTip(i18n("Clear errors from selected lines"));
	connect(clearErrorsAction, SIGNAL(triggered()), this, SLOT(clearErrors()));
	actionCollection->addAction(ACT_CLEAR_ERRORS, clearErrorsAction);
	actionManager->addAction(clearErrorsAction, UserAction::HasSelection | UserAction::FullScreenOff);

	KAction *showErrorsAction = new KAction(actionCollection);
	showErrorsAction->setText(i18n("Show Errors..."));
	showErrorsAction->setStatusTip(i18n("Show errors information for the current subtitle"));
	showErrorsAction->setShortcut(KShortcut("Shift+E"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(showErrorsAction, SIGNAL(triggered()), this, SLOT(showErrors()));
	actionCollection->addAction(ACT_SHOW_ERRORS, showErrorsAction);
	actionManager->addAction(showErrorsAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	KAction *findErrorAction = new KAction(actionCollection);
	findErrorAction->setIcon(KIcon("edit-find"));
	findErrorAction->setText(i18n("Find Error..."));
	findErrorAction->setStatusTip(i18n("Find lines with specified errors"));
	findErrorAction->setShortcut(KShortcut("Ctrl+Shift+E"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(findErrorAction, SIGNAL(triggered()), this, SLOT(findError()));
	actionCollection->addAction(ACT_FIND_ERROR, findErrorAction);
	actionManager->addAction(findErrorAction, UserAction::SubHasLine);

	KAction *findNextErrorAction = new KAction(actionCollection);
//  findNextErrorAction->setIcon( KIcon( "go-down" ) );
	findNextErrorAction->setIcon(KIcon("go-down-search"));
	findNextErrorAction->setText(i18n("Find Next Error"));
	findNextErrorAction->setStatusTip(i18n("Find next line with specified errors"));
	findNextErrorAction->setShortcut(KShortcut("F4"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(findNextErrorAction, SIGNAL(triggered()), this, SLOT(findNextError()));
	actionCollection->addAction(ACT_FIND_NEXT_ERROR, findNextErrorAction);
	actionManager->addAction(findNextErrorAction, UserAction::SubHasLine);

	KAction *findPreviousErrorAction = new KAction(actionCollection);
//  findPreviousErrorAction->setIcon( KIcon( "go-up" ) );
	findPreviousErrorAction->setIcon(KIcon("go-up-search"));
	findPreviousErrorAction->setText(i18n("Find Previous Error"));
	findPreviousErrorAction->setStatusTip(i18n("Find previous line with specified errors"));
	findPreviousErrorAction->setShortcut(KShortcut("Shift+F4"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(findPreviousErrorAction, SIGNAL(triggered()), this, SLOT(findPreviousError()));
	actionCollection->addAction(ACT_FIND_PREVIOUS_ERROR, findPreviousErrorAction);
	actionManager->addAction(findPreviousErrorAction, UserAction::SubHasLine);

	KAction *spellCheckAction = new KAction(actionCollection);
	spellCheckAction->setIcon(KIcon("tools-check-spelling"));
	spellCheckAction->setText(i18n("Spelling..."));
	spellCheckAction->setStatusTip(i18n("Check lines spelling"));
	connect(spellCheckAction, SIGNAL(triggered()), this, SLOT(spellCheck()));
	actionCollection->addAction(ACT_SPELL_CHECK, spellCheckAction);
	actionManager->addAction(spellCheckAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	KAction *toggleSelectedLinesMarkAction = new KAction(actionCollection);
	toggleSelectedLinesMarkAction->setText(i18n("Toggle Mark"));
	toggleSelectedLinesMarkAction->setStatusTip(i18n("Toggle selected lines mark"));
	toggleSelectedLinesMarkAction->setShortcut(KShortcut("Ctrl+M"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(toggleSelectedLinesMarkAction, SIGNAL(triggered()), this, SLOT(toggleSelectedLinesMark()));
	actionCollection->addAction(ACT_TOGGLE_SELECTED_LINES_MARK, toggleSelectedLinesMarkAction);
	actionManager->addAction(toggleSelectedLinesMarkAction, UserAction::HasSelection | UserAction::FullScreenOff);

	KAction *toggleSelectedLinesBoldAction = new KAction(actionCollection);
	toggleSelectedLinesBoldAction->setIcon(KIcon("format-text-bold"));
	toggleSelectedLinesBoldAction->setText(i18n("Toggle Bold"));
	toggleSelectedLinesBoldAction->setStatusTip(i18n("Toggle selected lines bold attribute"));
	toggleSelectedLinesBoldAction->setShortcut(KShortcut("Ctrl+B"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(toggleSelectedLinesBoldAction, SIGNAL(triggered()), this, SLOT(toggleSelectedLinesBold()));
	actionCollection->addAction(ACT_TOGGLE_SELECTED_LINES_BOLD, toggleSelectedLinesBoldAction);
	actionManager->addAction(toggleSelectedLinesBoldAction, UserAction::HasSelection | UserAction::FullScreenOff);

	KAction *toggleSelectedLinesItalicAction = new KAction(actionCollection);
	toggleSelectedLinesItalicAction->setIcon(KIcon("format-text-italic"));
	toggleSelectedLinesItalicAction->setText(i18n("Toggle Italic"));
	toggleSelectedLinesItalicAction->setStatusTip(i18n("Toggle selected lines italic attribute"));
	toggleSelectedLinesItalicAction->setShortcut(KShortcut("Ctrl+I"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(toggleSelectedLinesItalicAction, SIGNAL(triggered()), this, SLOT(toggleSelectedLinesItalic()));
	actionCollection->addAction(ACT_TOGGLE_SELECTED_LINES_ITALIC, toggleSelectedLinesItalicAction);
	actionManager->addAction(toggleSelectedLinesItalicAction, UserAction::HasSelection | UserAction::FullScreenOff);

	KAction *toggleSelectedLinesUnderlineAction = new KAction(actionCollection);
	toggleSelectedLinesUnderlineAction->setIcon(KIcon("format-text-underline"));
	toggleSelectedLinesUnderlineAction->setText(i18n("Toggle Underline"));
	toggleSelectedLinesUnderlineAction->setStatusTip(i18n("Toggle selected lines underline attribute"));
	toggleSelectedLinesUnderlineAction->setShortcut(KShortcut("Ctrl+U"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(toggleSelectedLinesUnderlineAction, SIGNAL(triggered()), this, SLOT(toggleSelectedLinesUnderline()));
	actionCollection->addAction(ACT_TOGGLE_SELECTED_LINES_UNDERLINE, toggleSelectedLinesUnderlineAction);
	actionManager->addAction(toggleSelectedLinesUnderlineAction, UserAction::HasSelection | UserAction::FullScreenOff);

	KAction *toggleSelectedLinesStrikeThroughAction = new KAction(actionCollection);
	toggleSelectedLinesStrikeThroughAction->setIcon(KIcon("format-text-strikethrough"));
	toggleSelectedLinesStrikeThroughAction->setText(i18n("Toggle Strike Through"));
	toggleSelectedLinesStrikeThroughAction->setStatusTip(i18n("Toggle selected lines strike through attribute"));
	toggleSelectedLinesStrikeThroughAction->setShortcut(KShortcut("Ctrl+T"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(toggleSelectedLinesStrikeThroughAction, SIGNAL(triggered()), this, SLOT(toggleSelectedLinesStrikeThrough()));
	actionCollection->addAction(ACT_TOGGLE_SELECTED_LINES_STRIKETHROUGH, toggleSelectedLinesStrikeThroughAction);
	actionManager->addAction(toggleSelectedLinesStrikeThroughAction, UserAction::HasSelection | UserAction::FullScreenOff);

	KAction *changeSelectedLinesColorAction = new KAction(actionCollection);
	changeSelectedLinesColorAction->setIcon(KIcon("format-text-color"));
	changeSelectedLinesColorAction->setText(i18n("Change Text Color"));
	changeSelectedLinesColorAction->setStatusTip(i18n("Change text color of selected lines"));
	changeSelectedLinesColorAction->setShortcut(KShortcut("Ctrl+Shift+C"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(changeSelectedLinesColorAction, SIGNAL(triggered()), this, SLOT(changeSelectedLinesColor()));
	actionCollection->addAction(ACT_CHANGE_SELECTED_LINES_TEXT_COLOR, changeSelectedLinesColorAction);
	actionManager->addAction(changeSelectedLinesColorAction, UserAction::HasSelection | UserAction::FullScreenOff);

	KAction *shiftAction = new KAction(actionCollection);
	shiftAction->setText(i18n("Shift..."));
	shiftAction->setStatusTip(i18n("Shift lines an specified amount of time"));
	shiftAction->setShortcut(KShortcut("Ctrl+D"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(shiftAction, SIGNAL(triggered()), this, SLOT(shiftLines()));
	actionCollection->addAction(ACT_SHIFT, shiftAction);
	actionManager->addAction(shiftAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	QString shiftTimeMillis(generalConfig()->linesQuickShiftAmount());

	KAction *shiftSelectedLinesFwdAction = new KAction(actionCollection);
	shiftSelectedLinesFwdAction->setShortcut(KShortcut("Shift++"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(shiftSelectedLinesFwdAction, SIGNAL(triggered()), this, SLOT(shiftSelectedLinesForwards()));
	actionCollection->addAction(ACT_SHIFT_SELECTED_LINES_FORWARDS, shiftSelectedLinesFwdAction);
	actionManager->addAction(shiftSelectedLinesFwdAction, UserAction::HasSelection | UserAction::FullScreenOff);

	KAction *shiftSelectedLinesBwdAction = new KAction(actionCollection);
	shiftSelectedLinesBwdAction->setShortcut(KShortcut("Shift+-"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(shiftSelectedLinesBwdAction, SIGNAL(triggered()), this, SLOT(shiftSelectedLinesBackwards()));
	actionCollection->addAction(ACT_SHIFT_SELECTED_LINES_BACKWARDS, shiftSelectedLinesBwdAction);
	actionManager->addAction(shiftSelectedLinesBwdAction, UserAction::HasSelection | UserAction::FullScreenOff);

	// update shiftSelectedLinesFwdAction and shiftSelectedLinesBwdAction texts and status tips
	onGeneralOptionChanged(GeneralConfig::keyLinesQuickShiftAmount(), QString::number(generalConfig()->linesQuickShiftAmount()));

	KAction *adjustAction = new KAction(actionCollection);
	adjustAction->setText(i18n("Adjust..."));
	adjustAction->setStatusTip(i18n("Linearly adjust all lines to a specified interval"));
	adjustAction->setShortcut(KShortcut("Ctrl+J"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(adjustAction, SIGNAL(triggered()), this, SLOT(adjustLines()));
	actionCollection->addAction(ACT_ADJUST, adjustAction);
	actionManager->addAction(adjustAction, UserAction::SubHasLines | UserAction::FullScreenOff);

	KAction *sortLinesAction = new KAction(actionCollection);
	sortLinesAction->setText(i18n("Sort..."));
	sortLinesAction->setStatusTip(i18n("Sort lines based on their show time"));
	connect(sortLinesAction, SIGNAL(triggered()), this, SLOT(sortLines()));
	actionCollection->addAction(ACT_SORT_LINES, sortLinesAction);
	actionManager->addAction(sortLinesAction, UserAction::SubHasLines | UserAction::FullScreenOff);

	KAction *changeFrameRateAction = new KAction(actionCollection);
	changeFrameRateAction->setText(i18n("Change Frame Rate..."));
	changeFrameRateAction->setStatusTip(i18n("Retime subtitle lines by reinterpreting its frame rate"));
	connect(changeFrameRateAction, SIGNAL(triggered()), this, SLOT(changeFrameRate()));
	actionCollection->addAction(ACT_CHANGE_FRAME_RATE, changeFrameRateAction);
	actionManager->addAction(changeFrameRateAction, UserAction::SubOpened | UserAction::FullScreenOff);

	KAction *durationLimitsAction = new KAction(actionCollection);
	durationLimitsAction->setText(i18n("Enforce Duration Limits..."));
	durationLimitsAction->setStatusTip(i18n("Enforce lines minimum and/or maximum duration limits"));
	durationLimitsAction->setShortcut(KShortcut("Ctrl+L"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(durationLimitsAction, SIGNAL(triggered()), this, SLOT(enforceDurationLimits()));
	actionCollection->addAction(ACT_DURATION_LIMITS, durationLimitsAction);
	actionManager->addAction(durationLimitsAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	KAction *autoDurationsAction = new KAction(actionCollection);
	autoDurationsAction->setText(i18n("Set Automatic Durations..."));
	autoDurationsAction->setStatusTip(i18n("Set lines durations based on amount of letters, words and line breaks"));
	connect(autoDurationsAction, SIGNAL(triggered()), this, SLOT(setAutoDurations()));
	actionCollection->addAction(ACT_AUTOMATIC_DURATIONS, autoDurationsAction);
	actionManager->addAction(autoDurationsAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	KAction *maximizeDurationsAction = new KAction(actionCollection);
	maximizeDurationsAction->setText(i18n("Maximize Durations..."));
	maximizeDurationsAction->setStatusTip(i18n("Extend lines durations up to their next lines show time"));
	connect(maximizeDurationsAction, SIGNAL(triggered()), this, SLOT(maximizeDurations()));
	actionCollection->addAction(ACT_MAXIMIZE_DURATIONS, maximizeDurationsAction);
	actionManager->addAction(maximizeDurationsAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	KAction *fixOverlappingLinesAction = new KAction(actionCollection);
	fixOverlappingLinesAction->setText(i18n("Fix Overlapping Times..."));
	fixOverlappingLinesAction->setStatusTip(i18n("Fix lines overlapping times"));
	connect(fixOverlappingLinesAction, SIGNAL(triggered()), this, SLOT(fixOverlappingLines()));
	actionCollection->addAction(ACT_FIX_OVERLAPPING_LINES, fixOverlappingLinesAction);
	actionManager->addAction(fixOverlappingLinesAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	KAction *syncWithSubtitleAction = new KAction(actionCollection);
	syncWithSubtitleAction->setText(i18n("Synchronize with Subtitle..."));
	syncWithSubtitleAction->setStatusTip(i18n("Copy timing information from another subtitle"));
	connect(syncWithSubtitleAction, SIGNAL(triggered()), this, SLOT(syncWithSubtitle()));
	actionCollection->addAction(ACT_SYNC_WITH_SUBTITLE, syncWithSubtitleAction);
	actionManager->addAction(syncWithSubtitleAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	KAction *breakLinesAction = new KAction(actionCollection);
	breakLinesAction->setText(i18n("Break Lines..."));
	breakLinesAction->setStatusTip(i18n("Automatically set line breaks"));
	connect(breakLinesAction, SIGNAL(triggered()), this, SLOT(breakLines()));
	actionCollection->addAction(ACT_ADJUST_TEXTS, breakLinesAction);
	actionManager->addAction(breakLinesAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	KAction *unbreakTextsAction = new KAction(actionCollection);
	unbreakTextsAction->setText(i18n("Unbreak Lines..."));
	unbreakTextsAction->setStatusTip(i18n("Remove line breaks from lines"));
	connect(unbreakTextsAction, SIGNAL(triggered()), this, SLOT(unbreakTexts()));
	actionCollection->addAction(ACT_UNBREAK_TEXTS, unbreakTextsAction);
	actionManager->addAction(unbreakTextsAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	KAction *simplifySpacesAction = new KAction(actionCollection);
	simplifySpacesAction->setText(i18n("Simplify Spaces..."));
	simplifySpacesAction->setStatusTip(i18n("Remove unneeded spaces from lines"));
	connect(simplifySpacesAction, SIGNAL(triggered()), this, SLOT(simplifySpaces()));
	actionCollection->addAction(ACT_SIMPLIFY_SPACES, simplifySpacesAction);
	actionManager->addAction(simplifySpacesAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	KAction *changeCaseAction = new KAction(actionCollection);
	changeCaseAction->setText(i18n("Change Case..."));
	changeCaseAction->setStatusTip(i18n("Change lines text to upper, lower, title or sentence case"));
	connect(changeCaseAction, SIGNAL(triggered()), this, SLOT(changeCase()));
	actionCollection->addAction(ACT_CHANGE_CASE, changeCaseAction);
	actionManager->addAction(changeCaseAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	KAction *fixPunctuationAction = new KAction(actionCollection);
	fixPunctuationAction->setText(i18n("Fix Punctuation..."));
	fixPunctuationAction->setStatusTip(i18n("Fix punctuation errors in lines"));
	connect(fixPunctuationAction, SIGNAL(triggered()), this, SLOT(fixPunctuation()));
	actionCollection->addAction(ACT_FIX_PUNCTUATION, fixPunctuationAction);
	actionManager->addAction(fixPunctuationAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	KAction *translateAction = new KAction(actionCollection);
	translateAction->setText(i18n("Translate..."));
	translateAction->setStatusTip(i18n("Translate lines texts using Google services"));
	connect(translateAction, SIGNAL(triggered()), this, SLOT(translate()));
	actionCollection->addAction(ACT_TRANSLATE, translateAction);
	actionManager->addAction(translateAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	KAction *editCurrentLineInPlaceAction = new KAction(actionCollection);
	editCurrentLineInPlaceAction->setText(i18n("Edit Line in Place"));
	editCurrentLineInPlaceAction->setStatusTip(i18n("Edit current line text in place"));
	editCurrentLineInPlaceAction->setShortcut(KShortcut("F2"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(editCurrentLineInPlaceAction, SIGNAL(triggered()), m_linesWidget, SLOT(editCurrentLineInPlace()));
	actionCollection->addAction(ACT_EDIT_CURRENT_LINE_IN_PLACE, editCurrentLineInPlaceAction);
	actionManager->addAction(editCurrentLineInPlaceAction, UserAction::HasSelection | UserAction::FullScreenOff);

	KAction *openVideoAction = new KAction(actionCollection);
	openVideoAction->setIcon(KIcon("document-open"));
	openVideoAction->setText(i18n("Open Video..."));
	openVideoAction->setStatusTip(i18n("Open video file"));
	connect(openVideoAction, SIGNAL(triggered()), this, SLOT(openVideo()));
	actionCollection->addAction(ACT_OPEN_VIDEO, openVideoAction);

	m_recentVideosAction = new KRecentFilesActionExt(actionCollection);
	m_recentVideosAction->setIcon(KIcon("document-open"));
	m_recentVideosAction->setText(i18n("Open &Recent Video"));
	m_recentVideosAction->setStatusTip(i18n("Open video file"));
	connect(m_recentVideosAction, SIGNAL(urlSelected(const KUrl &)), this, SLOT(openVideo(const KUrl &)));
	actionCollection->addAction(ACT_RECENT_VIDEOS, m_recentVideosAction);

	KAction *closeVideoAction = new KAction(actionCollection);
	closeVideoAction->setIcon(KIcon("window-close"));
	closeVideoAction->setText(i18n("Close Video"));
	closeVideoAction->setStatusTip(i18n("Close video file"));
	connect(closeVideoAction, SIGNAL(triggered()), m_player, SLOT(closeFile()));
	actionCollection->addAction(ACT_CLOSE_VIDEO, closeVideoAction);
	actionManager->addAction(closeVideoAction, UserAction::VideoOpened);

	KAction *extractVideoAudioAction = new KAction(actionCollection);
	extractVideoAudioAction->setIcon(KIcon("audio-extract"));
	extractVideoAudioAction->setText(i18n("Extract Audio"));
	extractVideoAudioAction->setStatusTip(i18n("Extract video's active audio stream"));
	connect(extractVideoAudioAction, SIGNAL(triggered()), this, SLOT(extractVideoAudio()));
	actionCollection->addAction(ACT_EXTRACT_VIDEO_AUDIO, extractVideoAudioAction);
	actionManager->addAction(extractVideoAudioAction, UserAction::VideoOpened);

	KAction *cancelAudioExtractionAction = new KAction(actionCollection);
	cancelAudioExtractionAction->setIcon(KIcon("dialog-cancel"));
	cancelAudioExtractionAction->setText(i18n("Cancel Audio Extraction"));
	cancelAudioExtractionAction->setStatusTip(i18n("Cancel video's audio stream extraction"));
	connect(cancelAudioExtractionAction, SIGNAL(triggered()), Decoder::instance(), SLOT(stop()));
	actionCollection->addAction(ACT_CANCEL_AUDIO_EXTRACTION, cancelAudioExtractionAction);
	actionManager->addAction(cancelAudioExtractionAction, UserAction::AudioDecoding);

	KToggleAction *fullScreenAction = new KToggleAction(actionCollection);
//  fullScreenAction->setChecked( false )
	fullScreenAction->setIcon(KIcon("view-fullscreen"));
	fullScreenAction->setText(i18n("Full Screen Mode"));
	fullScreenAction->setStatusTip(i18n("Toggle full screen mode"));
	fullScreenAction->setShortcut(KStandardShortcut::fullScreen(), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(fullScreenAction, SIGNAL(toggled(bool)), this, SLOT(setFullScreenMode(bool)));
	actionCollection->addAction(ACT_TOGGLE_FULL_SCREEN, fullScreenAction);

	KAction *stopAction = new KAction(actionCollection);
	stopAction->setIcon(KIcon("media-playback-stop"));
	stopAction->setText(i18n("Stop"));
	stopAction->setStatusTip(i18n("Stop video playback"));
	connect(stopAction, SIGNAL(triggered()), m_player, SLOT(stop()));
	actionCollection->addAction(ACT_STOP, stopAction);
	actionManager->addAction(stopAction, UserAction::VideoPlaying);

	KAction *playPauseAction = new KAction(actionCollection);
	playPauseAction->setIcon(KIcon("media-playback-start"));
	playPauseAction->setText(i18n("Play"));
	playPauseAction->setStatusTip(i18n("Toggle video playing/paused"));
	connect(playPauseAction, SIGNAL(triggered()), m_player, SLOT(togglePlayPaused()));
	actionCollection->addAction(ACT_PLAY_PAUSE, playPauseAction);
	actionManager->addAction(playPauseAction, UserAction::VideoOpened);

	KAction *seekBackwardsAction = new KAction(actionCollection);
	seekBackwardsAction->setIcon(KIcon("media-seek-backward"));
	seekBackwardsAction->setText(i18n("Seek Backwards"));
	seekBackwardsAction->setShortcut(KShortcut("Left"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(seekBackwardsAction, SIGNAL(triggered()), this, SLOT(seekBackwards()));
	actionCollection->addAction(ACT_SEEK_BACKWARDS, seekBackwardsAction);
	actionManager->addAction(seekBackwardsAction, UserAction::VideoPlaying);

	KAction *seekForwardsAction = new KAction(actionCollection);
	seekForwardsAction->setIcon(KIcon("media-seek-forward"));
	seekForwardsAction->setText(i18n("Seek Forwards"));
	seekForwardsAction->setShortcut(KShortcut("Right"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(seekForwardsAction, SIGNAL(triggered()), this, SLOT(seekForwards()));
	actionCollection->addAction(ACT_SEEK_FORWARDS, seekForwardsAction);
	actionManager->addAction(seekForwardsAction, UserAction::VideoPlaying);

	// update seekBackwardsAction and seekForwardsAction status tip
	onPlayerOptionChanged(PlayerConfig::keySeekJumpLength(), QString::number(playerConfig()->seekJumpLength()));

	KAction *seekToPrevLineAction = new KAction(actionCollection);
	seekToPrevLineAction->setIcon(KIcon("media-skip-backward"));
	seekToPrevLineAction->setText(i18n("Jump to Previous Line"));
	seekToPrevLineAction->setStatusTip(i18n("Seek to previous subtitle line show time"));
	seekToPrevLineAction->setShortcut(KShortcut("Shift+Left"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(seekToPrevLineAction, SIGNAL(triggered()), this, SLOT(seekToPrevLine()));
	actionCollection->addAction(ACT_SEEK_TO_PREVIOUS_LINE, seekToPrevLineAction);
	actionManager->addAction(seekToPrevLineAction, UserAction::SubHasLine | UserAction::VideoPlaying);

	KAction *seekToNextLineAction = new KAction(actionCollection);
	seekToNextLineAction->setIcon(KIcon("media-skip-forward"));
	seekToNextLineAction->setText(i18n("Jump to Next Line"));
	seekToNextLineAction->setStatusTip(i18n("Seek to next subtitle line show time"));
	seekToNextLineAction->setShortcut(KShortcut("Shift+Right"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(seekToNextLineAction, SIGNAL(triggered()), this, SLOT(seekToNextLine()));
	actionCollection->addAction(ACT_SEEK_TO_NEXT_LINE, seekToNextLineAction);
	actionManager->addAction(seekToNextLineAction, UserAction::SubHasLine | UserAction::VideoPlaying);

	KAction *setCurrentLineShowTimeFromVideoAction = new KAction(actionCollection);
//  setCurrentLineShowTimeFromVideoAction->setIcon( KIcon( "set-show-time" ) );
	setCurrentLineShowTimeFromVideoAction->setIcon(KIcon("zone-start"));
	setCurrentLineShowTimeFromVideoAction->setText(i18n("Set Current Line Show Time"));
	setCurrentLineShowTimeFromVideoAction->setStatusTip(i18n("Set current line show time to video position"));
	setCurrentLineShowTimeFromVideoAction->setShortcut(KShortcut("Shift+Z"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(setCurrentLineShowTimeFromVideoAction, SIGNAL(triggered()), this, SLOT(setCurrentLineShowTimeFromVideo()));
	actionCollection->addAction(ACT_SET_CURRENT_LINE_SHOW_TIME, setCurrentLineShowTimeFromVideoAction);
	actionManager->addAction(setCurrentLineShowTimeFromVideoAction, UserAction::HasSelection | UserAction::VideoPlaying);

	KAction *setCurrentLineHideTimeFromVideoAction = new KAction(actionCollection);
//  setCurrentLineHideTimeFromVideoAction->setIcon( KIcon( "set-hide-time" ) );
	setCurrentLineHideTimeFromVideoAction->setIcon(KIcon("zone-end"));
	setCurrentLineHideTimeFromVideoAction->setText(i18n("Set Current Line Hide Time"));
	setCurrentLineHideTimeFromVideoAction->setStatusTip(i18n("Set current line hide time to video position"));
	setCurrentLineHideTimeFromVideoAction->setShortcut(KShortcut("Shift+X"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(setCurrentLineHideTimeFromVideoAction, SIGNAL(triggered()), this, SLOT(setCurrentLineHideTimeFromVideo()));
	actionCollection->addAction(ACT_SET_CURRENT_LINE_HIDE_TIME, setCurrentLineHideTimeFromVideoAction);
	actionManager->addAction(setCurrentLineHideTimeFromVideoAction, UserAction::HasSelection | UserAction::VideoPlaying);

	KToggleAction *currentLineFollowsVideoAction = new KToggleAction(actionCollection);
	currentLineFollowsVideoAction->setIcon(KIcon("current-line-follows-video"));
	currentLineFollowsVideoAction->setText(i18n("Current Line Follows Video"));
	currentLineFollowsVideoAction->setStatusTip(i18n("Make current line follow the playing video position"));
	connect(currentLineFollowsVideoAction, SIGNAL(toggled(bool)), this, SLOT(onLinkCurrentLineToVideoToggled(bool)));
	actionCollection->addAction(ACT_CURRENT_LINE_FOLLOWS_VIDEO, currentLineFollowsVideoAction);

	KToggleAction *toggleMuteAction = new KToggleAction(actionCollection);
	toggleMuteAction->setIcon(KIcon("audio-volume-muted"));
	toggleMuteAction->setText(i18nc("@action:inmenu Toggle audio muted", "Mute"));
//  toggleMuteAction->setText( i18n( "Mute" ), i18n( "Unmute" ) );
	toggleMuteAction->setStatusTip(i18n("Enable/disable playback sound"));
	toggleMuteAction->setShortcut(KShortcut("/"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(toggleMuteAction, SIGNAL(toggled(bool)), m_player, SLOT(setMuted(bool)));
	actionCollection->addAction(ACT_TOGGLE_MUTED, toggleMuteAction);

	KAction *increaseVolumeAction = new KAction(actionCollection);
	increaseVolumeAction->setIcon(KIcon("audio-volume-high"));
	increaseVolumeAction->setText(i18n("Increase Volume"));
	increaseVolumeAction->setStatusTip(i18n("Increase volume by 5%"));
	increaseVolumeAction->setShortcut(Qt::Key_Plus, KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(increaseVolumeAction, SIGNAL(triggered()), m_player, SLOT(increaseVolume()));
	actionCollection->addAction(ACT_INCREASE_VOLUME, increaseVolumeAction);

	KAction *decreaseVolumeAction = new KAction(actionCollection);
	decreaseVolumeAction->setIcon(KIcon("audio-volume-low"));
	decreaseVolumeAction->setText(i18n("Decrease Volume"));
	decreaseVolumeAction->setStatusTip(i18n("Decrease volume by 5%"));
	decreaseVolumeAction->setShortcut(Qt::Key_Minus, KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(decreaseVolumeAction, SIGNAL(triggered()), m_player, SLOT(decreaseVolume()));
	actionCollection->addAction(ACT_DECREASE_VOLUME, decreaseVolumeAction);

	KSelectAction *setActiveAudioStreamAction = new KSelectAction(actionCollection);
	setActiveAudioStreamAction->setIcon(KIcon("select-stream"));
	setActiveAudioStreamAction->setText(i18n("Audio Streams"));
	setActiveAudioStreamAction->setStatusTip(i18n("Select active audio stream"));
	connect(setActiveAudioStreamAction, SIGNAL(triggered(int)), m_player, SLOT(setActiveAudioStream(int)));
	actionCollection->addAction(ACT_SET_ACTIVE_AUDIO_STREAM, setActiveAudioStreamAction);
	actionManager->addAction(setActiveAudioStreamAction, UserAction::VideoOpened);

	KAction *increaseSubtitleFontAction = new KAction(actionCollection);
	increaseSubtitleFontAction->setIcon(KIcon("format-font-size-more"));
	increaseSubtitleFontAction->setText(i18n("Increase Font Size"));
	increaseSubtitleFontAction->setStatusTip(i18n("Increase subtitles font size by 1 point"));
	increaseSubtitleFontAction->setShortcut(KShortcut("Alt++"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(increaseSubtitleFontAction, SIGNAL(triggered()), m_playerWidget, SLOT(increaseFontSize()));
	actionCollection->addAction(ACT_INCREASE_SUBTITLE_FONT, increaseSubtitleFontAction);

	KAction *decreaseSubtitleFontAction = new KAction(actionCollection);
	decreaseSubtitleFontAction->setIcon(KIcon("format-font-size-less"));
	decreaseSubtitleFontAction->setText(i18n("Decrease Font Size"));
	decreaseSubtitleFontAction->setStatusTip(i18n("Decrease subtitles font size by 1 point"));
	decreaseSubtitleFontAction->setShortcut(KShortcut("Alt+-"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(decreaseSubtitleFontAction, SIGNAL(triggered()), m_playerWidget, SLOT(decreaseFontSize()));
	actionCollection->addAction(ACT_DECREASE_SUBTITLE_FONT, decreaseSubtitleFontAction);

	KSelectAction *setActiveSubtitleStreamAction = new KSelectAction(actionCollection);
	setActiveSubtitleStreamAction->setIcon(KIcon("select-stream"));
	setActiveSubtitleStreamAction->setText(i18n("Subtitle Streams"));
	setActiveSubtitleStreamAction->setStatusTip(i18n("Select active subtitle stream"));
	connect(setActiveSubtitleStreamAction, SIGNAL(triggered(int)), this, SLOT(setActiveSubtitleStream(int)));
	actionCollection->addAction(ACT_SET_ACTIVE_SUBTITLE_STREAM, setActiveSubtitleStreamAction);
	actionManager->addAction(setActiveSubtitleStreamAction, UserAction::SubTrOpened);

	KAction *shiftToVideoPositionAction = new KAction(actionCollection);
	shiftToVideoPositionAction->setText(i18n("Shift to Video Position"));
	shiftToVideoPositionAction->setStatusTip(i18n("Set current line show time to video position by equally shifting all lines"));
	shiftToVideoPositionAction->setShortcut(KShortcut("Shift+A"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(shiftToVideoPositionAction, SIGNAL(triggered()), this, SLOT(shiftToVideoPosition()));
	actionCollection->addAction(ACT_SHIFT_TO_VIDEO_POSITION, shiftToVideoPositionAction);
	actionManager->addAction(shiftToVideoPositionAction, UserAction::HasSelection | UserAction::VideoPlaying | UserAction::FullScreenOff);

	KAction *adjustToVideoPositionAnchorLastAction = new KAction(actionCollection);
	adjustToVideoPositionAnchorLastAction->setText(i18n("Adjust to Video Position (Anchor Last Line)"));
	adjustToVideoPositionAnchorLastAction->setStatusTip(i18n("Set current line to video position by fixing the last line and linearly adjusting the other ones"));
	adjustToVideoPositionAnchorLastAction->setShortcut(KShortcut("Alt+Z"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(adjustToVideoPositionAnchorLastAction, SIGNAL(triggered()), this, SLOT(adjustToVideoPositionAnchorLast()));
	actionCollection->addAction(ACT_ADJUST_TO_VIDEO_POSITION_A_L, adjustToVideoPositionAnchorLastAction);
	actionManager->addAction(adjustToVideoPositionAnchorLastAction, UserAction::HasSelection | UserAction::VideoPlaying | UserAction::FullScreenOff);

	KAction *adjustToVideoPositionAnchorFirstAction = new KAction(actionCollection);
	adjustToVideoPositionAnchorFirstAction->setText(i18n("Adjust to Video Position (Anchor First Line)"));
	adjustToVideoPositionAnchorFirstAction->setStatusTip(i18n("Set current line to video position by fixing the first line and linearly adjusting the other ones"));
	adjustToVideoPositionAnchorFirstAction->setShortcut(KShortcut("Alt+X"), KAction::DefaultShortcut | KAction::ActiveShortcut);
	connect(adjustToVideoPositionAnchorFirstAction, SIGNAL(triggered()), this, SLOT(adjustToVideoPositionAnchorFirst()));
	actionCollection->addAction(ACT_ADJUST_TO_VIDEO_POSITION_A_F, adjustToVideoPositionAnchorFirstAction);
	actionManager->addAction(adjustToVideoPositionAnchorFirstAction, UserAction::HasSelection | UserAction::VideoPlaying | UserAction::FullScreenOff);

	KAction *scriptsManagerAction = new KAction(actionCollection);
	scriptsManagerAction->setIcon(KIcon("folder-development"));
	scriptsManagerAction->setText(i18nc("@action:inmenu Manage user scripts", "Scripts Manager..."));
	scriptsManagerAction->setStatusTip(i18n("Manage user scripts"));
	connect(scriptsManagerAction, SIGNAL(triggered()), m_scriptsManager, SLOT(showDialog()));
	actionCollection->addAction(ACT_SCRIPTS_MANAGER, scriptsManagerAction);
	actionManager->addAction(scriptsManagerAction, UserAction::FullScreenOff);

//  KAction* openAudioLevelsAction = new KAction( actionCollection );
//  openAudioLevelsAction->setIcon( "fileopen" );
//  openAudioLevelsAction->setText( i18n( "Open Levels..." ) );
//  openAudioLevelsAction->setStatusTip( i18n( "Open audio levels file" ) );
//  connect( openAudioLevelsAction, SIGNAL( triggered() ), this, SLOT( openAudioLevels() ) );
//  actionCollection->addAction( ACT_OPEN_WAVEFORM, openAudioLevelsAction );
//
//
//  m_recentAudioLevelsAction = new KRecentFilesActionExt( actionCollection );
//  m_recentAudioLevelsAction->setIcon( "fileopen" );
//  m_recentAudioLevelsAction->setText( i18n( "Open &Recent Levels" ) );
//  m_recentAudioLevelsAction->setStatusTip( i18n( "Open audio levels file" ) );
//  connect( m_recentAudioLevelsAction, SIGNAL(urlSelected(const KUrl&)), this, SLOT(openAudioLevels(const KUrl&)) );
//  actionCollection->addAction( ACT_RECENT_WAVEFORMS, m_recentAudioLevelsAction );
//
//
//  KAction* saveAudioLevelsAsAction = new KAction( actionCollection );
//  saveAudioLevelsAsAction->setIcon( "filesaveas" );
//  saveAudioLevelsAsAction->setText( i18n( "Save Levels As..." ) );
//  saveAudioLevelsAsAction->setStatusTip( i18n( "Save audio levels file" ) );
//  connect( saveAudioLevelsAsAction, SIGNAL( triggered() ), this, SLOT( saveAudioLevelsAs() ) );
//  actionCollection->addAction( ACT_SAVE_LEVELS_AS, saveAudioLevelsAsAction );
//  actionManager->addAction( saveAudioLevelsAsAction, UserAction::AudioLevelsOpened );
//
//
//  KAction* closeAudioLevelsAction = new KAction( actionCollection );
//  closeAudioLevelsAction->setIcon( "fileclose" );
//  closeAudioLevelsAction->setText( i18n( "Close Levels" ) );
//  closeAudioLevelsAction->setStatusTip( i18n( "Close audio levels file" ) );
//  connect( closeAudioLevelsAction, SIGNAL( triggered() ), this, SLOT( closeAudioLevels() ) );
//  actionCollection->addAction( ACT_CLOSE_WAVEFORM, closeAudioLevelsAction );
//  actionManager->addAction( closeAudioLevelsAction, UserAction::AudioLevelsOpened );
//
//
//  KAction* increaseAudioLevelsVZoomAction = new KAction( actionCollection );
//  increaseAudioLevelsVZoomAction->setIcon( "viewmag+" );
//  increaseAudioLevelsVZoomAction->setText( i18n( "Increase Vertical Zoom" ) );
//  increaseAudioLevelsVZoomAction->setStatusTip( i18n( "Increase audio levels vertical zoom" ) );
//  connect( increaseAudioLevelsVZoomAction, SIGNAL( triggered() ), this, SLOT( increaseAudioLevelsVZoom() ) );
//  actionCollection->addAction( ACT_INCREASE_LEVELS_V_ZOOM, increaseAudioLevelsVZoomAction );
//  actionManager->addAction( increaseAudioLevelsVZoomAction, UserAction::AudioLevelsOpened );
//
//
//  KAction* decreaseAudioLevelsVZoomAction = new KAction( actionCollection );
//  decreaseAudioLevelsVZoomAction->setIcon( "viewmag-" );
//  decreaseAudioLevelsVZoomAction->setText( i18n( "Decrease Vertical Zoom" ) );
//  decreaseAudioLevelsVZoomAction->setStatusTip( i18n( "Decrease audio levels vertical zoom" ) );
//  connect( decreaseAudioLevelsVZoomAction, SIGNAL( triggered() ), this, SLOT( decreaseAudioLevelsVZoom() ) );
//  actionCollection->addAction( ACT_DECREASE_LEVELS_V_ZOOM, decreaseAudioLevelsVZoomAction );
//  actionManager->addAction( decreaseAudioLevelsVZoomAction, UserAction::AudioLevelsOpened );
//
//
//  KAction* increaseAudioLevelsHZoomAction = new KAction( actionCollection );
//  increaseAudioLevelsHZoomAction->setIcon( "viewmag+" );
//  increaseAudioLevelsHZoomAction->setText( i18n( "Increase Horizontal Zoom" ) );
//  increaseAudioLevelsHZoomAction->setStatusTip( i18n( "Increase audio levels horizontal zoom" ) );
//  connect( increaseAudioLevelsHZoomAction, SIGNAL( triggered() ), this, SLOT( increaseAudioLevelsHZoom() ) );
//  actionCollection->addAction( ACT_INCREASE_LEVELS_H_ZOOM, increaseAudioLevelsHZoomAction );
//  actionManager->addAction( increaseAudioLevelsHZoomAction, UserAction::AudioLevelsOpened );
//
//
//  KAction* decreaseAudioLevelsHZoomAction = new KAction( actionCollection );
//  decreaseAudioLevelsHZoomAction->setIcon( "viewmag-" );
//  decreaseAudioLevelsHZoomAction->setText( i18n( "Decrease Horizontal Zoom" ) );
//  decreaseAudioLevelsHZoomAction->setStatusTip( i18n( "Decrease audio levels horizontal zoom" ) );
//  connect( decreaseAudioLevelsHZoomAction, SIGNAL( triggered() ), this, SLOT( decreaseAudioLevelsHZoom() ) );
//  actionCollection->addAction( ACT_DECREASE_LEVELS_H_ZOOM, decreaseAudioLevelsHZoomAction );
//  actionManager->addAction( decreaseAudioLevelsHZoomAction, UserAction::AudioLevelsOpened );
}

/// BEGIN ACTION HANDLERS

Time
Application::videoPosition(bool compensate)
{
	if(compensate && !m_player->isPaused())
		return Time((long)(m_player->position() * 1000) - generalConfig()->grabbedPositionCompensation());
	else
		return Time((long)(m_player->position() * 1000));
}

void
Application::undo()
{
	if(m_subtitle)
		m_subtitle->actionManager().undo();
}

void
Application::redo()
{
	if(m_subtitle)
		m_subtitle->actionManager().redo();
}

QTextCodec *
Application::codecForUrl(const KUrl &url, bool useRecentFiles, bool useDefault)
{
	QString encoding = url.fileEncoding();

	if(useRecentFiles) {
		if(encoding.isEmpty())
			m_recentSubtitlesAction->encodingForUrl(url);
		if(encoding.isEmpty())
			encoding = m_recentSubtitlesTrAction->encodingForUrl(url);
	}

	QTextCodec *codec = 0;

	if(!encoding.isEmpty()) {
		bool codecFound = false;
		codec = KGlobal::charsets()->codecForName(encoding, codecFound);
		if(!codecFound)
			codec = 0;
	}

	if(!codec && useDefault)
		codec = generalConfig()->defaultSubtitlesCodec();

	return codec;
}

QTextCodec *
Application::codecForEncoding(const QString &encoding, bool useDefault)
{
	QTextCodec *codec = 0;

	if(!encoding.isEmpty()) {
		bool codecFound = false;
		codec = KGlobal::charsets()->codecForName(encoding, codecFound);
		if(!codecFound)
			codec = 0;
	}

	if(!codec && useDefault)
		codec = generalConfig()->defaultSubtitlesCodec();

	return codec;
}

bool
Application::acceptClashingUrls(const KUrl &subtitleUrl, const KUrl &subtitleTrUrl)
{
	KUrl url(subtitleUrl);
	url.setFileEncoding(QString());
	KUrl trUrl(subtitleTrUrl);
	trUrl.setFileEncoding(QString());

	if(url != trUrl || url.isEmpty() || trUrl.isEmpty())
		return true;

	return KMessageBox::Continue == KMessageBox::warningContinueCancel(m_mainWindow, i18n("The requested action would make the subtitle and its translation share the same file, possibly resulting in loss of information when saving.\nAre you sure you want to continue?"), i18n("Conflicting Subtitle Files")
																	   );
}

void
Application::newSubtitle()
{
	if(!closeSubtitle())
		return;

	m_subtitle = new Subtitle();

	emit subtitleOpened(m_subtitle);

	connect(m_subtitle, SIGNAL(primaryDirtyStateChanged(bool)), this, SLOT(updateTitle()));
	connect(m_subtitle, SIGNAL(secondaryDirtyStateChanged(bool)), this, SLOT(updateTitle()));
	connect(&m_subtitle->actionManager(), SIGNAL(stateChanged()), this, SLOT(updateUndoRedoToolTips()));

	updateTitle();
}

void
Application::openSubtitle()
{
	OpenSubtitleDialog openDlg(true, m_lastSubtitleUrl.prettyUrl(), QString());

	if(openDlg.exec() == QDialog::Accepted) {
		if(!acceptClashingUrls(openDlg.selectedUrl(), m_subtitleTrUrl))
			return;

		if(!closeSubtitle())
			return;

		m_lastSubtitleUrl = openDlg.selectedUrl();

		KUrl fileUrl = m_lastSubtitleUrl;
		fileUrl.setFileEncoding(openDlg.selectedEncoding());
		openSubtitle(fileUrl);
	}
}

void
Application::openSubtitle(const KUrl &url, bool warnClashingUrls)
{
	if(warnClashingUrls && !acceptClashingUrls(url, m_subtitleTrUrl))
		return;

	if(!closeSubtitle())
		return;

	QTextCodec *codec = codecForUrl(url, true, false);

	KUrl fileUrl = url;
	fileUrl.setFileEncoding(QString());

	m_subtitle = new Subtitle();

	if(FormatManager::instance().readSubtitle(*m_subtitle, true, fileUrl, KEncodingDetector::SemiautomaticDetection, &codec, &m_subtitleEOL, &m_subtitleFormat)) {
		// The loading of the subtitle shouldn't be an undoable action as there's no state before it
		m_subtitle->actionManager().clearHistory();
		m_subtitle->clearPrimaryDirty();
		m_subtitle->clearSecondaryDirty();

		emit subtitleOpened(m_subtitle);

		m_subtitleUrl = fileUrl;
		m_subtitleFileName = QFileInfo(m_subtitleUrl.path()).fileName();
		m_subtitleEncoding = codec->name();

		fileUrl.setFileEncoding(codec->name());
		m_recentSubtitlesAction->addUrl(fileUrl);

		m_reopenSubtitleAsAction->setCurrentCodec(codec);

		connect(m_subtitle, SIGNAL(primaryDirtyStateChanged(bool)), this, SLOT(updateTitle()));
		connect(m_subtitle, SIGNAL(secondaryDirtyStateChanged(bool)), this, SLOT(updateTitle()));
		connect(&m_subtitle->actionManager(), SIGNAL(stateChanged()), this, SLOT(updateUndoRedoToolTips()));

		updateTitle();

		if(m_subtitleUrl.isLocalFile() && generalConfig()->automaticVideoLoad()) {
			static const QStringList videoExtensions(QString("avi ogm mkv mpeg mpg mp4 rv wmv").split(' '));

			QFileInfo subtitleFileInfo(m_subtitleUrl.path());

			QString subtitleFileName = m_subtitleFileName.toLower();
			QString videoFileName = QFileInfo(m_player->filePath()).completeBaseName().toLower();

			if(videoFileName.isEmpty() || subtitleFileName.indexOf(videoFileName) != 0) {
				QStringList subtitleDirFiles = subtitleFileInfo.dir().entryList(QDir::Files | QDir::Readable);
				for(QStringList::ConstIterator it = subtitleDirFiles.begin(), end = subtitleDirFiles.end(); it != end; ++it) {
					QFileInfo fileInfo(*it);
					if(videoExtensions.contains(fileInfo.suffix().toLower())) {
						if(subtitleFileName.indexOf(fileInfo.completeBaseName().toLower()) == 0) {
							KUrl auxUrl;
							auxUrl.setProtocol("file");
							auxUrl.setPath(subtitleFileInfo.dir().filePath(*it));
							openVideo(auxUrl);
							break;
						}
					}
				}
			}
		}
	} else {
		delete m_subtitle;
		m_subtitle = 0;

		KMessageBox::sorry(m_mainWindow, i18n("<qt>Could not parse the subtitle file.<br/>This may have been caused by usage of the wrong encoding.</qt>"));
	}
}

void
Application::reopenSubtitleWithDetectScript(KEncodingDetector::AutoDetectScript autodetectScript)
{
	reopenSubtitleWithCodecOrDetectScript((QTextCodec *)0, autodetectScript);
}

void
Application::reopenSubtitleWithCodec(QTextCodec *codec)
{
	reopenSubtitleWithCodecOrDetectScript(codec, KEncodingDetector::SemiautomaticDetection);
}

void
Application::reopenSubtitleWithCodecOrDetectScript(QTextCodec *codec, KEncodingDetector::AutoDetectScript autodetectScript)
{
	if(m_subtitleUrl.isEmpty())
		return;

	Subtitle *subtitle = new Subtitle();
	Format::NewLine subtitleEOL;
	QString subtitleFormat;

	if(!FormatManager::instance().readSubtitle(*subtitle, true, m_subtitleUrl, autodetectScript, &codec, &subtitleEOL, &subtitleFormat)) {
		delete subtitle;
		KMessageBox::sorry(m_mainWindow, i18n("<qt>Could not parse the subtitle file.<br/>This may have been caused by usage of the wrong encoding.</qt>"));
		return;
	}

	emit subtitleClosed();

	if(m_translationMode) {
		subtitle->setSecondaryData(*m_subtitle, false);
	}

	delete m_subtitle;
	m_subtitle = subtitle;

	// The loading of the subtitle shouldn't be an undoable action as there's no state before it
	m_subtitle->actionManager().clearHistory();
	m_subtitle->clearPrimaryDirty();
	m_subtitle->clearSecondaryDirty();

	emit subtitleOpened(m_subtitle);

	m_subtitleEncoding = codec->name();
	m_subtitleEOL = subtitleEOL;
	m_subtitleFormat = subtitleFormat;

	m_reopenSubtitleAsAction->setCurrentCodec(codec);

	KUrl fileUrl = m_subtitleUrl;
	fileUrl.setFileEncoding(codec->name());
	m_recentSubtitlesAction->addUrl(fileUrl);

	connect(m_subtitle, SIGNAL(primaryDirtyStateChanged(bool)), this, SLOT(updateTitle()));
	connect(m_subtitle, SIGNAL(secondaryDirtyStateChanged(bool)), this, SLOT(updateTitle()));
	connect(&m_subtitle->actionManager(), SIGNAL(stateChanged()), this, SLOT(updateUndoRedoToolTips()));
}

bool
Application::saveSubtitle()
{
	if(m_subtitleUrl.isEmpty() || m_subtitleEncoding.isEmpty() || !FormatManager::instance().hasOutput(m_subtitleFormat))
		return saveSubtitleAs();

	bool codecFound = true;
	QTextCodec *codec = KGlobal::charsets()->codecForName(m_subtitleEncoding, codecFound);
	if(!codecFound)
		codec = KGlobal::locale()->codecForEncoding();

	if(FormatManager::instance().writeSubtitle(*m_subtitle, true, m_subtitleUrl, codec, m_subtitleEOL, m_subtitleFormat, true)) {
		m_subtitle->clearPrimaryDirty();

		KUrl recentUrl = m_subtitleUrl;
		recentUrl.setFileEncoding(codec->name());
		m_recentSubtitlesAction->addUrl(recentUrl);

		m_reopenSubtitleAsAction->setCurrentCodec(codec);

		updateTitle();

		return true;
	} else {
		KMessageBox::sorry(m_mainWindow, i18n("There was an error saving the subtitle."));
		return false;
	}
}

bool
Application::saveSubtitleAs()
{
	SaveSubtitleDialog saveDlg(true, m_subtitleUrl.prettyUrl(), m_subtitleEncoding.isEmpty() ? generalConfig()->defaultSubtitlesEncoding() : m_subtitleEncoding, m_subtitleEOL, m_subtitleFormat);

	if(saveDlg.exec() == QDialog::Accepted) {
		if(!acceptClashingUrls(saveDlg.selectedUrl(), m_subtitleTrUrl))
			return false;

		m_subtitleUrl = saveDlg.selectedUrl();
		m_subtitleFileName = QFileInfo(m_subtitleUrl.path()).completeBaseName();
		m_subtitleEncoding = saveDlg.selectedEncoding();
		m_subtitleFormat = saveDlg.selectedFormat();
		m_subtitleEOL = saveDlg.selectedNewLine();

		return saveSubtitle();
	}

	return false;
}

bool
Application::closeSubtitle()
{
	if(m_subtitle) {
		if(m_translationMode && m_subtitle->isSecondaryDirty()) {
			int result = KMessageBox::warningYesNoCancel(0,
														 i18n("Currently opened translation subtitle has unsaved changes.\nDo you want to save them?"),
														 i18n("Close Translation Subtitle") + " - SubtitleComposer");
			if(result == KMessageBox::Cancel)
				return false;
			else if(result == KMessageBox::Yes)
				if(!saveSubtitleTr())
					return false;
		}

		if(m_subtitle->isPrimaryDirty()) {
			int result = KMessageBox::warningYesNoCancel(0,
														 i18n("Currently opened subtitle has unsaved changes.\nDo you want to save them?"),
														 i18n("Close Subtitle") + " - SubtitleComposer");
			if(result == KMessageBox::Cancel)
				return false;
			else if(result == KMessageBox::Yes)
				if(!saveSubtitle())
					return false;
		}

		if(m_translationMode) {
			m_translationMode = false;
			emit translationModeChanged(false);
		}

		emit subtitleClosed();

		delete m_subtitle;
		m_subtitle = 0;

		m_subtitleUrl = KUrl();
		m_subtitleFileName.clear();
		m_subtitleEncoding.clear();
		m_subtitleFormat.clear();

		m_subtitleTrUrl = KUrl();
		m_subtitleTrFileName.clear();
		m_subtitleTrEncoding.clear();
		m_subtitleTrFormat.clear();

		updateTitle();
	}

	return true;
}

void
Application::newSubtitleTr()
{
	if(!closeSubtitleTr())
		return;

	m_translationMode = true;
	emit translationModeChanged(true);

	updateTitle();
}

void
Application::openSubtitleTr()
{
	if(!m_subtitle)
		return;

	OpenSubtitleDialog openDlg(false, m_lastSubtitleUrl.prettyUrl(), QString());

	if(openDlg.exec() == QDialog::Accepted) {
		if(!acceptClashingUrls(m_subtitleUrl, openDlg.selectedUrl()))
			return;

		if(!closeSubtitleTr())
			return;

		m_lastSubtitleUrl = openDlg.selectedUrl();

		KUrl fileUrl = m_lastSubtitleUrl;
		fileUrl.setFileEncoding(openDlg.selectedEncoding());
		openSubtitleTr(fileUrl);
	}
}

void
Application::openSubtitleTr(const KUrl &url, bool warnClashingUrls)
{
	if(!m_subtitle)
		return;

	if(warnClashingUrls && !acceptClashingUrls(m_subtitleUrl, url))
		return;

	if(!closeSubtitleTr())
		return;

	QTextCodec *codec = codecForUrl(url, true, false);

	KUrl fileUrl = url;
	fileUrl.setFileEncoding(QString());

	if(FormatManager::instance().readSubtitle(*m_subtitle, false, fileUrl, KEncodingDetector::SemiautomaticDetection, &codec, &m_subtitleTrEOL, &m_subtitleTrFormat)) {
		m_subtitleTrUrl = fileUrl;
		m_subtitleTrFileName = QFileInfo(m_subtitleTrUrl.path()).fileName();
		m_subtitleTrEncoding = codec->name();

		fileUrl.setFileEncoding(codec->name());
		m_recentSubtitlesTrAction->addUrl(fileUrl);

		QStringList subtitleStreams;
		subtitleStreams.append(i18nc("@item:inmenu Display primary text in video player", "Primary"));
		subtitleStreams.append(i18nc("@item:inmenu Display translation text in video player", "Translation"));
		KSelectAction *activeSubtitleStreamAction = (KSelectAction *)action(ACT_SET_ACTIVE_SUBTITLE_STREAM);
		activeSubtitleStreamAction->setItems(subtitleStreams);
		activeSubtitleStreamAction->setCurrentItem(0);

		m_translationMode = true;

		updateTitle();
	} else
		KMessageBox::sorry(m_mainWindow, i18n("<qt>Could not parse the subtitle file.<br/>This may have been caused by usage of the wrong encoding.</qt>"));

	// After the loading of the translation subtitle we must clear the history or (from
	// a user POV) it would be possible to execute (undo) actions which would result in an
	// unexpected state.
	m_subtitle->actionManager().clearHistory();
	// We don't clear the primary dirty state because the loading of the translation
	// only changes it when actually needed (i.e., when the translation had more lines)
	m_subtitle->clearSecondaryDirty();

	if(m_translationMode)
		emit translationModeChanged(true);
}

void
Application::reopenSubtitleTrWithDetectScript(KEncodingDetector::AutoDetectScript autodetectScript)
{
	reopenSubtitleTrWithCodecOrDetectScript((QTextCodec *)0, autodetectScript);
}

void
Application::reopenSubtitleTrWithCodec(QTextCodec *codec)
{
	reopenSubtitleTrWithCodecOrDetectScript(codec, KEncodingDetector::SemiautomaticDetection);
}

void
Application::reopenSubtitleTrWithCodecOrDetectScript(QTextCodec *codec, KEncodingDetector::AutoDetectScript autodetectScript)
{
	if(m_subtitleTrUrl.isEmpty())
		return;

	Subtitle *subtitleTr = new Subtitle();
	Format::NewLine subtitleTrEOL;
	QString subtitleTrFormat;

	if(!FormatManager::instance().readSubtitle(*subtitleTr, false, m_subtitleTrUrl, autodetectScript, &codec, &subtitleTrEOL, &subtitleTrFormat)) {
		delete subtitleTr;
		KMessageBox::sorry(m_mainWindow, i18n("<qt>Could not parse the subtitle file.<br/>This may have been caused by usage of the wrong encoding.</qt>"));
		return;
	}

	emit subtitleClosed();

	subtitleTr->setPrimaryData(*m_subtitle, true);

	delete m_subtitle;
	m_subtitle = subtitleTr;

	// After the loading of the translation subtitle we must clear the history or (from
	// a user POV) it would be possible to execute (undo) actions which would result in an
	// unexpected state.
	m_subtitle->actionManager().clearHistory();
	// We don't clear the primary dirty state because the loading of the translation
	// only changes it when actually needed (i.e., when the translation had more lines)
	m_subtitle->clearPrimaryDirty();        // TODO is this needed?
	m_subtitle->clearSecondaryDirty();

	emit subtitleOpened(m_subtitle);

	m_subtitleTrEncoding = codec->name();
	m_subtitleTrEOL = subtitleTrEOL;
	m_subtitleTrFormat = subtitleTrFormat;

	m_reopenSubtitleTrAsAction->setCurrentCodec(codec);

	KUrl fileUrl = m_subtitleTrUrl;
	fileUrl.setFileEncoding(codec->name());
	m_recentSubtitlesTrAction->addUrl(fileUrl);

	connect(m_subtitle, SIGNAL(primaryDirtyStateChanged(bool)), this, SLOT(updateTitle()));
	connect(m_subtitle, SIGNAL(secondaryDirtyStateChanged(bool)), this, SLOT(updateTitle()));
	connect(&m_subtitle->actionManager(), SIGNAL(stateChanged()), this, SLOT(updateUndoRedoToolTips()));
}

bool
Application::saveSubtitleTr()
{
	if(m_subtitleTrUrl.isEmpty() || m_subtitleTrEncoding.isEmpty() || !FormatManager::instance().hasOutput(m_subtitleTrFormat))
		return saveSubtitleTrAs();

	bool codecFound = true;
	QTextCodec *codec = KGlobal::charsets()->codecForName(m_subtitleTrEncoding, codecFound);
	if(!codecFound)
		codec = KGlobal::locale()->codecForEncoding();

	if(FormatManager::instance().writeSubtitle(*m_subtitle, false, m_subtitleTrUrl, codec, m_subtitleTrEOL, m_subtitleTrFormat, true)) {
		m_subtitle->clearSecondaryDirty();

		KUrl recentUrl = m_subtitleTrUrl;
		recentUrl.setFileEncoding(codec->name());
		m_recentSubtitlesTrAction->addUrl(recentUrl);

		updateTitle();

		return true;
	} else {
		KMessageBox::sorry(m_mainWindow, i18n("There was an error saving the translation subtitle."));
		return false;
	}
}

bool
Application::saveSubtitleTrAs()
{
	SaveSubtitleDialog saveDlg(false, m_subtitleTrUrl.prettyUrl(), m_subtitleTrEncoding.isEmpty() ? generalConfig()->defaultSubtitlesEncoding() : m_subtitleTrEncoding, m_subtitleTrEOL, m_subtitleTrFormat);

	if(saveDlg.exec() == QDialog::Accepted) {
		if(!acceptClashingUrls(m_subtitleUrl, saveDlg.selectedUrl()))
			return false;

		m_subtitleTrUrl = saveDlg.selectedUrl();
		m_subtitleTrFileName = QFileInfo(m_subtitleTrUrl.path()).completeBaseName();
		m_subtitleTrEncoding = saveDlg.selectedEncoding();
		m_subtitleTrFormat = saveDlg.selectedFormat();
		m_subtitleTrEOL = saveDlg.selectedNewLine();

		return saveSubtitleTr();
	}

	return false;
}

bool
Application::closeSubtitleTr()
{
	if(m_subtitle && m_translationMode) {
		if(m_translationMode && m_subtitle->isSecondaryDirty()) {
			int result = KMessageBox::warningYesNoCancel(0,
														 i18n("Currently opened translation subtitle has unsaved changes.\nDo you want to save them?"),
														 i18n("Close Translation Subtitle") + " - SubtitleComposer");
			if(result == KMessageBox::Cancel)
				return false;
			else if(result == KMessageBox::Yes)
				if(!saveSubtitleTr())
					return false;
		}

		m_translationMode = false;
		emit translationModeChanged(false);

		updateTitle();

		m_linesWidget->setUpdatesEnabled(false);

		int oldUndoCount = m_subtitle->actionManager().undoCount();
		m_subtitle->clearSecondaryTextData();

		// The cleaning of the translations texts shouldn't be an undoable action so if
		// such action was stored by m_subtitle->clearSecondaryTextData() we remove it
		if(m_subtitle->actionManager().undoCount() != oldUndoCount)
			m_subtitle->actionManager().popUndo();

		m_linesWidget->setUpdatesEnabled(true);
	}

	return true;
}

void
Application::joinSubtitles()
{
	static JoinSubtitlesDialog *dlg = new JoinSubtitlesDialog(m_mainWindow);

	if(dlg->exec() == QDialog::Accepted) {
		QTextCodec *codec = codecForEncoding(dlg->subtitleEncoding(), false);

		Subtitle secondSubtitle;
		bool primary = dlg->selectedTextsTarget() != Subtitle::Secondary;
		if(FormatManager::instance().readSubtitle(secondSubtitle, primary, dlg->subtitleUrl(), KEncodingDetector::SemiautomaticDetection, &codec)) {
			if(dlg->selectedTextsTarget() == Subtitle::Both)
				secondSubtitle.setSecondaryData(secondSubtitle, true);

			m_subtitle->appendSubtitle(secondSubtitle, dlg->shiftTime().toMillis());
		} else
			KMessageBox::sorry(m_mainWindow, i18n("Could not read the subtitle file to append."));
	}
}

KUrl
Application::saveSplitSubtitle(const Subtitle &subtitle, const KUrl &srcUrl, QString encoding, QString format, bool primary)
{
	KUrl dstUrl;

	if(subtitle.linesCount()) {
		if(encoding.isEmpty())
			encoding = "UTF-8";

		if(format.isEmpty())
			format = "SubRip";

		QFileInfo dstFileInfo(srcUrl.path());
		if(srcUrl.isEmpty()) {
			QString baseName = primary ? i18n("Untitled") : i18n("Untitled Translation");
			QFileInfo(QDir(System::tempDir()), baseName + FormatManager::instance().defaultOutput()->extensions().first()
					  );
		}

		dstUrl = srcUrl;
		dstUrl.setPath(dstFileInfo.path());
		dstUrl = System::newUrl(dstUrl, dstFileInfo.completeBaseName() + " - " + i18nc("Suffix added to split subtitles", "split"), dstFileInfo.suffix());

		bool codecFound;
		QTextCodec *codec = KGlobal::charsets()->codecForName(encoding, codecFound);
		if(!codecFound)
			codec = KGlobal::locale()->codecForEncoding();

		bool success = FormatManager::instance().writeSubtitle(subtitle,
															   primary,
															   dstUrl,
															   codec,
															   (primary ? m_subtitleEOL : m_subtitleTrEOL),
															   format,
															   false);

		if(success)
			dstUrl.setFileEncoding(codec->name());
		else
			dstUrl = KUrl();
	}

	if(dstUrl.path().isEmpty()) {
		KMessageBox::sorry(m_mainWindow, primary ? i18n("Could not write the split subtitle file.") : i18n("Could not write the split subtitle translation file.")
						   );
	}

	return dstUrl;
}

void
Application::splitSubtitle()
{
	static SplitSubtitleDialog *dlg = new SplitSubtitleDialog(m_mainWindow);

	if(dlg->exec() != QDialog::Accepted)
		return;

	Subtitle newSubtitle;
	m_subtitle->splitSubtitle(newSubtitle, dlg->splitTime().toMillis(), dlg->shiftNewSubtitle());
	if(!newSubtitle.linesCount()) {
		KMessageBox::information(m_mainWindow, i18n("The specified time does not split the subtitles."));
		return;
	}

	KUrl splitUrl = saveSplitSubtitle(newSubtitle,
									  m_subtitleUrl.prettyUrl(),
									  m_subtitleEncoding,
									  m_subtitleFormat,
									  true);

	if(splitUrl.path().isEmpty()) {
		// there was an error saving the split part, undo the splitting of m_subtitle
		m_subtitle->actionManager().undo();
		return;
	}

	KUrl splitTrUrl;
	if(m_translationMode) {
		splitTrUrl = saveSplitSubtitle(newSubtitle, m_subtitleTrUrl, m_subtitleTrEncoding, m_subtitleTrFormat, false);

		if(splitTrUrl.path().isEmpty()) {
			// there was an error saving the split part, undo the splitting of m_subtitle
			m_subtitle->actionManager().undo();
			return;
		}
	}

	QStringList args;
	args << splitUrl.prettyUrl();
	if(m_translationMode)
		args << splitTrUrl.prettyUrl();

	if(!QProcess::startDetached(KCmdLineArgs::aboutData()->appName(), args)) {
		KMessageBox::sorry(m_mainWindow, m_translationMode ? i18n("Could not open a new Subtitle Composer window.\n" "The split part was saved as %1.", splitUrl.path()
																  ) : i18n("Could not open a new Subtitle Composer window.\n" "The split parts were saved as %1 and %2.", splitUrl.path(), splitTrUrl.path()
																		   )
						   );
	}
}

void
Application::insertBeforeCurrentLine()
{
	static InsertLineDialog *dlg = new InsertLineDialog(true, m_mainWindow);

	if(dlg->exec() == QDialog::Accepted) {
		SubtitleLine *newLine;
		{
			LinesWidgetScrollToModelDetacher detacher(*m_linesWidget);
			SubtitleLine *currentLine = m_linesWidget->currentLine();
			newLine = m_subtitle->insertNewLine(currentLine ? currentLine->index() : 0, false, dlg->selectedTextsTarget()
												);
		}
		m_linesWidget->setCurrentLine(newLine, true);
	}
}

void
Application::insertAfterCurrentLine()
{
	static InsertLineDialog *dlg = new InsertLineDialog(false, m_mainWindow);

	if(dlg->exec() == QDialog::Accepted) {
		SubtitleLine *newLine;
		{
			LinesWidgetScrollToModelDetacher detacher(*m_linesWidget);

			SubtitleLine *currentLine = m_linesWidget->currentLine();
			newLine = m_subtitle->insertNewLine(currentLine ? currentLine->index() + 1 : 0, true, dlg->selectedTextsTarget()
												);
		}
		m_linesWidget->setCurrentLine(newLine, true);
	}
}

void
Application::removeSelectedLines()
{
	static RemoveLinesDialog *dlg = new RemoveLinesDialog(m_mainWindow);

	if(dlg->exec() == QDialog::Accepted) {
		RangeList selectionRanges = m_linesWidget->selectionRanges();

		if(selectionRanges.isEmpty())
			return;

		{
			LinesWidgetScrollToModelDetacher detacher(*m_linesWidget);
			m_subtitle->removeLines(selectionRanges, dlg->selectedTextsTarget());
		}

		int firstIndex = selectionRanges.firstIndex();
		if(firstIndex < m_subtitle->linesCount())
			m_linesWidget->setCurrentLine(m_subtitle->line(firstIndex), true);
		else if(firstIndex - 1 < m_subtitle->linesCount())
			m_linesWidget->setCurrentLine(m_subtitle->line(firstIndex - 1), true);
	}
}

void
Application::joinSelectedLines()
{
	const RangeList &ranges = m_linesWidget->selectionRanges();

//  if ( ranges.count() > 1 && KMessageBox::Continue != KMessageBox::warningContinueCancel(
//      m_mainWindow,
//      i18n( "Current selection has multiple ranges.\nContinuing will join them all." ),
//      i18n( "Join Lines" ) ) )
//      return;

	m_subtitle->joinLines(ranges);
}

void
Application::splitSelectedLines()
{
	m_subtitle->splitLines(m_linesWidget->selectionRanges());
}

void
Application::selectAllLines()
{
	m_linesWidget->selectAll();
}

void
Application::gotoLine()
{
	IntInputDialog gotoLineDlg(i18n("Go to Line"), i18n("&Go to line:"), 1, m_subtitle->linesCount(), m_linesWidget->currentLineIndex() + 1);

	if(gotoLineDlg.exec() == QDialog::Accepted)
		m_linesWidget->setCurrentLine(m_subtitle->line(gotoLineDlg.value() - 1), true);
}

void
Application::find()
{
	m_lastFoundLine = 0;
	m_finder->find(m_linesWidget->selectionRanges(), m_linesWidget->currentLineIndex(), m_curLineWidget->focusedText(), false);
}

void
Application::findNext()
{
	if(!m_finder->findNext()) {
		m_lastFoundLine = 0;
		m_finder->find(m_linesWidget->selectionRanges(), m_linesWidget->currentLineIndex(), m_curLineWidget->focusedText(), false);
	}
}

void
Application::findPrevious()
{
	if(!m_finder->findPrevious()) {
		m_lastFoundLine = 0;
		m_finder->find(m_linesWidget->selectionRanges(), m_linesWidget->currentLineIndex(), m_curLineWidget->focusedText(), true);
	}
}

void
Application::replace()
{
	m_replacer->replace(m_linesWidget->selectionRanges(), m_linesWidget->currentLineIndex(), m_curLineWidget->focusedText()
						);
}

void
Application::spellCheck()
{
	m_speller->spellCheck(m_linesWidget->currentLineIndex());
}

void
Application::findError()
{
	m_lastFoundLine = 0;
	m_errorFinder->find(m_linesWidget->selectionRanges(), m_linesWidget->currentLineIndex(), false);
}

void
Application::findNextError()
{
	if(!m_errorFinder->findNext()) {
		m_lastFoundLine = 0;
		m_errorFinder->find(m_linesWidget->selectionRanges(), m_linesWidget->currentLineIndex(), false);
	}
}

void
Application::findPreviousError()
{
	if(!m_errorFinder->findPrevious()) {
		m_lastFoundLine = 0;
		m_errorFinder->find(m_linesWidget->selectionRanges(), m_linesWidget->currentLineIndex(), true);
	}
}

void
Application::retrocedeCurrentLine()
{
	SubtitleLine *currentLine = m_linesWidget->currentLine();
	if(currentLine && currentLine->prevLine())
		m_linesWidget->setCurrentLine(currentLine->prevLine(), true);
}

void
Application::advanceCurrentLine()
{
	SubtitleLine *currentLine = m_linesWidget->currentLine();
	if(currentLine && currentLine->nextLine())
		m_linesWidget->setCurrentLine(currentLine->nextLine(), true);
}

void
Application::checkErrors()
{
	static CheckErrorsDialog *dlg = new CheckErrorsDialog(m_mainWindow);

	if(dlg->exec() == QDialog::Accepted) {
		SubtitleCompositeActionExecutor executor(*m_subtitle, i18n("Check Lines Errors"));

		RangeList targetRanges(m_linesWidget->targetRanges(dlg->selectedLinesTarget()));

		if(dlg->clearOtherErrors()) {
			int flagsToClear = SubtitleLine::AllErrors & (~dlg->selectedErrorFlags() & ~SubtitleLine::UserMark);
			m_subtitle->clearErrors(targetRanges, flagsToClear);
		}

		if(dlg->clearMarks())
			m_subtitle->setMarked(targetRanges, false);

		m_subtitle->checkErrors(targetRanges, dlg->selectedErrorFlags(), errorsConfig()->minDuration(), errorsConfig()->maxDuration(), errorsConfig()->minDurationPerChar(), errorsConfig()->maxDurationPerChar(), errorsConfig()->maxCharacters(), errorsConfig()->maxLines()
								);
	}
}

void
Application::recheckAllErrors()
{
	m_subtitle->recheckErrors(Range::full(), errorsConfig()->minDuration(), errorsConfig()->maxDuration(), errorsConfig()->minDurationPerChar(), errorsConfig()->maxDurationPerChar(), errorsConfig()->maxCharacters(), errorsConfig()->maxLines()
							  );
}

void
Application::recheckSelectedErrors()
{
	// NOTE we can't just use Subtitle::recheckErrors() with the selected lines ranges
	// because this slots handles the error dialog action where the user can not only
	// select lines but can also select (or unselect) specific errors

	SubtitleCompositeActionExecutor executor(*m_subtitle, i18n("Check Lines Errors"));

	for(SubtitleIterator it(*m_subtitle, m_errorsWidget->selectionRanges()); it.current(); ++it) {
		it.current()->check(m_errorsWidget->lineSelectedErrorFlags(it.current()->index()), errorsConfig()->minDuration(), errorsConfig()->maxDuration(), errorsConfig()->minDurationPerChar(), errorsConfig()->maxDurationPerChar(), errorsConfig()->maxCharacters(), errorsConfig()->maxLines()
							);
	}
}

void
Application::clearErrors()
{
	static ClearErrorsDialog *dlg = new ClearErrorsDialog(m_mainWindow);

	if(dlg->exec() == QDialog::Accepted) {
		SubtitleCompositeActionExecutor executor(*m_subtitle, i18n("Clear Lines Errors"));

		RangeList targetRanges(m_linesWidget->targetRanges(dlg->selectedLinesTarget()));

		m_subtitle->clearErrors(targetRanges, dlg->selectedErrorFlags());
	}
}

void
Application::clearSelectedErrors(bool includeMarks)
{
	SubtitleCompositeActionExecutor executor(*m_subtitle, i18n("Clear Lines Errors"));

	for(SubtitleIterator it(*m_subtitle, m_errorsWidget->selectionRanges()); it.current(); ++it) {
		SubtitleLine *line = it.current();
		int errorFlags = m_errorsWidget->lineSelectedErrorFlags(line->index());

		if(!includeMarks)
			errorFlags = errorFlags & ~SubtitleLine::UserMark;

		line->setErrorFlags(errorFlags, false);
	}
}

void
Application::clearSelectedMarks()
{
	m_subtitle->setMarked(m_errorsWidget->selectionRanges(), false);
}

void
Application::showErrors()
{
	if(m_errorsDialog->isHidden())
		m_errorsDialog->show();
}

void
Application::showErrorsConfig()
{
	m_configDialog->setCurrentPage(ConfigDialog::Errors);
	m_configDialog->show();
}

void
Application::toggleSelectedLinesMark()
{
	m_subtitle->toggleMarked(m_linesWidget->selectionRanges());
}

void
Application::toggleSelectedLinesBold()
{
	m_subtitle->toggleStyleFlag(m_linesWidget->selectionRanges(), SString::Bold);
}

void
Application::toggleSelectedLinesItalic()
{
	m_subtitle->toggleStyleFlag(m_linesWidget->selectionRanges(), SString::Italic);
}

void
Application::toggleSelectedLinesUnderline()
{
	m_subtitle->toggleStyleFlag(m_linesWidget->selectionRanges(), SString::Underline);
}

void
Application::toggleSelectedLinesStrikeThrough()
{
	m_subtitle->toggleStyleFlag(m_linesWidget->selectionRanges(), SString::StrikeThrough);
}

void
Application::changeSelectedLinesColor()
{
	const RangeList range = m_linesWidget->selectionRanges();
	SubtitleIterator it(*m_subtitle, range);
	if(!it.current())
		return;

	QColor color = SubtitleColorDialog::getColor(QColor(it.current()->primaryText().styleColorAt(0)), m_mainWindow);
	if(color.isValid())
		m_subtitle->changeTextColor(range, color.rgba());
}

void
Application::shiftLines()
{
	static ShiftTimesDialog *dlg = new ShiftTimesDialog(m_mainWindow);

	dlg->resetShiftTime();

	if(dlg->exec() == QDialog::Accepted) {
		m_subtitle->shiftLines(m_linesWidget->targetRanges(dlg->selectedLinesTarget()), dlg->shiftTimeMillis());
	}
}

void
Application::shiftSelectedLinesForwards()
{
	m_subtitle->shiftLines(m_linesWidget->selectionRanges(), generalConfig()->linesQuickShiftAmount());
}

void
Application::shiftSelectedLinesBackwards()
{
	m_subtitle->shiftLines(m_linesWidget->selectionRanges(), -generalConfig()->linesQuickShiftAmount());
}

void
Application::adjustLines()
{
	static AdjustTimesDialog *dlg = new AdjustTimesDialog(m_mainWindow);

	dlg->setFirstLineTime(m_subtitle->firstLine()->showTime());
	dlg->setLastLineTime(m_subtitle->lastLine()->showTime());

	if(dlg->exec() == QDialog::Accepted) {
		m_subtitle->adjustLines(Range::full(), dlg->firstLineTime().toMillis(), dlg->lastLineTime().toMillis());
	}
}

void
Application::sortLines()
{
	static ActionWithLinesTargetDialog *dlg = new ActionWithLinesTargetDialog(i18n("Sort"), m_mainWindow);

	PROFILE();

	dlg->setLinesTargetEnabled(ActionWithTargetDialog::Selection, true);

	if(m_linesWidget->selectionHasMultipleRanges()) {
		if(m_linesWidget->showingContextMenu()) {
			KMessageBox::sorry(m_mainWindow, i18n("Can not perform sort on selection with multiple ranges."));
			return;
		} else {
			dlg->setLinesTargetEnabled(ActionWithTargetDialog::Selection, false);
			dlg->setSelectedLinesTarget(ActionWithTargetDialog::AllLines);
		}
	}

	if(dlg->exec() == QDialog::Accepted) {
		RangeList targetRanges(m_linesWidget->targetRanges(dlg->selectedLinesTarget()));
		if(!targetRanges.isEmpty()) {
			m_subtitle->sortLines(*targetRanges.begin());
		}
	}
}

void
Application::changeFrameRate()
{
	static ChangeFrameRateDialog *dlg = new ChangeFrameRateDialog(m_subtitle->framesPerSecond(), m_mainWindow);

	dlg->setFromFramesPerSecond(m_subtitle->framesPerSecond());

	if(dlg->exec() == QDialog::Accepted) {
		m_subtitle->changeFramesPerSecond(dlg->toFramesPerSecond(), dlg->fromFramesPerSecond());
	}
}

void
Application::enforceDurationLimits()
{
	static DurationLimitsDialog *dlg = new DurationLimitsDialog(errorsConfig()->minDuration(),
																errorsConfig()->maxDuration(),
																m_mainWindow);

	if(dlg->exec() == QDialog::Accepted) {
		m_subtitle->applyDurationLimits(m_linesWidget->targetRanges(dlg->selectedLinesTarget()), dlg->enforceMinDuration() ? dlg->minDuration() : 0, dlg->enforceMaxDuration() ? dlg->maxDuration() : Time::MaxMseconds, !dlg->preventOverlap());
	}
}

void
Application::setAutoDurations()
{
	static AutoDurationsDialog *dlg = new AutoDurationsDialog(60, 50, 50, m_mainWindow);

	if(dlg->exec() == QDialog::Accepted) {
		m_subtitle->setAutoDurations(m_linesWidget->targetRanges(dlg->selectedLinesTarget()), dlg->charMillis(), dlg->wordMillis(), dlg->lineMillis(), !dlg->preventOverlap(), dlg->calculationMode()
									 );
	}
}

void
Application::maximizeDurations()
{
	static ActionWithLinesTargetDialog *dlg = new ActionWithLinesTargetDialog(i18n("Maximize Durations"),
																			  m_mainWindow);

	if(dlg->exec() == QDialog::Accepted)
		m_subtitle->setMaximumDurations(m_linesWidget->targetRanges(dlg->selectedLinesTarget()));
}

void
Application::fixOverlappingLines()
{
	static FixOverlappingTimesDialog *dlg = new FixOverlappingTimesDialog(m_mainWindow);

	if(dlg->exec() == QDialog::Accepted)
		m_subtitle->fixOverlappingLines(m_linesWidget->targetRanges(dlg->selectedLinesTarget()), dlg->minimumInterval());
}

void
Application::syncWithSubtitle()
{
	static SyncSubtitlesDialog *dlg = new SyncSubtitlesDialog(m_mainWindow);

	if(dlg->exec() == QDialog::Accepted) {
		QTextCodec *codec = codecForEncoding(dlg->subtitleEncoding(), false);

		Subtitle referenceSubtitle;
		if(FormatManager::instance().readSubtitle(referenceSubtitle, true, dlg->subtitleUrl(), KEncodingDetector::SemiautomaticDetection, &codec)) {
			if(dlg->adjustToReferenceSubtitle()) {
				if(referenceSubtitle.linesCount() <= 1)
					KMessageBox::sorry(m_mainWindow, i18n("The reference subtitle must have more than one line to proceed."));
				else
					m_subtitle->adjustLines(Range::full(), referenceSubtitle.firstLine()->showTime().toMillis(), referenceSubtitle.lastLine()->showTime().toMillis()
											);
			} else // if ( dlg->synchronizeToReferenceTimes() )
				m_subtitle->syncWithSubtitle(referenceSubtitle);
		} else
			KMessageBox::sorry(m_mainWindow, i18n("Could not parse the reference subtitle file."));
	}
}

void
Application::breakLines()
{
	static SmartTextsAdjustDialog *dlg = new SmartTextsAdjustDialog(30, m_mainWindow);

	if(dlg->exec() == QDialog::Accepted)
		m_subtitle->breakLines(m_linesWidget->targetRanges(dlg->selectedLinesTarget()), dlg->minLengthForLineBreak(), dlg->selectedTextsTarget()
							   );
}

void
Application::unbreakTexts()
{
	static ActionWithLinesAndTextsTargetDialog *dlg = new ActionWithLinesAndTextsTargetDialog(i18n("Unbreak Lines"),
																							  m_mainWindow);

	if(dlg->exec() == QDialog::Accepted)
		m_subtitle->unbreakTexts(m_linesWidget->targetRanges(dlg->selectedLinesTarget()), dlg->selectedTextsTarget()
								 );
}

void
Application::simplifySpaces()
{
	static ActionWithLinesAndTextsTargetDialog *dlg = new ActionWithLinesAndTextsTargetDialog(i18n("Simplify Spaces"),
																							  m_mainWindow);

	if(dlg->exec() == QDialog::Accepted)
		m_subtitle->simplifyTextWhiteSpace(m_linesWidget->targetRanges(dlg->selectedLinesTarget()), dlg->selectedTextsTarget()
										   );
}

void
Application::changeCase()
{
	static ChangeTextsCaseDialog *dlg = new ChangeTextsCaseDialog(m_mainWindow);

	if(dlg->exec() == QDialog::Accepted) {
		switch(dlg->caseOperation()) {
		case ChangeTextsCaseDialog::Upper:
			m_subtitle->upperCase(m_linesWidget->targetRanges(dlg->selectedLinesTarget()), dlg->selectedTextsTarget()
								  );
			break;
		case ChangeTextsCaseDialog::Lower:
			m_subtitle->lowerCase(m_linesWidget->targetRanges(dlg->selectedLinesTarget()), dlg->selectedTextsTarget()
								  );
			break;
		case ChangeTextsCaseDialog::Title:
			m_subtitle->titleCase(m_linesWidget->targetRanges(dlg->selectedLinesTarget()), dlg->lowerFirst(), dlg->selectedTextsTarget()
								  );
			break;
		case ChangeTextsCaseDialog::Sentence:
			m_subtitle->sentenceCase(m_linesWidget->targetRanges(dlg->selectedLinesTarget()), dlg->lowerFirst(), dlg->selectedTextsTarget()
									 );
			break;
		}
	}
}

void
Application::fixPunctuation()
{
	static FixPunctuationDialog *dlg = new FixPunctuationDialog(m_mainWindow);

	if(dlg->exec() == QDialog::Accepted) {
		m_subtitle->fixPunctuation(m_linesWidget->targetRanges(dlg->selectedLinesTarget()), dlg->spaces(), dlg->quotes(), dlg->englishI(), dlg->ellipisis(), dlg->selectedTextsTarget()
								   );
	}
}

bool
Application::applyTranslation(RangeList ranges, bool primary, int inputLanguage, int outputLanguage, int textTargets)
{
	Translator translator;

	QString inputLanguageName = Language::name((Language::Value)inputLanguage);
	QString outputLanguageName = Language::name((Language::Value)outputLanguage);

	ProgressDialog progressDialog(i18n("Translate"), i18n("Translating text (%1 to %2)...", inputLanguageName, outputLanguageName), true, m_mainWindow);

	if(textTargets == SubtitleLine::Both) {
		progressDialog.setDescription(primary
									  ? i18n("Translating primary text (%1 to %2)...", inputLanguageName, outputLanguageName)
									  : i18n("Translating secondary text (%1 to %2)...", inputLanguageName, outputLanguageName));
	}

	QString inputText;
	QRegExp dialogCueRegExp2("-([^-])");
	for(SubtitleIterator it(*m_subtitle, ranges); it.current(); ++it) {
		QString lineText = it.current()->primaryText().richString();
		lineText.replace('\n', ' ').replace("--", "---").replace(dialogCueRegExp2, "- \\1");
		inputText += lineText + "\n()() ";
	}

	translator.syncTranslate(inputText, (Language::Value)inputLanguage, (Language::Value)outputLanguage, &progressDialog);

	if(translator.isAborted())
		return false; // ended with error

	QStringList outputLines;
	QString errorMessage;

	if(translator.isFinishedWithError()) {
		errorMessage = translator.errorMessage();
	} else {
		outputLines = translator.outputText().split(QRegExp("\\s*\n\\(\\) ?\\(\\)\\s*"));

//		qDebug() << translator.inputText();
//		qDebug() << translator.outputText();

		if(outputLines.count() != ranges.indexesCount() + 1)
			errorMessage = i18n("Unable to perform texts synchronization (sent and received lines count do not match).");
	}

	if(errorMessage.isEmpty()) {
		SubtitleCompositeActionExecutor executor(*m_subtitle, primary ? i18n("Translate Primary Text") : i18n("Translate Secondary Text"));

		int index = -1;
		QRegExp ellipsisRegExp("\\s+\\.\\.\\.");
		QRegExp dialogCueRegExp("(^| )- ");
		for(SubtitleIterator it(*m_subtitle, ranges); it.current(); ++it) {
			QString line = outputLines.at(++index);
			line.replace(" ---", "--");
			line.replace(ellipsisRegExp, "...");
			line.replace(dialogCueRegExp, "\n-");
			SString text;
			text.setRichString(line);
			it.current()->setPrimaryText(text.trimmed());
		}
	} else {
		KMessageBox::sorry(m_mainWindow, i18n("There was an error performing the translation:\n\n%1", errorMessage));
	}

	return errorMessage.isEmpty();
}

void
Application::translate()
{
	static TranslateDialog *dlg = new TranslateDialog(m_mainWindow);

	if(dlg->exec() == QDialog::Accepted) {
		if(dlg->selectedTextsTarget() == Subtitle::Primary || dlg->selectedTextsTarget() == Subtitle::Both) {
			if(!applyTranslation(m_linesWidget->targetRanges(dlg->selectedLinesTarget()), true, dlg->inputLanguage(), dlg->outputLanguage(), dlg->selectedTextsTarget()))
				return;
		}

		if(dlg->selectedTextsTarget() == Subtitle::Secondary || dlg->selectedTextsTarget() == Subtitle::Both) {
			if(!applyTranslation(m_linesWidget->targetRanges(dlg->selectedLinesTarget()), false, dlg->inputLanguage(), dlg->outputLanguage(), dlg->selectedTextsTarget()))
				return;
		}
	}
}

void
Application::openVideo(const KUrl &url)
{
	if(url.protocol() != "file")
		return;

	m_player->closeFile();

	m_player->openFile(url.path());
}

void
Application::openVideo()
{
	KFileDialog openDlg(m_lastVideoUrl, buildMediaFilesFilter(), m_mainWindow);

	openDlg.setModal(true);
	openDlg.setCaption(i18n("Open Video"));

	if(openDlg.exec() == QDialog::Accepted) {
		m_lastVideoUrl = openDlg.selectedUrl();
		openVideo(openDlg.selectedUrl());
	}
}

void
Application::toggleFullScreenMode()
{
	setFullScreenMode(!m_playerWidget->fullScreenMode());
}

void
Application::setFullScreenMode(bool enabled)
{
	if(enabled != m_playerWidget->fullScreenMode()) {
		m_playerWidget->setFullScreenMode(enabled);

		KToggleAction *toggleFullScreenAction = (KToggleAction *)action(ACT_TOGGLE_FULL_SCREEN);
		toggleFullScreenAction->setChecked(enabled);

		emit fullScreenModeChanged(enabled);
	}
}

void
Application::seekBackwards()
{
	double position = m_player->position() - playerConfig()->seekJumpLength();
	m_player->seek(position > 0.0 ? position : 0.0, false);
}

void
Application::seekForwards()
{
	double position = m_player->position() + playerConfig()->seekJumpLength();
	m_player->seek(position <= m_player->length() ? position : m_player->length(), false);
}

void
Application::seekToPrevLine()
{
	SubtitleLine *overlayLine = m_playerWidget->overlayLine();
	if(overlayLine) {
		SubtitleLine *prevLine = overlayLine->prevLine();
		if(prevLine) {
			m_player->seek((prevLine->showTime().toMillis()) / 1000.0, true);
			m_linesWidget->setCurrentLine(prevLine);
			m_curLineWidget->setCurrentLine(prevLine);
		}
	}
}

void
Application::seekToNextLine()
{
	SubtitleLine *overlayLine = m_playerWidget->overlayLine();
	if(overlayLine) {
		SubtitleLine *nextLine = overlayLine->nextLine();
		if(nextLine) {
			m_player->seek((nextLine->showTime().toMillis()) / 1000.0, true);
			m_linesWidget->setCurrentLine(nextLine);
			m_curLineWidget->setCurrentLine(nextLine);
		}
	}
}

void
Application::setCurrentLineShowTimeFromVideo()
{
	SubtitleLine *currentLine = m_linesWidget->currentLine();
	if(currentLine)
		currentLine->setShowTime(videoPosition(true));
}

void
Application::setCurrentLineHideTimeFromVideo()
{
	SubtitleLine *currentLine = m_linesWidget->currentLine();
	if(currentLine) {
		currentLine->setHideTime(videoPosition(true));
		SubtitleLine *nextLine = currentLine->nextLine();
		if(nextLine)
			m_linesWidget->setCurrentLine(nextLine, true);
	}
}

void
Application::setActiveSubtitleStream(int subtitleStream)
{
	KSelectAction *activeSubtitleStreamAction = (KSelectAction *)action(ACT_SET_ACTIVE_SUBTITLE_STREAM);
	activeSubtitleStreamAction->setCurrentItem(subtitleStream);

	m_playerWidget->setShowTranslation(subtitleStream ? true : false);
}

void
Application::shiftToVideoPosition()
{
	SubtitleLine *currentLine = m_linesWidget->currentLine();
	if(currentLine) {
		m_subtitle->shiftLines(Range::full(), videoPosition(true).toMillis() - currentLine->showTime().toMillis()
							   );
	}
}

void
Application::adjustToVideoPositionAnchorLast()
{
	SubtitleLine *currentLine = m_linesWidget->currentLine();
	if(currentLine) {
		long lastLineTime = m_subtitle->lastLine()->showTime().toMillis();

		long oldCurrentLineTime = currentLine->showTime().toMillis();
		long oldDeltaTime = lastLineTime - oldCurrentLineTime;

		if(!oldDeltaTime)
			return;

		long newCurrentLineTime = videoPosition(true).toMillis();
		long newDeltaTime = lastLineTime - newCurrentLineTime;

		double scaleFactor = (double)newDeltaTime / oldDeltaTime;
		long shiftTime = (long)(newCurrentLineTime - scaleFactor * oldCurrentLineTime);

		long newFirstLineTime = (long)(shiftTime + m_subtitle->firstLine()->showTime().toMillis() * scaleFactor);

		if(newFirstLineTime < 0) {
			if(KMessageBox::warningContinueCancel(m_mainWindow, i18n("Continuing would result in loss of timing information for some lines.\nAre you sure you want to continue?")
												  ) != KMessageBox::Continue)
				return;
		}

		m_subtitle->adjustLines(Range::full(), newFirstLineTime, lastLineTime);
	}
}

void
Application::extractVideoAudio()
{
	if(m_decoder->filePath() != m_player->filePath()) {
		m_decoder->closeFile();

		QxtSignalWaiter openedWaiter(Decoder::instance(), SIGNAL(fileOpened(const QString &)), SIGNAL(fileOpenError(const QString &)));
		m_decoder->openFile(m_player->filePath());
		openedWaiter.wait(10000);

		if(m_decoder->filePath().isEmpty()) {
			KMessageBox::sorry(m_mainWindow, i18n("<qt>There was an error opening file %1 for audio extraction.</qt>", m_player->filePath()));
			return;
		}
	}

	WaveFormat outputFormat = m_decoder->audioStreamFormat(m_player->activeAudioStream());
	outputFormat.setBitsPerSample(8);
	outputFormat.setChannels(1);
	outputFormat.setInteger(true);
	while(outputFormat.sampleRate() >= 5000 && outputFormat.sampleRate() % 2 == 0)
		outputFormat.setSampleRate(outputFormat.sampleRate() / 2);

	QFileInfo fileInfo(m_player->filePath());
	QString fileBaseName = fileInfo.path() + "/" + fileInfo.completeBaseName();
	fileInfo.setFile(fileBaseName + QString("-stream%1.wav").arg(m_player->activeAudioStream() + 1));
	int count = 1;
	while(fileInfo.exists())
		fileInfo.setFile(fileBaseName + QString("-stream%1(%2).wav").arg(m_player->activeAudioStream() + 1).arg(++count));

	m_decoder->decode(m_player->activeAudioStream(), fileInfo.filePath(), outputFormat);
}

void
Application::onDecodingError(const QString &errorMessage)
{
	if(errorMessage.isEmpty())
		KMessageBox::error(m_mainWindow, i18n("Unexpected error when extracting audio."), i18n("Error Extracting Audio")
						   );
	else
		KMessageBox::detailedError(m_mainWindow, i18n("Unexpected error when extracting audio."), errorMessage, i18n("Error Extracting Audio")
								   );
	m_decoder->closeFile();
}

void
Application::adjustToVideoPositionAnchorFirst()
{
	SubtitleLine *currentLine = m_linesWidget->currentLine();
	if(currentLine) {
		long firstLineTime = m_subtitle->firstLine()->showTime().toMillis();

		long oldCurrentLineTime = currentLine->showTime().toMillis();
		long oldDeltaTime = oldCurrentLineTime - firstLineTime;

		if(!oldDeltaTime)
			return;

		long newCurrentLineTime = videoPosition(true).toMillis();
		long newDeltaTime = newCurrentLineTime - firstLineTime;

		double scaleFactor = (double)newDeltaTime / oldDeltaTime;
		long shiftTime = (long)(firstLineTime - scaleFactor * firstLineTime);

		long newLastLineTime = (long)(shiftTime + m_subtitle->lastLine()->showTime().toMillis() * scaleFactor);

		if(newLastLineTime > Time::MaxMseconds) {
			if(KMessageBox::warningContinueCancel(m_mainWindow, i18n("Continuing would result in loss of timing information for some lines.\nAre you sure you want to continue?")
												  ) != KMessageBox::Continue)
				return;
		}

		m_subtitle->adjustLines(Range::full(), firstLineTime, newLastLineTime);
	}
}

void
Application::openAudioLevels()
{
//  KFileDialog openDlg( m_lastAudioLevelsUrl, buildLevelsFilesFilter(), m_mainWindow );
//
//  openDlg.setModal( true );
//  openDlg.setCaption( i18n( "Open AudioLevels" ) );
//
//  if ( openDlg.exec() == QDialog::Accepted )
//  {
//      m_lastAudioLevelsUrl = openDlg.selectedUrl();
//      openAudioLevels( openDlg.selectedUrl() );
//  }
}

void
Application::openAudioLevels(const KUrl & /*url */)
{
// FIXME audio levels
/*
		closeAudioLevels();

		m_audiolevels = new AudioLevels();
   //   if ( m_audiolevels->loadFromMedia( url.path(), true ) )
		if ( m_audiolevels->load( url ) )
		{
				m_recentAudioLevelsAction->addUrl( url );

				emit audiolevelsOpened( m_audiolevels );
		}
		else
		{
				delete m_audiolevels;
				m_audiolevels = 0;

				KMessageBox::sorry( m_mainWindow, i18n( "There was an error opening the audiolevels." ) );
		}
 */
}

void
Application::saveAudioLevelsAs()
{
// FIXME audio levels
/*
		KFileDialog saveDlg( m_lastAudioLevelsUrl, QString(), m_mainWindow );
		saveDlg.setModal( true );
		saveDlg.setCaption( i18n( "Save AudioLevels" ) );
		saveDlg.setOperationMode( KFileDialog::Saving );
		saveDlg.setMode( KFile::File );

		if ( saveDlg.exec() == QDialog::Accepted )
		{
				KUrl selectedUrl = saveDlg.selectedUrl();

				if ( FileSaveHelper::exists( selectedUrl ) )
				{
						if ( KMessageBox::warningContinueCancel(
										m_mainWindow,
										i18n(
												"A file named \"%1\" already exists. Are you sure you want to overwrite it?",
												QFileInfo( selectedUrl.path() ).fileName()
										),
										i18n( "Overwrite File?" ),
										KGuiItem( i18n( "Overwrite" ) )
								) != KMessageBox::Continue )
								return;
				}

				if ( ! m_audiolevels->save( selectedUrl, true ) )
						KMessageBox::sorry( m_mainWindow, i18n( "There was an error saving the audiolevels." ) );
		}
 */
}

void
Application::closeAudioLevels()
{
// FIXME audio levels
/*
		delete m_audiolevels;
		m_audiolevels = 0;

		emit audiolevelsClosed();
 */
}

void
Application::increaseAudioLevelsVZoom()
{}

void
Application::decreaseAudioLevelsVZoom()
{}

void
Application::increaseAudioLevelsHZoom()
{}

void
Application::decreaseAudioLevelsHZoom()
{}

/// END ACTION HANDLERS

void
Application::updateTitle()
{
	if(m_subtitle) {
		if(m_translationMode) {
			static const QString titleBuilder("%1%2 | %3%4");
			static const QString modified = QString::fromUtf8(" [") + i18n("modified") + QString::fromUtf8("]");

			m_mainWindow->setCaption(titleBuilder.arg(m_subtitleUrl.isEmpty() ? i18n("Untitled") : m_subtitleFileName)
									  .arg(m_subtitle->isPrimaryDirty() ? modified : QString())
									  .arg(m_subtitleTrUrl.isEmpty() ? i18n("Untitled Translation") : m_subtitleTrFileName)
									  .arg(m_subtitle->isSecondaryDirty() ? modified : QString()), false);
		} else {
			m_mainWindow->setCaption(m_subtitleUrl.isEmpty() ? i18n("Untitled") : (m_subtitleUrl.isLocalFile() ? m_subtitleUrl.path() : m_subtitleUrl.prettyUrl()), m_subtitle->isPrimaryDirty()
									 );
		}
	} else
		m_mainWindow->setCaption(QString());
}

void
Application::updateUndoRedoToolTips()
{
	static QAction *undoAction = action(ACT_UNDO);
	static QAction *redoAction = action(ACT_REDO);

	static const QString undoToolTip = undoAction->toolTip();
	static const QString redoToolTip = redoAction->toolTip();

	if(m_subtitle) {
		if(m_subtitle->actionManager().hasUndo())
			undoAction->setToolTip(undoToolTip + ": " + m_subtitle->actionManager().undoDescription());
		else
			undoAction->setToolTip(undoToolTip);

		if(m_subtitle->actionManager().hasRedo())
			redoAction->setToolTip(redoToolTip + ": " + m_subtitle->actionManager().redoDescription());
		else
			redoAction->setToolTip(redoToolTip);
	}
}

void
Application::onLineDoubleClicked(SubtitleLine *line)
{
	if(m_player->state() == Player::Ready)
		m_player->play();

	int mseconds = line->showTime().toMillis() - generalConfig()->seekOffsetOnDoubleClick();
	m_player->seek(mseconds > 0 ? mseconds / 1000.0 : 0.0, true);

	if(m_player->state() == Player::Paused)
		m_player->play();
}

void
Application::onHighlightLine(SubtitleLine *line, bool primary, int firstIndex, int lastIndex)
{
	if(m_playerWidget->fullScreenMode()) {
		if(m_lastFoundLine != line) {
			m_lastFoundLine = line;

			m_player->seek(line->showTime().toMillis() / 1000.0, true);
		}
	} else {
		m_linesWidget->setCurrentLine(line, true);

		if(firstIndex >= 0 && lastIndex >= 0) {
			if(primary)
				m_curLineWidget->highlightPrimary(firstIndex, lastIndex);
			else
				m_curLineWidget->highlightSecondary(firstIndex, lastIndex);
		}
	}
}

void
Application::onPlayingLineChanged(SubtitleLine *line)
{
	m_linesWidget->setPlayingLine(line);

	if(m_linkCurrentLineToPosition)
		m_linesWidget->setCurrentLine(line, true);
}

void
Application::onLinkCurrentLineToVideoToggled(bool value)
{
	if(m_linkCurrentLineToPosition != value) {
		m_linkCurrentLineToPosition = value;

		if(m_linkCurrentLineToPosition)
			m_linesWidget->setCurrentLine(m_playerWidget->playingLine(), true);
	}
}

void
Application::onPlayerFileOpened(const QString &filePath)
{
	m_recentVideosAction->addUrl(KUrl(filePath));
}

void
Application::onPlayerPlaying()
{
	QAction *playPauseAction = action(ACT_PLAY_PAUSE);
	playPauseAction->setIcon(KIcon("media-playback-pause"));
	playPauseAction->setText(i18n("Pause"));
}

void
Application::onPlayerPaused()
{
	QAction *playPauseAction = action(ACT_PLAY_PAUSE);
	playPauseAction->setIcon(KIcon("media-playback-start"));
	playPauseAction->setText(i18n("Play"));
}

void
Application::onPlayerStopped()
{
	QAction *playPauseAction = action(ACT_PLAY_PAUSE);
	playPauseAction->setIcon(KIcon("media-playback-start"));
	playPauseAction->setText(i18n("Play"));
}

void
Application::onPlayerAudioStreamsChanged(const QStringList &audioStreams)
{
	KSelectAction *activeAudioStreamAction = (KSelectAction *)action(ACT_SET_ACTIVE_AUDIO_STREAM);
	activeAudioStreamAction->setItems(audioStreams);
	action(ACT_EXTRACT_VIDEO_AUDIO)->setEnabled(!audioStreams.isEmpty() && !m_decoder->isActiveBackendDummy());
}

void
Application::onPlayerActiveAudioStreamChanged(int audioStream)
{
	KSelectAction *activeAudioStreamAction = (KSelectAction *)action(ACT_SET_ACTIVE_AUDIO_STREAM);
	if(audioStream >= 0)
		activeAudioStreamAction->setCurrentItem(audioStream);
}

void
Application::onPlayerMuteChanged(bool muted)
{
	KToggleAction *toggleMutedAction = (KToggleAction *)action(ACT_TOGGLE_MUTED);
	toggleMutedAction->setChecked(muted);
}

void
Application::onPlayerOptionChanged(const QString &option, const QString &value)
{
	if(option == PlayerConfig::keyDecoderBackend()) {
		m_decoder->reinitialize(value);
	} else if(option == PlayerConfig::keySeekJumpLength()) {
		action(ACT_SEEK_BACKWARDS)->setStatusTip(i18np("Seek backwards 1 second", "Seek backwards %1 seconds", value.toInt()));
		action(ACT_SEEK_FORWARDS)->setStatusTip(i18np("Seek forwards 1 second", "Seek forwards %1 seconds", value.toInt()));
	}
}

void
Application::onGeneralOptionChanged(const QString &option, const QString &value)
{
	if(option == GeneralConfig::keyLinesQuickShiftAmount()) {
		int shiftTimeMillis = value.toInt();

		QAction *shiftSelectedLinesFwdAction = action(ACT_SHIFT_SELECTED_LINES_FORWARDS);
		shiftSelectedLinesFwdAction->setText(i18np("Shift %21 Millisecond", "Shift %2%1 Milliseconds", shiftTimeMillis, "+"));
		shiftSelectedLinesFwdAction->setStatusTip(i18np("Shift selected lines %21 millisecond", "Shift selected lines %2%1 milliseconds", shiftTimeMillis, "+"));

		QAction *shiftSelectedLinesBwdAction = action(ACT_SHIFT_SELECTED_LINES_BACKWARDS);
		shiftSelectedLinesBwdAction->setText(i18np("Shift %21 Millisecond", "Shift %2%1 Milliseconds", shiftTimeMillis, "-"));
		shiftSelectedLinesBwdAction->setStatusTip(i18np("Shift selected lines %21 millisecond", "Shift selected lines -%2%1 milliseconds", shiftTimeMillis, "-"));
	}
}

void
Application::updateConfigFromDialog()
{
	m_config = m_configDialog->config();

	// We have to manually update the player backends configurations because they have their own copies
	QStringList backendNames(m_player->backendNames());
	for(QStringList::ConstIterator it = backendNames.begin(), end = backendNames.end(); it != end; ++it)
		m_player->backend(*it)->setConfig(m_config.group(*it));
}

#include "application.moc"
