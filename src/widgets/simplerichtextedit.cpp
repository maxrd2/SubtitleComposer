/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "simplerichtextedit.h"

#include "appglobal.h"
#include "application.h"
#include "actions/useractionnames.h"
#include "core/undo/undostack.h"
#include "core/richtext/richdocument.h"
#include "dialogs/subtitlecolordialog.h"

#include <QRegExp>
#include <QEvent>
#include <QMenu>
#include <QShortcutEvent>
#include <QContextMenuEvent>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QAction>
#include <QIcon>
#include <QDebug>

#include <KStandardShortcut>
#include <KLocalizedString>

using namespace SubtitleComposer;

SimpleRichTextEdit::SimpleRichTextEdit(QWidget *parent)
	: KTextEdit(parent)
{
	enableFindReplace(false);
	setCheckSpellingEnabled(true);

	setAutoFormatting(AutoNone);
	setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);

	setTextInteractionFlags(Qt::TextEditorInteraction);

	connect(app(), &Application::actionsReady, this, &SimpleRichTextEdit::setupActions);

	QMenu *menu = QTextEdit::createStandardContextMenu();
	menu->setParent(this);
	QList<QAction *> actions = menu->actions();
	m_insertUnicodeControlCharMenu = 0;
	for(QList<QAction *>::ConstIterator it = actions.constBegin(), end = actions.constEnd(); it != end; ++it) {
		if((*it)->menu()) {
			// this depends on Qt private implementation but at least is guaranteed
			// to behave reasonably if that implementation changes in the future.
			if(!strcmp((*it)->menu()->metaObject()->className(), "QUnicodeControlCharacterMenu")) {
				m_insertUnicodeControlCharMenu = (*it)->menu();
				break;
			}
		}
	}
}

SimpleRichTextEdit::~SimpleRichTextEdit()
{
	if(m_insertUnicodeControlCharMenu)
		delete m_insertUnicodeControlCharMenu->parent();
}

void
SimpleRichTextEdit::changeTextColor()
{
	QColor color = SubtitleComposer::SubtitleColorDialog::getColor(textColor(), this);
	if(color.isValid()) {
		if(color.rgba() == 0) {
			QTextCursor cursor(textCursor());
			QTextCharFormat format;
			format.setForeground(QBrush(Qt::NoBrush));
			cursor.mergeCharFormat(format);
			setTextCursor(cursor);
		} else {
			setTextColor(color);
		}
	}
}

void
SimpleRichTextEdit::deleteText()
{
	QTextCursor cursor = textCursor();
	if(cursor.hasSelection())
		cursor.removeSelectedText();
	else
		cursor.deleteChar();
}

void
SimpleRichTextEdit::undoableClear()
{
	QTextCursor cursor = textCursor();
	cursor.beginEditBlock();
	cursor.select(QTextCursor::Document);
	cursor.removeSelectedText();
	cursor.endEditBlock();
}

void
SimpleRichTextEdit::setSelection(int startIndex, int endIndex)
{
	QTextCursor cursor(document());
	cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, startIndex);
	cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, endIndex - startIndex + 1);
	setTextCursor(cursor);
}

void
SimpleRichTextEdit::clearSelection()
{
	QTextCursor cursor(textCursor());
	cursor.clearSelection();
	setTextCursor(cursor);
}

void
SimpleRichTextEdit::setupWordUnderPositionCursor(const QPoint &globalPos)
{
	// Get the word under the (mouse-)cursor with apostrophes at the start/end
	m_selectedWordCursor = cursorForPosition(mapFromGlobal(globalPos));
	m_selectedWordCursor.clearSelection();
	m_selectedWordCursor.select(QTextCursor::WordUnderCursor);

	QString selectedWord = m_selectedWordCursor.selectedText();

	// Clear the selection again, we re-select it below (without the apostrophes).
	m_selectedWordCursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, selectedWord.size());
	if(selectedWord.startsWith('\'') || selectedWord.startsWith('\"')) {
		selectedWord = selectedWord.right(selectedWord.size() - 1);
		m_selectedWordCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor);
	}

	if(selectedWord.endsWith('\'') || selectedWord.endsWith('\"'))
		selectedWord.chop(1);

	m_selectedWordCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, selectedWord.size());
}

void
SimpleRichTextEdit::addToIgnoreList()
{
	highlighter()->ignoreWord(m_selectedWordCursor.selectedText());
	highlighter()->rehighlight();
	m_selectedWordCursor.clearSelection();
}

void
SimpleRichTextEdit::addToDictionary()
{
	highlighter()->addWordToDictionary(m_selectedWordCursor.selectedText());
	highlighter()->rehighlight();
	m_selectedWordCursor.clearSelection();
}

void
SimpleRichTextEdit::replaceWithSuggestion()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if(action) {
		m_selectedWordCursor.insertText(action->text());
		setTextCursor(m_selectedWordCursor);
		m_selectedWordCursor.clearSelection();
	}
}

QMenu *
SimpleRichTextEdit::createContextMenu(const QPoint &mouseGlobalPos)
{
	Qt::TextInteractionFlags interactionFlags = this->textInteractionFlags();
	QTextDocument *document = this->document();
	QTextCursor cursor = textCursor();

	const bool showTextSelectionActions = (Qt::TextEditable | Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse) & interactionFlags;

	QMenu *menu = new QMenu(this);

	if(interactionFlags & Qt::TextEditable) {
		m_actions[Undo]->setEnabled(appUndoStack()->canUndo());
		menu->addAction(m_actions[Undo]);

		m_actions[Redo]->setEnabled(appUndoStack()->canRedo());
		menu->addAction(m_actions[Redo]);

		menu->addSeparator();

		m_actions[Cut]->setEnabled(cursor.hasSelection());
		menu->addAction(m_actions[Cut]);
	}

	if(showTextSelectionActions) {
		m_actions[Copy]->setEnabled(cursor.hasSelection());
		menu->addAction(m_actions[Copy]);
	}

	if(interactionFlags & Qt::TextEditable) {
#if !defined(QT_NO_CLIPBOARD)
		m_actions[Paste]->setEnabled(canPaste());
		menu->addAction(m_actions[Paste]);
#endif
		m_actions[Delete]->setEnabled(cursor.hasSelection());
		menu->addAction(m_actions[Delete]);

		m_actions[Clear]->setEnabled(!document->isEmpty());
		menu->addAction(m_actions[Clear]);

		if(m_insertUnicodeControlCharMenu && interactionFlags & Qt::TextEditable) {
			menu->addSeparator();
			menu->addMenu(m_insertUnicodeControlCharMenu);
		}
	}

	if(showTextSelectionActions) {
		menu->addSeparator();

		m_actions[SelectAll]->setEnabled(!document->isEmpty());
		menu->addAction(m_actions[SelectAll]);
	}

	if(interactionFlags & Qt::TextEditable) {
		menu->addSeparator();

		m_actions[ToggleBold]->setChecked(fontBold());
		menu->addAction(m_actions[ToggleBold]);

		m_actions[ToggleItalic]->setChecked(fontItalic());
		menu->addAction(m_actions[ToggleItalic]);

		m_actions[ToggleUnderline]->setChecked(fontUnderline());
		menu->addAction(m_actions[ToggleUnderline]);

		m_actions[ToggleStrikeOut]->setChecked(fontStrikeOut());
		menu->addAction(m_actions[ToggleStrikeOut]);

		menu->addAction(m_actions[ChangeTextColor]);

		menu->addSeparator();

		m_actions[CheckSpelling]->setEnabled(!document->isEmpty());
		menu->addAction(m_actions[CheckSpelling]);

		m_actions[ToggleAutoSpellChecking]->setChecked(checkSpellingEnabled());
		menu->addAction(m_actions[ToggleAutoSpellChecking]);

		if(checkSpellingEnabled()) {
			setupWordUnderPositionCursor(mouseGlobalPos);

			QString selectedWord = m_selectedWordCursor.selectedText();
			if(!selectedWord.isEmpty() && highlighter() && highlighter()->isWordMisspelled(selectedWord)) {
				QMenu *suggestionsMenu = menu->addMenu(i18n("Suggestions"));
				suggestionsMenu->addAction(i18n("Ignore"), this, &SimpleRichTextEdit::addToIgnoreList);
				suggestionsMenu->addAction(i18n("Add to Dictionary"), this, &SimpleRichTextEdit::addToDictionary);
				suggestionsMenu->addSeparator();
				QStringList suggestions = highlighter()->suggestionsForWord(m_selectedWordCursor.selectedText());
				if(suggestions.empty())
					suggestionsMenu->addAction(i18n("No suggestions"))->setEnabled(false);
				else {
					for(QStringList::ConstIterator it = suggestions.constBegin(), end = suggestions.constEnd(); it != end; ++it) 
						suggestionsMenu->addAction(*it, this, &SimpleRichTextEdit::replaceWithSuggestion);
				}
			}
		}

		menu->addSeparator();

		m_actions[AllowTabulations]->setChecked(!tabChangesFocus());
		menu->addAction(m_actions[AllowTabulations]);
	}

	return menu;
}

void
SimpleRichTextEdit::contextMenuEvent(QContextMenuEvent *event)
{
	QMenu *menu = createContextMenu(event->globalPos());
	menu->exec(event->globalPos());
	delete menu;
}

bool
SimpleRichTextEdit::event(QEvent *event)
{
	if(event->type() == QEvent::ShortcutOverride) {
		const QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		const QKeySequence key(keyEvent->modifiers() + keyEvent->key());

		for(int i = 0; i < ActionCount; i++) {
			if(m_actions.at(i)->shortcuts().contains(key)) {
				event->accept();
				return true;
			}
		}
	}

	return KTextEdit::event(event);
}

void
SimpleRichTextEdit::keyPressEvent(QKeyEvent *event)
{
	const QKeySequence key(event->modifiers() + event->key());

	for(int i = 0; i < ActionCount; i++) {
		if(m_actions.at(i)->shortcuts().contains(key)) {
			m_actions.at(i)->trigger();
			if(i == Undo || i == Redo) {
				RichDocument *doc = qobject_cast<RichDocument *>(document());
				if(doc)
					setTextCursor(*doc->undoableCursor());
			}
			return;
		}
	}

	KTextEdit::keyPressEvent(event);
}

static void
setupActionCommon(QAction *act, const char *appActionId)
{
	QAction *appAction = qobject_cast<QAction *>(app()->action(appActionId));
	QObject::connect(appAction, &QAction::changed, act, [act, appAction](){ act->setShortcut(appAction->shortcut()); });
	act->setShortcuts(appAction->shortcuts());
}

void
SimpleRichTextEdit::setupActions()
{
	m_actions[Undo] = app()->action(ACT_UNDO);
	m_actions[Redo] = app()->action(ACT_REDO);

	QAction *act = m_actions[Cut] = new QAction(this);
	act->setIcon(QIcon::fromTheme("edit-cut"));
	act->setText(i18n("Cut"));
	act->setShortcuts(KStandardShortcut::cut());
	connect(act, &QAction::triggered, this, &QTextEdit::cut);

	act = m_actions[Copy] = new QAction(this);
	act->setIcon(QIcon::fromTheme("edit-copy"));
	act->setText(i18n("Copy"));
	act->setShortcuts(KStandardShortcut::copy());
	connect(act, &QAction::triggered, this, &QTextEdit::copy);

#ifndef QT_NO_CLIPBOARD
	act = m_actions[Paste] = new QAction(this);
	act->setIcon(QIcon::fromTheme("edit-paste"));
	act->setText(i18n("Paste"));
	act->setShortcuts(KStandardShortcut::paste());
	connect(act, &QAction::triggered, this, &QTextEdit::paste);
#endif

	act = m_actions[Delete] = new QAction(this);
	act->setIcon(QIcon::fromTheme("edit-delete"));
	act->setText(i18n("Delete"));
	act->setShortcut(QKeySequence::Delete);
	connect(act, &QAction::triggered, this, &SimpleRichTextEdit::deleteText);

	act = m_actions[Clear] = new QAction(this);
	act->setIcon(QIcon::fromTheme("edit-clear"));
	act->setText(i18nc("@action:inmenu Clear all text", "Clear"));
	connect(act, &QAction::triggered, this, &SimpleRichTextEdit::undoableClear);

	act = m_actions[SelectAll] = new QAction(this);
	act->setIcon(QIcon::fromTheme("edit-select-all"));
	act->setText(i18n("Select All"));
	setupActionCommon(act, ACT_SELECT_ALL_LINES);
	connect(act, &QAction::triggered, this, &QTextEdit::selectAll);

	act = m_actions[ToggleBold] = new QAction(this);
	act->setIcon(QIcon::fromTheme("format-text-bold"));
	act->setText(i18nc("@action:inmenu Toggle bold style", "Bold"));
	act->setCheckable(true);
	setupActionCommon(act, ACT_TOGGLE_SELECTED_LINES_BOLD);
	connect(act, &QAction::triggered, this, &SimpleRichTextEdit::toggleFontBold);

	act = m_actions[ToggleItalic] = new QAction(this);
	act->setIcon(QIcon::fromTheme("format-text-italic"));
	act->setText(i18nc("@action:inmenu Toggle italic style", "Italic"));
	act->setCheckable(true);
	setupActionCommon(act, ACT_TOGGLE_SELECTED_LINES_ITALIC);
	connect(act, &QAction::triggered, this, &SimpleRichTextEdit::toggleFontItalic);

	act = m_actions[ToggleUnderline] = new QAction(this);
	act->setIcon(QIcon::fromTheme("format-text-underline"));
	act->setText(i18nc("@action:inmenu Toggle underline style", "Underline"));
	act->setCheckable(true);
	setupActionCommon(act, ACT_TOGGLE_SELECTED_LINES_UNDERLINE);
	connect(act, &QAction::triggered, this, &SimpleRichTextEdit::toggleFontUnderline);

	act = m_actions[ToggleStrikeOut] = new QAction(this);
	act->setIcon(QIcon::fromTheme("format-text-strikethrough"));
	act->setText(i18nc("@action:inmenu Toggle strike through style", "Strike Through"));
	act->setCheckable(true);
	setupActionCommon(act, ACT_TOGGLE_SELECTED_LINES_STRIKETHROUGH);
	connect(act, &QAction::triggered, this, &SimpleRichTextEdit::toggleFontStrikeOut);

	act = m_actions[ChangeTextColor] = new QAction(this);
	act->setIcon(QIcon::fromTheme("format-text-color"));
	act->setText(i18nc("@action:inmenu Change Text Color", "Text Color"));
	setupActionCommon(act, ACT_CHANGE_SELECTED_LINES_TEXT_COLOR);
	connect(act, &QAction::triggered, this, &SimpleRichTextEdit::changeTextColor);

	act = m_actions[CheckSpelling] = new QAction(this);
	act->setIcon(QIcon::fromTheme("tools-check-spelling"));
	act->setText(i18n("Check Spelling..."));
	connect(act, &QAction::triggered, app(), &Application::spellCheck);
	connect(act, &QAction::triggered, this, &KTextEdit::checkSpelling);

	act = m_actions[ToggleAutoSpellChecking] = new QAction(this);
	act->setText(i18n("Auto Spell Check"));
	act->setCheckable(true);
	connect(act, &QAction::triggered, this, &SimpleRichTextEdit::toggleAutoSpellChecking);

	act = m_actions[AllowTabulations] = new QAction(this);
	act->setText(i18n("Allow Tabulations"));
	connect(act, &QAction::triggered, this, &SimpleRichTextEdit::toggleTabChangesFocus);
}

