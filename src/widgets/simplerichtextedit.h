#ifndef SIMPLERICHTEXTEDIT_H
#define SIMPLERICHTEXTEDIT_H

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

#include "../core/sstring.h"

#include <KTextEdit>

#include <KShortcut>

class QEvent;
class QKeyEvent;
class QFocusEvent;
class KAction;
class QMenu;

class SimpleRichTextEdit : public KTextEdit
{
	Q_OBJECT

public:
	typedef enum {
		Undo = 0, Redo,
		Cut, Copy, Paste, Delete, Clear, SelectAll,
		ToggleBold, ToggleItalic, ToggleUnderline, ToggleStrikeOut,
		CheckSpelling, ToggleAutoSpellChecking,
		AllowTabulations, ChangeTextColor,
		ActionCount
	} Action;

	explicit SimpleRichTextEdit(QWidget *parent = 0);
	virtual ~SimpleRichTextEdit();

	bool hasSelection() const;
	QString selectedText() const;

	bool fontBold();
	bool fontStrikeOut();

	virtual KAction * action(int action) const;
	virtual QList<KAction *> actions() const;

	virtual bool event(QEvent *event);

public slots:
	SubtitleComposer::SString richText();
	void setRichText(const SubtitleComposer::SString &richText);

	void setSelection(int startIndex, int endIndex);
	void clearSelection();

	void setFontBold(bool enabled);
	void setFontStrikeOut(bool enabled);

	void toggleFontBold();
	void toggleFontItalic();
	void toggleFontUnderline();
	void toggleFontStrikeOut();
	void changeTextColor();

	void deleteText();
	void undoableClear();

	void clearUndoRedoHistory();

	void toggleTabChangesFocus();
	void toggleAutoSpellChecking();

protected:
	QMenu * createContextMenu(const QPoint &mousePos);

	virtual void contextMenuEvent(QContextMenuEvent *event);

	virtual void keyPressEvent(QKeyEvent *event);

	void setupWordUnderPositionCursor(const QPoint &globalPos);

protected slots:
	void addToIgnoreList();
	void addToDictionary();
	void replaceWithSuggestion();

protected:
	KAction *m_actions[ActionCount];
	QMenu *m_insertUnicodeControlCharMenu;
	QTextCursor m_selectedWordCursor;
};

#endif
