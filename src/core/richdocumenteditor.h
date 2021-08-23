/*
    SPDX-FileCopyrightText: 2020 The Qt Company Ltd.
    SPDX-FileCopyrightText: 2020 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RICHDOCUMENTEDITOR_H
#define RICHDOCUMENTEDITOR_H

// based on Qt 5.15.2 private QWidgetLineControl and QLineEdit

#include <QtGlobal>

#include <QClipboard>
#include <QCompleter>
#include <QInputMethod>
#include <QPoint>
#include <QPointer>
#include <QTextLayout>
#include <QTextDocumentFragment>

#include <vector>

#include "core/richdocument.h"

#ifdef DrawText
#  undef DrawText
#endif

namespace SubtitleComposer {

class RichDocumentEditor : public QObject
{
	Q_OBJECT

public:
	RichDocumentEditor();

	bool isAcceptableInput(const QKeyEvent *event) const;
	static bool isCommonTextEditShortcut(const QKeyEvent *ke);

	void setAccessibleObject(QObject *object)
	{
		Q_ASSERT(object);
		m_accessibleObject = object;
	}

	QObject *accessibleObject()
	{
		if(m_accessibleObject)
			return m_accessibleObject;
		return parent();
	}

	bool hasSelection() const { return m_textCursor && m_textCursor->hasSelection(); }

	int width() const { return qRound(m_textLayout.lineAt(0).width()) + 1; }
	int height() const { return qRound(m_textLayout.lineAt(0).height()) + 1; }
	int ascent() const { return m_ascent; }
	qreal naturalTextWidth() const { return m_textLayout.lineAt(0).naturalTextWidth(); }

	void setSelection(int start, int length);

	inline QString selectedText() const { return m_textCursor ? m_textCursor->selectedText() : QString(); }
	inline QTextDocumentFragment selection() const { return m_textCursor ? m_textCursor->selection() : QTextDocumentFragment(); }

	int selectionStart() const { return m_textCursor ? m_textCursor->selectionStart() : -1; }
	int selectionEnd() const { return m_textCursor ? m_textCursor->selectionEnd() : -1; }
	bool selectionContains(int cursorPos) {
		if(!m_textCursor)
			return false;
		return m_textCursor->selectionStart() <= cursorPos && cursorPos < m_textCursor->selectionEnd();
	}
	bool selectionContainsX(int x) { return selectionContains(xToPos(x)); }

	bool inSelection(int x) const
	{
		if(!m_textCursor || !m_textCursor->hasSelection())
			return false;
		int pos = xToPos(x, QTextLine::CursorOnCharacter);
		return pos >= m_textCursor->selectionStart() && pos < m_textCursor->selectionEnd();
	}

	void eraseSelectedText()
	{
		removeSelectedText();
		finishChange(true);
	}

	int start() const { return 0; }
	int end() const { return m_document->length(); }

#ifndef QT_NO_CLIPBOARD
	void cut(QClipboard::Mode mode = QClipboard::Clipboard);
	void copy(QClipboard::Mode mode = QClipboard::Clipboard) const;
	void paste(QClipboard::Mode mode = QClipboard::Clipboard);
#endif

	int cursor() const{ return m_textCursor ? m_textCursor->position() : -1; }

	int cursorWidth() const { return m_cursorWidth; }
	void setCursorWidth(int value) { m_cursorWidth = value; }

	Qt::CursorMoveStyle cursorMoveStyle() const { return m_document->defaultCursorMoveStyle(); }
	void setCursorMoveStyle(Qt::CursorMoveStyle style) { m_document->setDefaultCursorMoveStyle(style); }

	void cursorSetPosition(int pos, bool mark=false);
	void cursorMoveRelative(QTextCursor::MoveOperation oper, bool mark=false, int n=1);
	void cursorMovePosition(int steps, bool mark=false);

	void cursorWordForward(bool mark) { cursorMoveRelative(QTextCursor::NextWord, mark); }
	void cursorWordBackward(bool mark) { cursorMoveRelative(QTextCursor::PreviousWord, mark); }

	void home(bool mark);
	void end(bool mark);

	int xToPos(int x, QTextLine::CursorPosition = QTextLine::CursorBetweenCharacters) const;
	QRect rectForPos(int pos) const;
	QRect cursorRect() const;
	QRect anchorRect() const;

	qreal cursorToX(int cursor) const { return m_textLayout.lineAt(0).cursorToX(cursor); }
	qreal cursorToX() const { return cursorToX(m_textCursor ? m_textCursor->position() : 0); }

	bool isReadOnly() const { return m_readOnly; }
	void setReadOnly(bool enable);

	void setDocument(RichDocument *doc);

	QString text() const
	{
		return m_document == nullptr ? QString::fromLatin1("") : m_document->toPlainText();
	}
	void commitPreedit();

	QString displayText() const { return m_textLayout.text(); }

	QString surroundingText() const
	{
		return m_document == nullptr ? QString::fromLatin1("") : m_document->toPlainText();
	}

	void backspace();
	void del();
	void deselect() { internalDeselect(); finishChange(false); }
	void selectAll();

	enum TextType { Auto, Plain, HTML };
	int insert(const QString &html, int pos=-1, TextType textType=Auto);
	void clear();
	void selectWordAtPos(int);

	QTextCharFormat styleAtPosition(int pos) const;

	void toggleBold();
	void toggleItalic();
	void toggleUnderline();
	void toggleStrikeOut();
	const QColor textColor() const;
	void setTextColor(const QColor &color);

#if QT_CONFIG(completer)
	QCompleter *completer() const { return m_completer; }
	// Note that you must set the widget for the completer separately
	void setCompleter(const QCompleter *c) { m_completer = const_cast<QCompleter*>(c); }
	void complete(int key);
#endif

	int cursorPosition() const { return m_textCursor ? m_textCursor->position() : -1; }

	// input methods
#ifndef QT_NO_IM
	bool composeMode() const { return !m_textLayout.preeditAreaText().isEmpty(); }
	void setPreeditArea(int cursor, const QString &text) { m_textLayout.setPreeditArea(cursor, text); }
#endif

	QString preeditAreaText() const { return m_textLayout.preeditAreaText(); }

	Qt::LayoutDirection layoutDirection() const {
		if(m_layoutDirection == Qt::LayoutDirectionAuto && !m_document->isEmpty())
			return m_document->toPlainText().isRightToLeft() ? Qt::RightToLeft : Qt::LeftToRight;
		return m_layoutDirection;
	}
	void setLayoutDirection(Qt::LayoutDirection direction)
	{
		if(direction != m_layoutDirection) {
			m_layoutDirection = direction;
			updateDisplayText();
		}
	}

	void setFont(const QFont &font) { m_textLayout.setFont(font); updateDisplayText(); }

	void processInputMethodEvent(QInputMethodEvent *event);
	void processKeyEvent(QKeyEvent* ev);

	void setBlinkingCursorEnabled(bool enable);
	void updateCursorBlinking();

	const QPalette &palette() const { return m_palette; }
	void setPalette(const QPalette &p) { m_palette = p; }

	enum DrawFlags {
		DrawText = 0x01,
		DrawSelections = 0x02,
		DrawCursor = 0x04,
		DrawAll = DrawText | DrawSelections | DrawCursor
	};
	void draw(QPainter *, const QPoint &, const QRect &, int flags=DrawAll, int cursorPos=-1);

#ifndef QT_NO_SHORTCUT
	void processShortcutOverrideEvent(QKeyEvent *ke);
#endif

	QTextLayout *textLayout() const
	{
		return &m_textLayout;
	}

	void updateDisplayText(bool forceUpdate = false);

private:
	void init();

	void removeSelectedText();

	void internalDelete(bool wasBackspace = false);

	inline void internalDeselect()
	{
		if(m_textCursor) {
			m_selDirty |= m_textCursor->hasSelection();
			m_textCursor->clearSelection();
		}
	}

	void emitCursorPositionChanged();

	bool finishChange(bool edited);

	QPointer<QCompleter> m_completer;
#if QT_CONFIG(completer)
	bool advanceToEnabledItem(int dir);
#endif

	// masking
	bool hasAcceptableInput(const QString &text) const;

	virtual void timerEvent(QTimerEvent *event) override;

	int redoTextLayout() const;

signals:
	void cursorPositionChanged(int, int);
	void selectionChanged();

	void displayTextChanged(const QString &);
	void textChanged(const QString &);
	void textEdited(const QString &);

	void resetInputContext();
	void updateMicroFocus();

	void accepted();
	void editingFinished();
	void updateNeeded(const QRect &);
	void inputRejected();

private:
	mutable QTextLayout m_textLayout;

	RichDocument *m_document = nullptr;
	QTextCursor *m_textCursor = nullptr;

	QPalette m_palette;
	Qt::LayoutDirection m_layoutDirection = Qt::LayoutDirectionAuto;
	int m_cursorWidth = 0;

	uint m_readOnly : 1;
	uint m_textDirty : 1;
	uint m_selDirty : 1;
	uint m_blinkStatus : 1;
	uint m_blinkEnabled : 1;

	int m_blinkTimer = 0;
	int m_ascent = 0;
	int m_lastCursorPos = -1;

	int m_keyboardScheme = 0;
	QObject *m_accessibleObject = nullptr;
};

}

#endif // RICHDOCUMENTEDITOR_H
