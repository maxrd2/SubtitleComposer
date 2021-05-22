#include "application.h"

#include "actions/kcodecactionext.h"
#include "actions/krecentfilesactionext.h"
#include "actions/useraction.h"
#include "actions/useractionnames.h"
#include "core/undo/undostack.h"
#include "gui/playerwidget.h"
#include "gui/waveform/waveformwidget.h"
#include "scripting/scriptsmanager.h"
#include "videoplayer/videoplayer.h"

#include <QAction>
#include <QMenu>
#include <QTextCodec>

#include <KCharsets>
#include <KLocalizedString>
#include <kwidgetsaddons_version.h>


using namespace SubtitleComposer;

void
Application::setupActions()
{
	KActionCollection *actionCollection = m_mainWindow->actionCollection();
	UserActionManager *actionManager = UserActionManager::instance();

	QAction *quitAction = KStandardAction::quit(m_mainWindow, &MainWindow::close, actionCollection);
	quitAction->setStatusTip(i18n("Exit the application"));

	QAction *prefsAction = KStandardAction::preferences(this, &Application::showPreferences, actionCollection);
	prefsAction->setStatusTip(i18n("Configure Subtitle Composer"));

	QAction *newSubtitleAction = new QAction(actionCollection);
	newSubtitleAction->setIcon(QIcon::fromTheme("document-new"));
	newSubtitleAction->setText(i18nc("@action:inmenu Create a new subtitle", "New"));
	newSubtitleAction->setStatusTip(i18n("Create an empty subtitle"));
	actionCollection->setDefaultShortcuts(newSubtitleAction, KStandardShortcut::openNew());
	connect(newSubtitleAction, &QAction::triggered, this, &Application::newSubtitle);
	actionCollection->addAction(ACT_NEW_SUBTITLE, newSubtitleAction);
	actionManager->addAction(newSubtitleAction, UserAction::FullScreenOff);

	QAction *openSubtitleAction = new QAction(actionCollection);
	openSubtitleAction->setIcon(QIcon::fromTheme("document-open"));
	openSubtitleAction->setText(i18nc("@action:inmenu Open subtitle file", "Open Subtitle..."));
	openSubtitleAction->setStatusTip(i18n("Open subtitle file"));
	actionCollection->setDefaultShortcuts(openSubtitleAction, KStandardShortcut::open());
	connect(openSubtitleAction, &QAction::triggered, [this](){ openSubtitle(); });
	actionCollection->addAction(ACT_OPEN_SUBTITLE, openSubtitleAction);
	actionManager->addAction(openSubtitleAction, 0);

	m_reopenSubtitleAsAction = new KCodecActionExt(actionCollection, KCodecActionExt::Open);
	m_reopenSubtitleAsAction->setIcon(QIcon::fromTheme("view-refresh"));
	m_reopenSubtitleAsAction->setText(i18n("Reload As..."));
	m_reopenSubtitleAsAction->setStatusTip(i18n("Reload opened file with a different encoding"));
	connect(m_reopenSubtitleAsAction, &KCodecActionExt::triggered, this, &Application::reopenSubtitleWithCodec);
	actionCollection->addAction(ACT_REOPEN_SUBTITLE_AS, m_reopenSubtitleAsAction);
	actionManager->addAction(m_reopenSubtitleAsAction, UserAction::SubOpened | UserAction::SubPClean | UserAction::FullScreenOff);

	m_recentSubtitlesAction = new KRecentFilesActionExt(actionCollection);
	m_recentSubtitlesAction->setIcon(QIcon::fromTheme("document-open"));
	m_recentSubtitlesAction->setText(i18nc("@action:inmenu Open rencently used subtitle file", "Open &Recent Subtitle"));
	m_recentSubtitlesAction->setStatusTip(i18n("Open subtitle file"));
	connect(m_recentSubtitlesAction, &KRecentFilesActionExt::urlSelected, [this](const QUrl &url){ openSubtitle(url); });
	actionCollection->addAction(ACT_RECENT_SUBTITLES, m_recentSubtitlesAction);

	QAction *saveSubtitleAction = new QAction(actionCollection);
	saveSubtitleAction->setIcon(QIcon::fromTheme("document-save"));
	saveSubtitleAction->setText(i18n("Save"));
	saveSubtitleAction->setStatusTip(i18n("Save opened subtitle"));
	actionCollection->setDefaultShortcuts(saveSubtitleAction, KStandardShortcut::save());
	connect(saveSubtitleAction, &QAction::triggered, [this](){ saveSubtitle(KCharsets::charsets()->codecForName(m_subtitleEncoding)); });
	actionCollection->addAction(ACT_SAVE_SUBTITLE, saveSubtitleAction);
	actionManager->addAction(saveSubtitleAction, UserAction::SubPDirty | UserAction::FullScreenOff);

	m_saveSubtitleAsAction = new KCodecActionExt(actionCollection, KCodecActionExt::Save);
	m_saveSubtitleAsAction->setIcon(QIcon::fromTheme("document-save-as"));
	m_saveSubtitleAsAction->setText(i18n("Save As..."));
	m_saveSubtitleAsAction->setStatusTip(i18n("Save opened subtitle with different settings"));
	connect(m_saveSubtitleAsAction, &KCodecActionExt::triggered, this, &Application::saveSubtitleAs);
	connect(m_saveSubtitleAsAction, &QAction::triggered, this, [&](){ saveSubtitleAs(); });
	actionCollection->addAction(ACT_SAVE_SUBTITLE_AS, m_saveSubtitleAsAction);
	actionManager->addAction(m_saveSubtitleAsAction, UserAction::SubOpened | UserAction::FullScreenOff);

	QAction *closeSubtitleAction = new QAction(actionCollection);
	closeSubtitleAction->setIcon(QIcon::fromTheme("window-close"));
	closeSubtitleAction->setText(i18n("Close"));
	closeSubtitleAction->setStatusTip(i18n("Close opened subtitle"));
	actionCollection->setDefaultShortcuts(closeSubtitleAction, KStandardShortcut::close());
	connect(closeSubtitleAction, &QAction::triggered, this, &Application::closeSubtitle);
	actionCollection->addAction(ACT_CLOSE_SUBTITLE, closeSubtitleAction);
	actionManager->addAction(closeSubtitleAction, UserAction::SubOpened | UserAction::FullScreenOff);

	QAction *newSubtitleTrAction = new QAction(actionCollection);
	newSubtitleTrAction->setIcon(QIcon::fromTheme("document-new"));
	newSubtitleTrAction->setText(i18n("New Translation"));
	newSubtitleTrAction->setStatusTip(i18n("Create an empty translation subtitle"));
	actionCollection->setDefaultShortcut(newSubtitleTrAction, QKeySequence("Ctrl+Shift+N"));
	connect(newSubtitleTrAction, &QAction::triggered, this, &Application::newSubtitleTr);
	actionCollection->addAction(ACT_NEW_SUBTITLE_TR, newSubtitleTrAction);
	actionManager->addAction(newSubtitleTrAction, UserAction::SubOpened | UserAction::FullScreenOff);

	QAction *openSubtitleTrAction = new QAction(actionCollection);
	openSubtitleTrAction->setIcon(QIcon::fromTheme("document-open"));
	openSubtitleTrAction->setText(i18n("Open Translation..."));
	openSubtitleTrAction->setStatusTip(i18n("Open translation subtitle file"));
	actionCollection->setDefaultShortcut(openSubtitleTrAction, QKeySequence("Ctrl+Shift+O"));
	connect(openSubtitleTrAction, &QAction::triggered, [this](){ openSubtitleTr(); });
	actionCollection->addAction(ACT_OPEN_SUBTITLE_TR, openSubtitleTrAction);
	actionManager->addAction(openSubtitleTrAction, UserAction::SubOpened);

	m_reopenSubtitleTrAsAction = new KCodecActionExt(actionCollection, KCodecActionExt::Open);
	m_reopenSubtitleTrAsAction->setIcon(QIcon::fromTheme("view-refresh"));
	m_reopenSubtitleTrAsAction->setText(i18n("Reload Translation As..."));
	m_reopenSubtitleTrAsAction->setStatusTip(i18n("Reload opened translation file with a different encoding"));
	connect(m_reopenSubtitleTrAsAction, &KCodecActionExt::triggered, this, &Application::reopenSubtitleTrWithCodec);
	actionCollection->addAction(ACT_REOPEN_SUBTITLE_TR_AS, m_reopenSubtitleTrAsAction);
	actionManager->addAction(m_reopenSubtitleTrAsAction, UserAction::SubTrOpened | UserAction::SubSClean | UserAction::FullScreenOff);

	m_recentSubtitlesTrAction = new KRecentFilesActionExt(actionCollection);
	m_recentSubtitlesTrAction->setIcon(QIcon::fromTheme("document-open"));
	m_recentSubtitlesTrAction->setText(i18n("Open &Recent Translation"));
	m_recentSubtitlesTrAction->setStatusTip(i18n("Open translation subtitle file"));
	connect(m_recentSubtitlesTrAction, &KRecentFilesActionExt::urlSelected, [this](const QUrl &url){ openSubtitleTr(url); });
	actionCollection->addAction(ACT_RECENT_SUBTITLES_TR, m_recentSubtitlesTrAction);
	actionManager->addAction(m_recentSubtitlesTrAction, UserAction::SubOpened | UserAction::FullScreenOff);

	QAction *saveSubtitleTrAction = new QAction(actionCollection);
	saveSubtitleTrAction->setIcon(QIcon::fromTheme("document-save"));
	saveSubtitleTrAction->setText(i18n("Save Translation"));
	saveSubtitleTrAction->setStatusTip(i18n("Save opened translation subtitle"));
	actionCollection->setDefaultShortcut(saveSubtitleTrAction, QKeySequence("Ctrl+Shift+S"));
	connect(saveSubtitleTrAction, &QAction::triggered, [&](){
		saveSubtitleTr(KCharsets::charsets()->codecForName(m_subtitleTrEncoding));
	});
	actionCollection->addAction(ACT_SAVE_SUBTITLE_TR, saveSubtitleTrAction);
	actionManager->addAction(saveSubtitleTrAction, UserAction::SubSDirty | UserAction::FullScreenOff);

	m_saveSubtitleTrAsAction = new KCodecActionExt(actionCollection, KCodecActionExt::Save);
	m_saveSubtitleTrAsAction->setIcon(QIcon::fromTheme("document-save-as"));
	m_saveSubtitleTrAsAction->setText(i18n("Save Translation As..."));
	m_saveSubtitleTrAsAction->setStatusTip(i18n("Save opened translation subtitle with different settings"));
	connect(m_saveSubtitleTrAsAction, &KCodecActionExt::triggered, this, &Application::saveSubtitleTrAs);
	connect(m_saveSubtitleTrAsAction, &QAction::triggered, this, [&](){ saveSubtitleTrAs(); });
	actionCollection->addAction(ACT_SAVE_SUBTITLE_TR_AS, m_saveSubtitleTrAsAction);
	actionManager->addAction(m_saveSubtitleTrAsAction, UserAction::SubTrOpened | UserAction::FullScreenOff);

	QAction *closeSubtitleTrAction = new QAction(actionCollection);
	closeSubtitleTrAction->setIcon(QIcon::fromTheme("window-close"));
	closeSubtitleTrAction->setText(i18n("Close Translation"));
	closeSubtitleTrAction->setStatusTip(i18n("Close opened translation subtitle"));
	actionCollection->setDefaultShortcut(closeSubtitleTrAction, QKeySequence("Ctrl+Shift+F4; Ctrl+Shift+W"));
	connect(closeSubtitleTrAction, &QAction::triggered, this, &Application::closeSubtitleTr);
	actionCollection->addAction(ACT_CLOSE_SUBTITLE_TR, closeSubtitleTrAction);
	actionManager->addAction(closeSubtitleTrAction, UserAction::SubTrOpened | UserAction::FullScreenOff);

	QAction *undoAction = m_undoStack->createUndoAction(actionManager);
	undoAction->setIcon(QIcon::fromTheme("edit-undo"));
	actionCollection->setDefaultShortcuts(undoAction, KStandardShortcut::undo());
	actionCollection->addAction(ACT_UNDO, undoAction);

	QAction *redoAction = m_undoStack->createRedoAction(actionManager);
	redoAction->setIcon(QIcon::fromTheme("edit-redo"));
	actionCollection->setDefaultShortcuts(redoAction, KStandardShortcut::redo());
	actionCollection->addAction(ACT_REDO, redoAction);

	QAction *splitSubtitleAction = new QAction(actionCollection);
	splitSubtitleAction->setText(i18n("Split Subtitle..."));
	splitSubtitleAction->setStatusTip(i18n("Split the opened subtitle in two parts"));
	connect(splitSubtitleAction, &QAction::triggered, this, &Application::splitSubtitle);
	actionCollection->addAction(ACT_SPLIT_SUBTITLE, splitSubtitleAction);
	actionManager->addAction(splitSubtitleAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	QAction *joinSubtitlesAction = new QAction(actionCollection);
	joinSubtitlesAction->setText(i18n("Join Subtitles..."));
	joinSubtitlesAction->setStatusTip(i18n("Append to the opened subtitle another one"));
	connect(joinSubtitlesAction, &QAction::triggered, this, &Application::joinSubtitles);
	actionCollection->addAction(ACT_JOIN_SUBTITLES, joinSubtitlesAction);
	actionManager->addAction(joinSubtitlesAction, UserAction::SubOpened | UserAction::FullScreenOff);

	QAction *insertBeforeCurrentLineAction = new QAction(actionCollection);
	insertBeforeCurrentLineAction->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
	insertBeforeCurrentLineAction->setText(i18n("Insert Before"));
	insertBeforeCurrentLineAction->setStatusTip(i18n("Insert empty line before current one"));
	actionCollection->setDefaultShortcut(insertBeforeCurrentLineAction, QKeySequence("Ctrl+Insert"));
	connect(insertBeforeCurrentLineAction, &QAction::triggered, this, &Application::insertBeforeCurrentLine);
	actionCollection->addAction(ACT_INSERT_BEFORE_CURRENT_LINE, insertBeforeCurrentLineAction);
	actionManager->addAction(insertBeforeCurrentLineAction, UserAction::SubOpened | UserAction::FullScreenOff);

	QAction *insertAfterCurrentLineAction = new QAction(actionCollection);
	insertAfterCurrentLineAction->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
	insertAfterCurrentLineAction->setText(i18n("Insert After"));
	insertAfterCurrentLineAction->setStatusTip(i18n("Insert empty line after current one"));
	actionCollection->setDefaultShortcut(insertAfterCurrentLineAction, QKeySequence("Insert"));
	connect(insertAfterCurrentLineAction, &QAction::triggered, this, &Application::insertAfterCurrentLine);
	actionCollection->addAction(ACT_INSERT_AFTER_CURRENT_LINE, insertAfterCurrentLineAction);
	actionManager->addAction(insertAfterCurrentLineAction, UserAction::SubOpened | UserAction::FullScreenOff);

	QAction *removeSelectedLinesAction = new QAction(actionCollection);
	removeSelectedLinesAction->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
	removeSelectedLinesAction->setText(i18n("Remove"));
	removeSelectedLinesAction->setStatusTip(i18n("Remove selected lines"));
	actionCollection->setDefaultShortcut(removeSelectedLinesAction, QKeySequence("Delete"));
	connect(removeSelectedLinesAction, &QAction::triggered, this, &Application::removeSelectedLines);
	actionCollection->addAction(ACT_REMOVE_SELECTED_LINES, removeSelectedLinesAction);
	actionManager->addAction(removeSelectedLinesAction, UserAction::HasSelection | UserAction::FullScreenOff);

	QAction *splitSelectedLinesAction = new QAction(actionCollection);
	splitSelectedLinesAction->setText(i18n("Split Lines"));
	splitSelectedLinesAction->setStatusTip(i18n("Split selected lines"));
	connect(splitSelectedLinesAction, &QAction::triggered, this, &Application::splitSelectedLines);
	actionCollection->addAction(ACT_SPLIT_SELECTED_LINES, splitSelectedLinesAction);
	actionManager->addAction(splitSelectedLinesAction, UserAction::HasSelection | UserAction::FullScreenOff);

	QAction *joinSelectedLinesAction = new QAction(actionCollection);
	joinSelectedLinesAction->setText(i18n("Join Lines"));
	joinSelectedLinesAction->setStatusTip(i18n("Join selected lines"));
	connect(joinSelectedLinesAction, &QAction::triggered, this, &Application::joinSelectedLines);
	actionCollection->addAction(ACT_JOIN_SELECTED_LINES, joinSelectedLinesAction);
	actionManager->addAction(joinSelectedLinesAction, UserAction::HasSelection | UserAction::FullScreenOff);

	QAction *selectAllLinesAction = new QAction(actionCollection);
	selectAllLinesAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-select-all")));
	selectAllLinesAction->setText(i18n("Select All"));
	selectAllLinesAction->setStatusTip(i18n("Select all lines"));
	actionCollection->setDefaultShortcuts(selectAllLinesAction, KStandardShortcut::selectAll());
	connect(selectAllLinesAction, &QAction::triggered, this, &Application::selectAllLines);
	actionCollection->addAction(ACT_SELECT_ALL_LINES, selectAllLinesAction);
	actionManager->addAction(selectAllLinesAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	QAction *gotoLineAction = new QAction(actionCollection);
	gotoLineAction->setText(i18n("Go to Line..."));
	gotoLineAction->setStatusTip(i18n("Go to specified line"));
	actionCollection->setDefaultShortcuts(gotoLineAction, KStandardShortcut::gotoLine());
	connect(gotoLineAction, &QAction::triggered, this, &Application::gotoLine);
	actionCollection->addAction(ACT_GOTO_LINE, gotoLineAction);
	actionManager->addAction(gotoLineAction, UserAction::SubHasLines);

	QAction *findAction = new QAction(actionCollection);
	findAction->setIcon(QIcon::fromTheme("edit-find"));
	findAction->setText(i18n("Find..."));
	findAction->setStatusTip(i18n("Find occurrences of strings or regular expressions"));
	actionCollection->setDefaultShortcuts(findAction, KStandardShortcut::find());
	connect(findAction, &QAction::triggered, this, &Application::find);
	actionCollection->addAction(ACT_FIND, findAction);
	actionManager->addAction(findAction, UserAction::SubHasLine);

	QAction *findNextAction = new QAction(actionCollection);
	findNextAction->setIcon(QIcon::fromTheme("go-down-search"));
	findNextAction->setText(i18n("Find Next"));
	findNextAction->setStatusTip(i18n("Find next occurrence of string or regular expression"));
	actionCollection->setDefaultShortcuts(findNextAction, KStandardShortcut::findNext());
	connect(findNextAction, &QAction::triggered, this, &Application::findNext);
	actionCollection->addAction(ACT_FIND_NEXT, findNextAction);
	actionManager->addAction(findNextAction, UserAction::SubHasLine);

	QAction *findPreviousAction = new QAction(actionCollection);
	findPreviousAction->setIcon(QIcon::fromTheme("go-up-search"));
	findPreviousAction->setText(i18n("Find Previous"));
	findPreviousAction->setStatusTip(i18n("Find previous occurrence of string or regular expression"));
	actionCollection->setDefaultShortcuts(findPreviousAction, KStandardShortcut::findPrev());
	connect(findPreviousAction, &QAction::triggered, this, &Application::findPrevious);
	actionCollection->addAction(ACT_FIND_PREVIOUS, findPreviousAction);
	actionManager->addAction(findPreviousAction, UserAction::SubHasLine);

	QAction *replaceAction = new QAction(actionCollection);
	replaceAction->setText(i18n("Replace..."));
	replaceAction->setStatusTip(i18n("Replace occurrences of strings or regular expressions"));
	actionCollection->setDefaultShortcuts(replaceAction, KStandardShortcut::replace());
	connect(replaceAction, &QAction::triggered, this, &Application::replace);
	actionCollection->addAction(ACT_REPLACE, replaceAction);
	actionManager->addAction(replaceAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	QAction *retrocedeCurrentLineAction = new QAction(actionCollection);
	retrocedeCurrentLineAction->setIcon(QIcon::fromTheme("go-down"));
	retrocedeCurrentLineAction->setText(i18n("Retrocede current line"));
	retrocedeCurrentLineAction->setStatusTip(i18n("Makes the line before the current one active"));
	actionCollection->setDefaultShortcut(retrocedeCurrentLineAction, QKeySequence("Alt+Up"));
	connect(retrocedeCurrentLineAction, &QAction::triggered, this, &Application::retrocedeCurrentLine);
	actionCollection->addAction(ACT_RETROCEDE_CURRENT_LINE, retrocedeCurrentLineAction);
	actionManager->addAction(retrocedeCurrentLineAction, UserAction::SubHasLines | UserAction::HasSelection);

	QAction *advanceCurrentLineAction = new QAction(actionCollection);
	advanceCurrentLineAction->setIcon(QIcon::fromTheme("go-down"));
	advanceCurrentLineAction->setText(i18n("Advance current line"));
	advanceCurrentLineAction->setStatusTip(i18n("Makes the line after the current one active"));
	actionCollection->setDefaultShortcut(advanceCurrentLineAction, QKeySequence("Alt+Down"));
	connect(advanceCurrentLineAction, &QAction::triggered, this, &Application::advanceCurrentLine);
	actionCollection->addAction(ACT_ADVANCE_CURRENT_LINE, advanceCurrentLineAction);
	actionManager->addAction(advanceCurrentLineAction, UserAction::SubHasLines | UserAction::HasSelection);

	QAction *detectErrorsAction = new QAction(actionCollection);
	detectErrorsAction->setIcon(QIcon::fromTheme("edit-find"));
	detectErrorsAction->setText(i18n("Detect Errors..."));
	detectErrorsAction->setStatusTip(i18n("Detect errors in the current subtitle"));
	actionCollection->setDefaultShortcut(detectErrorsAction, QKeySequence("Ctrl+E"));
	connect(detectErrorsAction, &QAction::triggered, this, &Application::detectErrors);
	actionCollection->addAction(ACT_DETECT_ERRORS, detectErrorsAction);
	actionManager->addAction(detectErrorsAction, UserAction::HasSelection | UserAction::FullScreenOff);

	QAction *clearErrorsAction = new QAction(actionCollection);
	clearErrorsAction->setText(i18n("Clear Errors/Marks"));
	clearErrorsAction->setStatusTip(i18n("Clear detected errors and marks in the current subtitle"));
	actionCollection->setDefaultShortcut(clearErrorsAction, QKeySequence("Ctrl+Shift+E"));
	connect(clearErrorsAction, &QAction::triggered, this, &Application::clearErrors);
	actionCollection->addAction(ACT_CLEAR_ERRORS, clearErrorsAction);
	actionManager->addAction(clearErrorsAction, UserAction::HasSelection | UserAction::FullScreenOff);

	QAction *nextErrorAction = new QAction(actionCollection);
	nextErrorAction->setIcon(QIcon::fromTheme("go-down-search"));
	nextErrorAction->setText(i18n("Select Next Error/Mark"));
	nextErrorAction->setStatusTip(i18n("Select next line with error or mark"));
	actionCollection->setDefaultShortcut(nextErrorAction, QKeySequence("F4"));
	connect(nextErrorAction, &QAction::triggered, this, &Application::selectNextError);
	actionCollection->addAction(ACT_SELECT_NEXT_ERROR, nextErrorAction);
	actionManager->addAction(nextErrorAction, UserAction::SubHasLine);

	QAction *prevErrorAction = new QAction(actionCollection);
	prevErrorAction->setIcon(QIcon::fromTheme("go-up-search"));
	prevErrorAction->setText(i18n("Select Previous Error/Mark"));
	prevErrorAction->setStatusTip(i18n("Select previous line with error or mark"));
	actionCollection->setDefaultShortcut(prevErrorAction, QKeySequence("Shift+F4"));
	connect(prevErrorAction, &QAction::triggered, this, &Application::selectPreviousError);
	actionCollection->addAction(ACT_SELECT_PREVIOUS_ERROR, prevErrorAction);
	actionManager->addAction(prevErrorAction, UserAction::SubHasLine);

	QAction *spellCheckAction = new QAction(actionCollection);
	spellCheckAction->setIcon(QIcon::fromTheme("tools-check-spelling"));
	spellCheckAction->setText(i18n("Spelling..."));
	spellCheckAction->setStatusTip(i18n("Check lines spelling"));
	connect(spellCheckAction, &QAction::triggered, this, &Application::spellCheck);
	actionCollection->addAction(ACT_SPELL_CHECK, spellCheckAction);
	actionManager->addAction(spellCheckAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	QAction *toggleSelectedLinesMarkAction = new QAction(actionCollection);
	toggleSelectedLinesMarkAction->setText(i18n("Toggle Mark"));
	toggleSelectedLinesMarkAction->setStatusTip(i18n("Toggle mark on selected lines"));
	actionCollection->setDefaultShortcut(toggleSelectedLinesMarkAction, QKeySequence("Ctrl+M"));
	connect(toggleSelectedLinesMarkAction, &QAction::triggered, this, &Application::toggleSelectedLinesMark);
	actionCollection->addAction(ACT_TOGGLE_SELECTED_LINES_MARK, toggleSelectedLinesMarkAction);
	actionManager->addAction(toggleSelectedLinesMarkAction, UserAction::HasSelection | UserAction::FullScreenOff);

	QAction *toggleSelectedLinesBoldAction = new QAction(actionCollection);
	toggleSelectedLinesBoldAction->setIcon(QIcon::fromTheme("format-text-bold"));
	toggleSelectedLinesBoldAction->setText(i18n("Toggle Bold"));
	toggleSelectedLinesBoldAction->setStatusTip(i18n("Toggle selected lines bold attribute"));
	actionCollection->setDefaultShortcut(toggleSelectedLinesBoldAction, QKeySequence("Ctrl+B"));
	connect(toggleSelectedLinesBoldAction, &QAction::triggered, this, &Application::toggleSelectedLinesBold);
	actionCollection->addAction(ACT_TOGGLE_SELECTED_LINES_BOLD, toggleSelectedLinesBoldAction);
	actionManager->addAction(toggleSelectedLinesBoldAction, UserAction::HasSelection | UserAction::FullScreenOff);

	QAction *toggleSelectedLinesItalicAction = new QAction(actionCollection);
	toggleSelectedLinesItalicAction->setIcon(QIcon::fromTheme("format-text-italic"));
	toggleSelectedLinesItalicAction->setText(i18n("Toggle Italic"));
	toggleSelectedLinesItalicAction->setStatusTip(i18n("Toggle selected lines italic attribute"));
	actionCollection->setDefaultShortcut(toggleSelectedLinesItalicAction, QKeySequence("Ctrl+I"));
	connect(toggleSelectedLinesItalicAction, &QAction::triggered, this, &Application::toggleSelectedLinesItalic);
	actionCollection->addAction(ACT_TOGGLE_SELECTED_LINES_ITALIC, toggleSelectedLinesItalicAction);
	actionManager->addAction(toggleSelectedLinesItalicAction, UserAction::HasSelection | UserAction::FullScreenOff);

	QAction *toggleSelectedLinesUnderlineAction = new QAction(actionCollection);
	toggleSelectedLinesUnderlineAction->setIcon(QIcon::fromTheme("format-text-underline"));
	toggleSelectedLinesUnderlineAction->setText(i18n("Toggle Underline"));
	toggleSelectedLinesUnderlineAction->setStatusTip(i18n("Toggle selected lines underline attribute"));
	actionCollection->setDefaultShortcut(toggleSelectedLinesUnderlineAction, QKeySequence("Ctrl+U"));
	connect(toggleSelectedLinesUnderlineAction, &QAction::triggered, this, &Application::toggleSelectedLinesUnderline);
	actionCollection->addAction(ACT_TOGGLE_SELECTED_LINES_UNDERLINE, toggleSelectedLinesUnderlineAction);
	actionManager->addAction(toggleSelectedLinesUnderlineAction, UserAction::HasSelection | UserAction::FullScreenOff);

	QAction *toggleSelectedLinesStrikeThroughAction = new QAction(actionCollection);
	toggleSelectedLinesStrikeThroughAction->setIcon(QIcon::fromTheme("format-text-strikethrough"));
	toggleSelectedLinesStrikeThroughAction->setText(i18n("Toggle Strike Through"));
	toggleSelectedLinesStrikeThroughAction->setStatusTip(i18n("Toggle selected lines strike through attribute"));
	actionCollection->setDefaultShortcut(toggleSelectedLinesStrikeThroughAction, QKeySequence("Ctrl+T"));
	connect(toggleSelectedLinesStrikeThroughAction, &QAction::triggered, this, &Application::toggleSelectedLinesStrikeThrough);
	actionCollection->addAction(ACT_TOGGLE_SELECTED_LINES_STRIKETHROUGH, toggleSelectedLinesStrikeThroughAction);
	actionManager->addAction(toggleSelectedLinesStrikeThroughAction, UserAction::HasSelection | UserAction::FullScreenOff);

	QAction *changeSelectedLinesColorAction = new QAction(actionCollection);
	changeSelectedLinesColorAction->setIcon(QIcon::fromTheme("format-text-color"));
	changeSelectedLinesColorAction->setText(i18n("Change Text Color"));
	changeSelectedLinesColorAction->setStatusTip(i18n("Change text color of selected lines"));
	actionCollection->setDefaultShortcut(changeSelectedLinesColorAction, QKeySequence("Ctrl+Shift+C"));
	connect(changeSelectedLinesColorAction, &QAction::triggered, this, &Application::changeSelectedLinesColor);
	actionCollection->addAction(ACT_CHANGE_SELECTED_LINES_TEXT_COLOR, changeSelectedLinesColorAction);
	actionManager->addAction(changeSelectedLinesColorAction, UserAction::HasSelection | UserAction::FullScreenOff);

	QAction *shiftAction = new QAction(actionCollection);
	shiftAction->setText(i18n("Shift..."));
	shiftAction->setStatusTip(i18n("Shift lines an specified amount of time"));
	actionCollection->setDefaultShortcut(shiftAction, QKeySequence("Ctrl+D"));
	connect(shiftAction, &QAction::triggered, this, &Application::shiftLines);
	actionCollection->addAction(ACT_SHIFT, shiftAction);
	actionManager->addAction(shiftAction, UserAction::SubHasLine | UserAction::FullScreenOff | UserAction::AnchorsNone);

	QAction *shiftSelectedLinesFwdAction = new QAction(actionCollection);
	actionCollection->setDefaultShortcut(shiftSelectedLinesFwdAction, QKeySequence("Shift++"));
	connect(shiftSelectedLinesFwdAction, &QAction::triggered, this, &Application::shiftSelectedLinesForwards);
	actionCollection->addAction(ACT_SHIFT_SELECTED_LINES_FORWARDS, shiftSelectedLinesFwdAction);
	actionManager->addAction(shiftSelectedLinesFwdAction, UserAction::HasSelection | UserAction::FullScreenOff | UserAction::EditableShowTime);

	QAction *shiftSelectedLinesBwdAction = new QAction(actionCollection);
	actionCollection->setDefaultShortcut(shiftSelectedLinesBwdAction, QKeySequence("Shift+-"));
	connect(shiftSelectedLinesBwdAction, &QAction::triggered, this, &Application::shiftSelectedLinesBackwards);
	actionCollection->addAction(ACT_SHIFT_SELECTED_LINES_BACKWARDS, shiftSelectedLinesBwdAction);
	actionManager->addAction(shiftSelectedLinesBwdAction, UserAction::HasSelection | UserAction::FullScreenOff | UserAction::EditableShowTime);

	QAction *adjustAction = new QAction(actionCollection);
	adjustAction->setText(i18n("Adjust..."));
	adjustAction->setStatusTip(i18n("Linearly adjust all lines to a specified interval"));
	actionCollection->setDefaultShortcut(adjustAction, QKeySequence("Ctrl+J"));
	connect(adjustAction, &QAction::triggered, this, &Application::adjustLines);
	actionCollection->addAction(ACT_ADJUST, adjustAction);
	actionManager->addAction(adjustAction, UserAction::SubHasLines | UserAction::FullScreenOff | UserAction::AnchorsNone);

	QAction *sortLinesAction = new QAction(actionCollection);
	sortLinesAction->setText(i18n("Sort..."));
	sortLinesAction->setStatusTip(i18n("Sort lines based on their show time"));
	connect(sortLinesAction, &QAction::triggered, this, &Application::sortLines);
	actionCollection->addAction(ACT_SORT_LINES, sortLinesAction);
	actionManager->addAction(sortLinesAction, UserAction::SubHasLines | UserAction::FullScreenOff);

	QAction *changeFrameRateAction = new QAction(actionCollection);
	changeFrameRateAction->setText(i18n("Change Frame Rate..."));
	changeFrameRateAction->setStatusTip(i18n("Retime subtitle lines by reinterpreting its frame rate"));
	connect(changeFrameRateAction, &QAction::triggered, this, &Application::changeFrameRate);
	actionCollection->addAction(ACT_CHANGE_FRAME_RATE, changeFrameRateAction);
	actionManager->addAction(changeFrameRateAction, UserAction::SubOpened | UserAction::FullScreenOff | UserAction::AnchorsNone);

	QAction *durationLimitsAction = new QAction(actionCollection);
	durationLimitsAction->setText(i18n("Enforce Duration Limits..."));
	durationLimitsAction->setStatusTip(i18n("Enforce lines minimum and/or maximum duration limits"));
	actionCollection->setDefaultShortcut(durationLimitsAction, QKeySequence("Ctrl+L"));
	connect(durationLimitsAction, &QAction::triggered, this, &Application::enforceDurationLimits);
	actionCollection->addAction(ACT_DURATION_LIMITS, durationLimitsAction);
	actionManager->addAction(durationLimitsAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	QAction *autoDurationsAction = new QAction(actionCollection);
	autoDurationsAction->setText(i18n("Set Automatic Durations..."));
	autoDurationsAction->setStatusTip(i18n("Set lines durations based on amount of letters, words and line breaks"));
	connect(autoDurationsAction, &QAction::triggered, this, &Application::setAutoDurations);
	actionCollection->addAction(ACT_AUTOMATIC_DURATIONS, autoDurationsAction);
	actionManager->addAction(autoDurationsAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	QAction *maximizeDurationsAction = new QAction(actionCollection);
	maximizeDurationsAction->setText(i18n("Maximize Durations..."));
	maximizeDurationsAction->setStatusTip(i18n("Extend lines durations up to their next lines show time"));
	connect(maximizeDurationsAction, &QAction::triggered, this, &Application::maximizeDurations);
	actionCollection->addAction(ACT_MAXIMIZE_DURATIONS, maximizeDurationsAction);
	actionManager->addAction(maximizeDurationsAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	QAction *fixOverlappingLinesAction = new QAction(actionCollection);
	fixOverlappingLinesAction->setText(i18n("Fix Overlapping Times..."));
	fixOverlappingLinesAction->setStatusTip(i18n("Fix lines overlapping times"));
	connect(fixOverlappingLinesAction, &QAction::triggered, this, &Application::fixOverlappingLines);
	actionCollection->addAction(ACT_FIX_OVERLAPPING_LINES, fixOverlappingLinesAction);
	actionManager->addAction(fixOverlappingLinesAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	QAction *syncWithSubtitleAction = new QAction(actionCollection);
	syncWithSubtitleAction->setText(i18n("Synchronize with Subtitle..."));
	syncWithSubtitleAction->setStatusTip(i18n("Copy timing information from another subtitle"));
	connect(syncWithSubtitleAction, &QAction::triggered, this, &Application::syncWithSubtitle);
	actionCollection->addAction(ACT_SYNC_WITH_SUBTITLE, syncWithSubtitleAction);
	actionManager->addAction(syncWithSubtitleAction, UserAction::SubHasLine | UserAction::FullScreenOff | UserAction::AnchorsNone);

	QAction *breakLinesAction = new QAction(actionCollection);
	breakLinesAction->setText(i18n("Break Lines..."));
	breakLinesAction->setStatusTip(i18n("Automatically set line breaks"));
	connect(breakLinesAction, &QAction::triggered, this, &Application::breakLines);
	actionCollection->addAction(ACT_ADJUST_TEXTS, breakLinesAction);
	actionManager->addAction(breakLinesAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	QAction *unbreakTextsAction = new QAction(actionCollection);
	unbreakTextsAction->setText(i18n("Unbreak Lines..."));
	unbreakTextsAction->setStatusTip(i18n("Remove line breaks from lines"));
	connect(unbreakTextsAction, &QAction::triggered, this, &Application::unbreakTexts);
	actionCollection->addAction(ACT_UNBREAK_TEXTS, unbreakTextsAction);
	actionManager->addAction(unbreakTextsAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	QAction *simplifySpacesAction = new QAction(actionCollection);
	simplifySpacesAction->setText(i18n("Simplify Spaces..."));
	simplifySpacesAction->setStatusTip(i18n("Remove unneeded spaces from lines"));
	connect(simplifySpacesAction, &QAction::triggered, this, &Application::simplifySpaces);
	actionCollection->addAction(ACT_SIMPLIFY_SPACES, simplifySpacesAction);
	actionManager->addAction(simplifySpacesAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	QAction *changeCaseAction = new QAction(actionCollection);
	changeCaseAction->setText(i18n("Change Case..."));
	changeCaseAction->setStatusTip(i18n("Change lines text to upper, lower, title or sentence case"));
	connect(changeCaseAction, &QAction::triggered, this, &Application::changeCase);
	actionCollection->addAction(ACT_CHANGE_CASE, changeCaseAction);
	actionManager->addAction(changeCaseAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	QAction *fixPunctuationAction = new QAction(actionCollection);
	fixPunctuationAction->setText(i18n("Fix Punctuation..."));
	fixPunctuationAction->setStatusTip(i18n("Fix punctuation errors in lines"));
	connect(fixPunctuationAction, &QAction::triggered, this, &Application::fixPunctuation);
	actionCollection->addAction(ACT_FIX_PUNCTUATION, fixPunctuationAction);
	actionManager->addAction(fixPunctuationAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	QAction *translateAction = new QAction(actionCollection);
	translateAction->setText(i18n("Translate..."));
	translateAction->setStatusTip(i18n("Translate lines texts using Google services"));
	connect(translateAction, &QAction::triggered, this, &Application::translate);
	actionCollection->addAction(ACT_TRANSLATE, translateAction);
	actionManager->addAction(translateAction, UserAction::SubHasLine | UserAction::FullScreenOff);

	QAction *editCurrentLineInPlaceAction = new QAction(actionCollection);
	editCurrentLineInPlaceAction->setText(i18n("Edit Line in Place"));
	editCurrentLineInPlaceAction->setStatusTip(i18n("Edit current line text in place"));
	actionCollection->setDefaultShortcut(editCurrentLineInPlaceAction, QKeySequence("F2"));
	connect(editCurrentLineInPlaceAction, &QAction::triggered, m_linesWidget, &LinesWidget::editCurrentLineInPlace);
	actionCollection->addAction(ACT_EDIT_CURRENT_LINE_IN_PLACE, editCurrentLineInPlaceAction);
	actionManager->addAction(editCurrentLineInPlaceAction, UserAction::HasSelection | UserAction::FullScreenOff);

	QAction *openVideoAction = new QAction(actionCollection);
	openVideoAction->setIcon(QIcon::fromTheme("document-open"));
	openVideoAction->setText(i18n("Open Video..."));
	openVideoAction->setStatusTip(i18n("Open video file"));
	connect(openVideoAction, &QAction::triggered, [this](){ openVideo(); });
	actionCollection->addAction(ACT_OPEN_VIDEO, openVideoAction);

	m_recentVideosAction = new KRecentFilesActionExt(actionCollection);
	m_recentVideosAction->setIcon(QIcon::fromTheme("document-open"));
	m_recentVideosAction->setText(i18n("Open &Recent Video"));
	m_recentVideosAction->setStatusTip(i18n("Open video file"));
	connect(m_recentVideosAction, &KRecentFilesActionExt::urlSelected, [this](const QUrl &url){ openVideo(url); });
	actionCollection->addAction(ACT_RECENT_VIDEOS, m_recentVideosAction);

	QAction *demuxTextStreamAction = new KSelectAction(actionCollection);
	QMenu *demuxTextStreamActionMenu = new QMenu(m_mainWindow);
	demuxTextStreamAction->setMenu(demuxTextStreamActionMenu);
	demuxTextStreamAction->setIcon(QIcon::fromTheme("select_stream"));
	demuxTextStreamAction->setText(i18n("Import Subtitle Stream"));
	demuxTextStreamAction->setStatusTip(i18n("Import subtitle stream into subtitle editor"));
	connect(demuxTextStreamActionMenu, &QMenu::triggered, [this](QAction *action){ demuxTextStream(action->data().value<int>()); });
	actionCollection->addAction(ACT_DEMUX_TEXT_STREAM, demuxTextStreamAction);

	QAction *speechImportStreamAction = new KSelectAction(actionCollection);
	QMenu *speechImportStreamActionMenu = new QMenu(m_mainWindow);
	speechImportStreamAction->setMenu(speechImportStreamActionMenu);
	speechImportStreamAction->setIcon(QIcon::fromTheme("select_stream"));
	speechImportStreamAction->setText(i18n("Recognize Speech"));
	speechImportStreamAction->setStatusTip(i18n("Recognize speech in audio stream"));
	connect(speechImportStreamActionMenu, &QMenu::triggered, [this](QAction *action){ speechImportAudioStream(action->data().value<int>()); });
	actionCollection->addAction(ACT_ASR_IMPORT_AUDIO_STREAM, speechImportStreamAction);

	QAction *closeVideoAction = new QAction(actionCollection);
	closeVideoAction->setIcon(QIcon::fromTheme("window-close"));
	closeVideoAction->setText(i18n("Close Video"));
	closeVideoAction->setStatusTip(i18n("Close video file"));
	connect(closeVideoAction, &QAction::triggered, m_player, &VideoPlayer::closeFile);
	actionCollection->addAction(ACT_CLOSE_VIDEO, closeVideoAction);
	actionManager->addAction(closeVideoAction, UserAction::VideoOpened);

	KToggleAction *fullScreenAction = new KToggleAction(actionCollection);
	fullScreenAction->setIcon(QIcon::fromTheme("view-fullscreen"));
	fullScreenAction->setText(i18n("Full Screen Mode"));
	fullScreenAction->setStatusTip(i18n("Toggle full screen mode"));
	actionCollection->setDefaultShortcuts(fullScreenAction, KStandardShortcut::fullScreen());
	connect(fullScreenAction, &QAction::toggled, this, &Application::setFullScreenMode);
	actionCollection->addAction(ACT_TOGGLE_FULL_SCREEN, fullScreenAction);

	QAction *stopAction = new QAction(actionCollection);
	stopAction->setIcon(QIcon::fromTheme("media-playback-stop"));
	stopAction->setText(i18n("Stop"));
	stopAction->setStatusTip(i18n("Stop video playback"));
	connect(stopAction, &QAction::triggered, m_player, &VideoPlayer::stop);
	actionCollection->addAction(ACT_STOP, stopAction);
	actionManager->addAction(stopAction, UserAction::VideoPlaying);

	QAction *playPauseAction = new QAction(actionCollection);
	playPauseAction->setIcon(QIcon::fromTheme("media-playback-start"));
	playPauseAction->setText(i18n("Play"));
	playPauseAction->setStatusTip(i18n("Toggle video playing/paused"));
	actionCollection->setDefaultShortcut(playPauseAction, QKeySequence("Ctrl+Space"));
	connect(playPauseAction, &QAction::triggered, m_player, &VideoPlayer::togglePlayPaused);
	actionCollection->addAction(ACT_PLAY_PAUSE, playPauseAction);
	actionManager->addAction(playPauseAction, UserAction::VideoOpened);

	QAction *seekBackwardAction = new QAction(actionCollection);
	seekBackwardAction->setIcon(QIcon::fromTheme("media-seek-backward"));
	seekBackwardAction->setText(i18n("Seek Backward"));
	actionCollection->setDefaultShortcut(seekBackwardAction, QKeySequence("Left"));
	connect(seekBackwardAction, &QAction::triggered, this, &Application::seekBackward);
	actionCollection->addAction(ACT_SEEK_BACKWARD, seekBackwardAction);
	actionManager->addAction(seekBackwardAction, UserAction::VideoPlaying);

	QAction *seekForwardAction = new QAction(actionCollection);
	seekForwardAction->setIcon(QIcon::fromTheme("media-seek-forward"));
	seekForwardAction->setText(i18n("Seek Forward"));
	actionCollection->setDefaultShortcut(seekForwardAction, QKeySequence("Right"));
	connect(seekForwardAction, &QAction::triggered, this, &Application::seekForward);
	actionCollection->addAction(ACT_SEEK_FORWARD, seekForwardAction);
	actionManager->addAction(seekForwardAction, UserAction::VideoPlaying);

	QAction *stepFrameBackwardAction = new QAction(actionCollection);
	stepFrameBackwardAction->setText(i18n("Frame Step Backward"));
	actionCollection->setDefaultShortcut(stepFrameBackwardAction, QKeySequence("Ctrl+Left"));
	connect(stepFrameBackwardAction, &QAction::triggered, this, &Application::stepBackward);
	actionCollection->addAction(ACT_STEP_FRAME_BACKWARD, stepFrameBackwardAction);
	actionManager->addAction(stepFrameBackwardAction, UserAction::VideoPlaying);

	QAction *stepFrameForwardAction = new QAction(actionCollection);
	stepFrameForwardAction->setText(i18n("Frame Step Forward"));
	actionCollection->setDefaultShortcut(stepFrameForwardAction, QKeySequence("Ctrl+Right"));
	connect(stepFrameForwardAction, &QAction::triggered, this, &Application::stepForward);
	actionCollection->addAction(ACT_STEP_FRAME_FORWARD, stepFrameForwardAction);
	actionManager->addAction(stepFrameForwardAction, UserAction::VideoPlaying);

	QAction *seekToPrevLineAction = new QAction(actionCollection);
	seekToPrevLineAction->setIcon(QIcon::fromTheme("media-skip-backward"));
	seekToPrevLineAction->setText(i18n("Seek to Previous Line"));
	seekToPrevLineAction->setStatusTip(i18n("Seek to previous subtitle line show time"));
	actionCollection->setDefaultShortcut(seekToPrevLineAction, QKeySequence("Shift+Left"));
	connect(seekToPrevLineAction, &QAction::triggered, this, &Application::seekToPrevLine);
	actionCollection->addAction(ACT_SEEK_TO_PREVIOUS_LINE, seekToPrevLineAction);
	actionManager->addAction(seekToPrevLineAction, UserAction::SubHasLine | UserAction::VideoPlaying);

	QAction *playrateIncreaseAction = new QAction(actionCollection);
	playrateIncreaseAction->setIcon(QIcon::fromTheme(QStringLiteral("playrate_plus")));
	playrateIncreaseAction->setText(i18n("Increase media play rate"));
	playrateIncreaseAction->setStatusTip(i18n("Increase media playback rate"));
	connect(playrateIncreaseAction, &QAction::triggered, this, &Application::playrateIncrease);
	actionCollection->addAction(ACT_PLAY_RATE_INCREASE, playrateIncreaseAction);
	actionManager->addAction(playrateIncreaseAction, UserAction::VideoPlaying);

	QAction *playrateDecreaseAction = new QAction(actionCollection);
	playrateDecreaseAction->setIcon(QIcon::fromTheme(QStringLiteral("playrate_minus")));
	playrateDecreaseAction->setText(i18n("Decrease media play rate"));
	playrateDecreaseAction->setStatusTip(i18n("Decrease media playback rate"));
	connect(playrateDecreaseAction, &QAction::triggered, this, &Application::playrateDecrease);
	actionCollection->addAction(ACT_PLAY_RATE_DECREASE, playrateDecreaseAction);
	actionManager->addAction(playrateDecreaseAction, UserAction::VideoPlaying);

	QAction *playCurrentLineAndPauseAction = new QAction(actionCollection);
	playCurrentLineAndPauseAction->setText(i18n("Play Current Line and Pause"));
	playCurrentLineAndPauseAction->setStatusTip(i18n("Seek to selected subtitle line show time, play it and then pause"));
	actionCollection->setDefaultShortcut(playCurrentLineAndPauseAction, QKeySequence("Ctrl+Return"));
	connect(playCurrentLineAndPauseAction, &QAction::triggered, this, &Application::playOnlyCurrentLine);
	actionCollection->addAction(ACT_PLAY_CURRENT_LINE_AND_PAUSE, playCurrentLineAndPauseAction);
	actionManager->addAction(playCurrentLineAndPauseAction, UserAction::SubHasLine | UserAction::VideoPlaying);

	QAction *seekCurrentLineAction = new QAction(actionCollection);
	seekCurrentLineAction->setText(i18n("Seek to Current Line"));
	seekCurrentLineAction->setStatusTip(i18n("Seek to selected subtitle line show time"));
	connect(seekCurrentLineAction, &QAction::triggered, this, &Application::seekToCurrentLine);
	actionCollection->addAction(ACT_SEEK_TO_CURRENT_LINE, seekCurrentLineAction);
	actionManager->addAction(seekCurrentLineAction, UserAction::SubHasLine | UserAction::VideoPlaying);

	QAction *seekToNextLineAction = new QAction(actionCollection);
	seekToNextLineAction->setIcon(QIcon::fromTheme("media-skip-forward"));
	seekToNextLineAction->setText(i18n("Seek to Next Line"));
	seekToNextLineAction->setStatusTip(i18n("Seek to next subtitle line show time"));
	actionCollection->setDefaultShortcut(seekToNextLineAction, QKeySequence("Shift+Right"));
	connect(seekToNextLineAction, &QAction::triggered, this, &Application::seekToNextLine);
	actionCollection->addAction(ACT_SEEK_TO_NEXT_LINE, seekToNextLineAction);
	actionManager->addAction(seekToNextLineAction, UserAction::SubHasLine | UserAction::VideoPlaying);

	QAction *setCurrentLineShowTimeFromVideoAction = new QAction(actionCollection);
	setCurrentLineShowTimeFromVideoAction->setIcon(QIcon::fromTheme(QStringLiteral("set_show_time")));
	setCurrentLineShowTimeFromVideoAction->setText(i18n("Set Line Show Time to Video Position"));
	setCurrentLineShowTimeFromVideoAction->setStatusTip(i18n("Set current line show time to video position"));
	actionCollection->setDefaultShortcut(setCurrentLineShowTimeFromVideoAction, QKeySequence("Shift+Z"));
	connect(setCurrentLineShowTimeFromVideoAction, &QAction::triggered, this, &Application::setCurrentLineShowTimeFromVideo);
	actionCollection->addAction(ACT_SET_CURRENT_LINE_SHOW_TIME, setCurrentLineShowTimeFromVideoAction);
	actionManager->addAction(setCurrentLineShowTimeFromVideoAction, UserAction::HasSelection | UserAction::VideoPlaying | UserAction::EditableShowTime);

	QAction *setCurrentLineHideTimeFromVideoAction = new QAction(actionCollection);
	setCurrentLineHideTimeFromVideoAction->setIcon(QIcon::fromTheme(QStringLiteral("set_hide_time")));
	setCurrentLineHideTimeFromVideoAction->setText(i18n("Set Line Hide Time to Video Position"));
	setCurrentLineHideTimeFromVideoAction->setStatusTip(i18n("Set current line hide time to video position"));
	actionCollection->setDefaultShortcut(setCurrentLineHideTimeFromVideoAction, QKeySequence("Shift+X"));
	connect(setCurrentLineHideTimeFromVideoAction, &QAction::triggered, this, &Application::setCurrentLineHideTimeFromVideo);
	actionCollection->addAction(ACT_SET_CURRENT_LINE_HIDE_TIME, setCurrentLineHideTimeFromVideoAction);
	actionManager->addAction(setCurrentLineHideTimeFromVideoAction, UserAction::HasSelection | UserAction::VideoPlaying | UserAction::EditableShowTime);

	QAction *shiftToVideoPositionAction = new QAction(actionCollection);
	shiftToVideoPositionAction->setText(i18n("Shift Current Line to Video Position"));
	shiftToVideoPositionAction->setStatusTip(i18n("Set current line show time to video position by equally shifting all lines"));
	actionCollection->setDefaultShortcut(shiftToVideoPositionAction, QKeySequence("Shift+A"));
	connect(shiftToVideoPositionAction, &QAction::triggered, this, &Application::shiftToVideoPosition);
	actionCollection->addAction(ACT_SHIFT_TO_VIDEO_POSITION, shiftToVideoPositionAction);
	actionManager->addAction(shiftToVideoPositionAction, UserAction::HasSelection | UserAction::VideoPlaying | UserAction::FullScreenOff | UserAction::EditableShowTime);

	KToggleAction *currentLineFollowsVideoAction = new KToggleAction(actionCollection);
	currentLineFollowsVideoAction->setIcon(QIcon::fromTheme(QStringLiteral("current_line_follows_video")));
	currentLineFollowsVideoAction->setText(i18n("Current Line Follows Video"));
	currentLineFollowsVideoAction->setStatusTip(i18n("Make current line follow the playing video position"));
	connect(currentLineFollowsVideoAction, &QAction::toggled, this, &Application::onLinkCurrentLineToVideoToggled);
	actionCollection->addAction(ACT_CURRENT_LINE_FOLLOWS_VIDEO, currentLineFollowsVideoAction);

	KToggleAction *toggleMuteAction = new KToggleAction(actionCollection);
	toggleMuteAction->setIcon(QIcon::fromTheme("audio-volume-muted"));
	toggleMuteAction->setText(i18nc("@action:inmenu Toggle audio muted", "Mute"));
	toggleMuteAction->setStatusTip(i18n("Enable/disable playback sound"));
	actionCollection->setDefaultShortcut(toggleMuteAction, QKeySequence("/"));
	connect(toggleMuteAction, &QAction::toggled, m_player, &VideoPlayer::setMuted);
	actionCollection->addAction(ACT_TOGGLE_MUTED, toggleMuteAction);

	QAction *increaseVolumeAction = new QAction(actionCollection);
	increaseVolumeAction->setIcon(QIcon::fromTheme("audio-volume-high"));
	increaseVolumeAction->setText(i18n("Increase Volume"));
	increaseVolumeAction->setStatusTip(i18n("Increase volume by 5%"));
	actionCollection->setDefaultShortcut(increaseVolumeAction, Qt::Key_Plus);
	connect(increaseVolumeAction, &QAction::triggered, [this](){ m_player->increaseVolume(); });
	actionCollection->addAction(ACT_INCREASE_VOLUME, increaseVolumeAction);

	QAction *decreaseVolumeAction = new QAction(actionCollection);
	decreaseVolumeAction->setIcon(QIcon::fromTheme("audio-volume-low"));
	decreaseVolumeAction->setText(i18n("Decrease Volume"));
	decreaseVolumeAction->setStatusTip(i18n("Decrease volume by 5%"));
	actionCollection->setDefaultShortcut(decreaseVolumeAction, Qt::Key_Minus);
	connect(decreaseVolumeAction, &QAction::triggered, [this](){ m_player->decreaseVolume(); });
	actionCollection->addAction(ACT_DECREASE_VOLUME, decreaseVolumeAction);

	KSelectAction *setActiveAudioStreamAction = new KSelectAction(actionCollection);
	setActiveAudioStreamAction->setIcon(QIcon::fromTheme(QStringLiteral("languages")));
	setActiveAudioStreamAction->setText(i18n("Audio Streams"));
	setActiveAudioStreamAction->setStatusTip(i18n("Select active audio stream"));
#if KWIDGETSADDONS_VERSION < QT_VERSION_CHECK(5, 78, 0)
	connect(setActiveAudioStreamAction, QOverload<int>::of(&KSelectAction::triggered), m_player, &VideoPlayer::selectAudioStream);
#else
	connect(setActiveAudioStreamAction, &KSelectAction::indexTriggered, m_player, &VideoPlayer::selectAudioStream);
#endif
	actionCollection->addAction(ACT_SET_ACTIVE_AUDIO_STREAM, setActiveAudioStreamAction);
	actionManager->addAction(setActiveAudioStreamAction, UserAction::VideoOpened);

	QAction *increaseSubtitleFontAction = new QAction(actionCollection);
	increaseSubtitleFontAction->setIcon(QIcon::fromTheme("format-font-size-more"));
	increaseSubtitleFontAction->setText(i18n("Increase Font Size"));
	increaseSubtitleFontAction->setStatusTip(i18n("Increase subtitles font size by 1 point"));
	actionCollection->setDefaultShortcut(increaseSubtitleFontAction, QKeySequence("Alt++"));
	connect(increaseSubtitleFontAction, &QAction::triggered, m_playerWidget, &PlayerWidget::increaseFontSize);
	actionCollection->addAction(ACT_INCREASE_SUBTITLE_FONT, increaseSubtitleFontAction);

	QAction *decreaseSubtitleFontAction = new QAction(actionCollection);
	decreaseSubtitleFontAction->setIcon(QIcon::fromTheme("format-font-size-less"));
	decreaseSubtitleFontAction->setText(i18n("Decrease Font Size"));
	decreaseSubtitleFontAction->setStatusTip(i18n("Decrease subtitles font size by 1 point"));
	actionCollection->setDefaultShortcut(decreaseSubtitleFontAction, QKeySequence("Alt+-"));
	connect(decreaseSubtitleFontAction, &QAction::triggered, m_playerWidget, &PlayerWidget::decreaseFontSize);
	actionCollection->addAction(ACT_DECREASE_SUBTITLE_FONT, decreaseSubtitleFontAction);

	KSelectAction *setActiveSubtitleStreamAction = new KSelectAction(actionCollection);
	setActiveSubtitleStreamAction->setIcon(QIcon::fromTheme(QStringLiteral("languages")));
	setActiveSubtitleStreamAction->setText(i18n("Displayed Subtitle Text"));
	setActiveSubtitleStreamAction->setStatusTip(i18n("Select visible subtitle text"));
#if KWIDGETSADDONS_VERSION < QT_VERSION_CHECK(5, 78, 0)
	connect(setActiveSubtitleStreamAction, QOverload<int>::of(&KSelectAction::triggered), this, &Application::setActiveSubtitleStream);
#else
	connect(setActiveSubtitleStreamAction, &KSelectAction::indexTriggered, this, &Application::setActiveSubtitleStream);
#endif
	actionCollection->addAction(ACT_SET_ACTIVE_SUBTITLE_STREAM, setActiveSubtitleStreamAction);
	actionManager->addAction(setActiveSubtitleStreamAction, UserAction::SubTrOpened);

	QAction *toggleAnchor = new QAction(actionCollection);
	toggleAnchor->setIcon(QIcon::fromTheme(QStringLiteral("anchor")));
	toggleAnchor->setText(i18n("Toggle Anchor"));
	toggleAnchor->setStatusTip(i18n("(Un)Anchor current line's show time to timeline (Editing anchored line's show time will stretch/compact the timeline between adjacent anchors)"));
	actionCollection->setDefaultShortcut(toggleAnchor, QKeySequence("Alt+A"));
	connect(toggleAnchor, &QAction::triggered, this, &Application::anchorToggle);
	actionCollection->addAction(ACT_ANCHOR_TOGGLE, toggleAnchor);
	actionManager->addAction(toggleAnchor, UserAction::HasSelection | UserAction::FullScreenOff);

	QAction *removeAllAnchors = new QAction(actionCollection);
	removeAllAnchors->setText(i18n("Remove All Anchors"));
	removeAllAnchors->setStatusTip(i18n("Unanchor show time from the timeline on all anchored lines"));
	actionCollection->setDefaultShortcut(removeAllAnchors, QKeySequence("Alt+Shift+A"));
	connect(removeAllAnchors, &QAction::triggered, this, &Application::anchorRemoveAll);
	actionCollection->addAction(ACT_ANCHOR_REMOVE_ALL, removeAllAnchors);
	actionManager->addAction(removeAllAnchors, UserAction::HasSelection | UserAction::FullScreenOff | UserAction::AnchorsSome);

	QAction *scriptsManagerAction = new QAction(actionCollection);
	scriptsManagerAction->setIcon(QIcon::fromTheme("folder-development"));
	scriptsManagerAction->setText(i18nc("@action:inmenu Manage user scripts", "Scripts Manager..."));
	scriptsManagerAction->setStatusTip(i18n("Manage user scripts"));
	connect(scriptsManagerAction, &QAction::triggered, m_scriptsManager, &ScriptsManager::showDialog);
	actionCollection->addAction(ACT_SCRIPTS_MANAGER, scriptsManagerAction);
	actionManager->addAction(scriptsManagerAction, UserAction::FullScreenOff);

	QAction *waveformZoomInAction = new QAction(actionCollection);
	waveformZoomInAction->setIcon(QIcon::fromTheme("zoom-in"));
	waveformZoomInAction->setText(i18n("Waveform Zoom In"));
	waveformZoomInAction->setStatusTip(i18n("Waveform Zoom In"));
	connect(waveformZoomInAction, &QAction::triggered, m_mainWindow->m_waveformWidget, &WaveformWidget::zoomIn);
	actionCollection->addAction(ACT_WAVEFORM_ZOOM_IN, waveformZoomInAction);

	QAction *waveformZoomOutAction = new QAction(actionCollection);
	waveformZoomOutAction->setIcon(QIcon::fromTheme("zoom-out"));
	waveformZoomOutAction->setText(i18n("Waveform Zoom Out"));
	waveformZoomOutAction->setStatusTip(i18n("Waveform Zoom Out"));
	connect(waveformZoomOutAction, &QAction::triggered, m_mainWindow->m_waveformWidget, &WaveformWidget::zoomOut);
	actionCollection->addAction(ACT_WAVEFORM_ZOOM_OUT, waveformZoomOutAction);

	QAction *waveformAutoScrollAction = new QAction(actionCollection);
	waveformAutoScrollAction->setCheckable(true);
	waveformAutoScrollAction->setIcon(QIcon::fromTheme(QStringLiteral("current_line_follows_video")));
	waveformAutoScrollAction->setText(i18n("Waveform Auto Scroll"));
	waveformAutoScrollAction->setStatusTip(i18n("Waveform display will automatically scroll to video position"));
	connect(waveformAutoScrollAction, &QAction::toggled, m_mainWindow->m_waveformWidget, &WaveformWidget::setAutoscroll);
	actionCollection->addAction(ACT_WAVEFORM_AUTOSCROLL, waveformAutoScrollAction);

	updateActionTexts();

	emit actionsReady();
}
