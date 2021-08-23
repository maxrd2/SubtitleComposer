/*
    SPDX-FileCopyrightText: 2016 The Qt Company Ltd.
    SPDX-FileCopyrightText: 2020 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "richdocumenteditor.h"

#include <QAbstractItemView>
#include <QAccessible>
#include <QApplication>
#include <QClipboard>
#include <QGraphicsSceneEvent>
#include <QMimeData>
#include <QStyleHints>
#include <QTextBlock>

#include <private/qguiapplication_p.h>
#include <qpa/qplatformtheme.h>


#define HANDLE_TAB_KEY false

using namespace SubtitleComposer;

RichDocumentEditor::RichDocumentEditor()
	: QObject(nullptr),
	  m_readOnly(0),
	  m_textDirty(0),
	  m_selDirty(0),
	  m_blinkStatus(0),
	  m_blinkEnabled(false)
{
	init();
}

int
RichDocumentEditor::redoTextLayout() const
{
	m_textLayout.clearLayout();

	m_textLayout.beginLayout();
	QTextLine l = m_textLayout.createLine();
	m_textLayout.endLayout();

	return qRound(l.ascent());
}

void
RichDocumentEditor::updateDisplayText(bool forceUpdate)
{
	QString orig = m_textLayout.text();

	QString text;
	QVector<QTextLayout::FormatRange> ranges;
	if(m_document) {
		for(QTextBlock bi = m_document->begin(); bi != m_document->end(); bi = bi.next()) {
			if(bi != m_document->begin()) {
				QTextCharFormat fmt;
				fmt.setTextOutline(QPen(QApplication::palette().color(QPalette::Link), .75));
				ranges.push_back(QTextLayout::FormatRange{text.length(), 1, fmt});
				text.append(QChar(0x299a));
			}
			for(QTextBlock::iterator it = bi.begin(); !it.atEnd(); ++it) {
				if(!it.fragment().isValid())
					continue;
				const QString &t = it.fragment().text();
				ranges.push_back(QTextLayout::FormatRange{text.length(), t.length(), it.fragment().charFormat()});
				text.append(t);
			}
		}
	}

	// replace certain non-printable characters with spaces (to avoid
	// drawing boxes when using fonts that don't have glyphs for such
	// characters)
	QChar* uc = text.data();
	for(int i = 0; i < (int)text.length(); ++i) {
		if((uc[i].unicode() < 0x20 && uc[i].unicode() != 0x09)
			|| uc[i] == QChar::LineSeparator
			|| uc[i] == QChar::ParagraphSeparator
			|| uc[i] == QChar::ObjectReplacementCharacter)
			uc[i] = QChar(0x0020);
	}

	m_textLayout.setText(text);
	m_textLayout.setFormats(ranges);

	QTextOption option = m_textLayout.textOption();
	option.setTextDirection(m_layoutDirection);
	option.setFlags(QTextOption::IncludeTrailingSpaces);
	m_textLayout.setTextOption(option);

	m_ascent = redoTextLayout();

	if(text != orig || forceUpdate)
		emit displayTextChanged(text);
}

#ifndef QT_NO_CLIPBOARD
void
RichDocumentEditor::cut(QClipboard::Mode mode)
{
	copy(mode);
	removeSelectedText();
}

void
RichDocumentEditor::copy(QClipboard::Mode mode) const
{
	const QTextDocumentFragment s = selection();
	if(!s.isEmpty()) {
		QMimeData *mime = new QMimeData();
		mime->setHtml(s.toHtml());
		mime->setText(s.toPlainText());
		QGuiApplication::clipboard()->setMimeData(mime, mode);
	}
}

void
RichDocumentEditor::paste(QClipboard::Mode clipboardMode)
{
	const QMimeData *clip = QGuiApplication::clipboard()->mimeData(clipboardMode);
	if(!clip->text().isEmpty() || hasSelection()) {
		if(clip->hasHtml())
			insert(clip->html(), -1, HTML);
		else
			insert(clip->text(), -1, Plain);
	}
}

#endif // !QT_NO_CLIPBOARD

void
RichDocumentEditor::commitPreedit()
{
#ifndef QT_NO_IM
	if(!composeMode())
		return;

	QGuiApplication::inputMethod()->commit();
	if(!composeMode())
		return;

	setPreeditArea(-1, QString());
	m_textLayout.clearFormats();
	updateDisplayText(/*force*/ true);
#endif
}

void
RichDocumentEditor::backspace()
{
	if(hasSelection())
		removeSelectedText();
	else
		internalDelete(true);
	finishChange(true);
}

void
RichDocumentEditor::del()
{
	if(hasSelection())
		removeSelectedText();
	else
		internalDelete();
	finishChange(true);
}

int
RichDocumentEditor::insert(const QString &html, int pos, TextType textType)
{
	int curPos = m_textCursor->position();
	removeSelectedText();
	if(pos >= 0) {
		pos -= curPos - m_textCursor->position();
		m_textCursor->movePosition(
					pos > curPos ? QTextCursor::NextCharacter : QTextCursor::PreviousCharacter,
					QTextCursor::MoveAnchor, qAbs(pos - curPos));
		curPos = pos;
	}
#ifndef QT_NO_ACCESSIBILITY
	QAccessibleTextInsertEvent insertEvent(accessibleObject(), curPos, html);
	QAccessible::updateAccessibility(&insertEvent);
#endif
	if(textType == Auto)
		textType = Qt::mightBeRichText(html) ? HTML : Plain;
	if(textType == HTML)
		m_textCursor->insertHtml(html);
	else
		m_textCursor->insertText(html);
	m_textDirty = true;
	finishChange(true);
	return m_textCursor->position() - curPos;
}

void
RichDocumentEditor::clear()
{
	if(!m_document || m_document->isEmpty())
		return;

	m_textCursor->beginEditBlock();
	m_textCursor->select(QTextCursor::Document);
	m_textCursor->removeSelectedText();
	m_textCursor->endEditBlock();

	finishChange(true);
}

void
RichDocumentEditor::setSelection(int start, int length)
{
	commitPreedit();

	m_textCursor->movePosition(QTextCursor::Start);
	m_textCursor->movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, start);
	m_textCursor->movePosition(
				length > 0 ? QTextCursor::NextCharacter : QTextCursor::PreviousCharacter,
				QTextCursor::KeepAnchor, qAbs(length));

	emit selectionChanged();
	emitCursorPositionChanged();
}

void
RichDocumentEditor::init()
{
	m_textLayout.setCacheEnabled(true);
	updateDisplayText();
	if(const QPlatformTheme *theme = QGuiApplicationPrivate::platformTheme()) {
		m_keyboardScheme = theme->themeHint(QPlatformTheme::KeyboardScheme).toInt();
	}
	// Generalize for X11
	if(m_keyboardScheme == QPlatformTheme::KdeKeyboardScheme
		|| m_keyboardScheme == QPlatformTheme::GnomeKeyboardScheme
		|| m_keyboardScheme == QPlatformTheme::CdeKeyboardScheme) {
		m_keyboardScheme = QPlatformTheme::X11KeyboardScheme;
	}
}

int
RichDocumentEditor::xToPos(int x, QTextLine::CursorPosition betweenOrOn) const
{
	return textLayout()->lineAt(0).xToCursor(x, betweenOrOn);
}

QRect
RichDocumentEditor::rectForPos(int pos) const
{
	QTextLine l = textLayout()->lineAt(0);
	int cix = qRound(l.cursorToX(pos));
	int w = m_cursorWidth;
	int ch = l.height() + 1;

	return QRect(cix-5, 0, w+9, ch);
}

QRect
RichDocumentEditor::cursorRect() const
{
	return rectForPos(m_textCursor->position());
}

QRect
RichDocumentEditor::anchorRect() const
{
	return rectForPos(m_textCursor->anchor());
}

void
RichDocumentEditor::cursorSetPosition(int pos, bool mark)
{
	cursorMovePosition(pos - m_textCursor->position(), mark);
}

void
RichDocumentEditor::cursorMovePosition(int steps, bool mark)
{
	cursorMoveRelative(steps > 0 ? QTextCursor::NextCharacter : QTextCursor::PreviousCharacter, mark, qAbs(steps));
}

void
RichDocumentEditor::cursorMoveRelative(QTextCursor::MoveOperation oper, bool mark, int n)
{
	commitPreedit();

	if(mark) {
		m_textCursor->movePosition(oper, QTextCursor::KeepAnchor, n);
		updateDisplayText();
	} else {
		m_textCursor->movePosition(oper, QTextCursor::MoveAnchor, n);
		internalDeselect();
	}
	if(mark || m_selDirty) {
		m_selDirty = false;
		emit selectionChanged();
	}

	emitCursorPositionChanged();
}

void
RichDocumentEditor::home(bool mark)
{
	if(!m_textCursor)
		return;
	if(m_textCursor->atBlockStart()) {
		m_textCursor->movePosition(QTextCursor::PreviousCharacter,
				mark ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
	}
	cursorMoveRelative(QTextCursor::StartOfLine, mark);
}

void
RichDocumentEditor::end(bool mark)
{
	if(!m_textCursor)
		return;
	if(m_textCursor->atBlockEnd()) {
		m_textCursor->movePosition(QTextCursor::NextCharacter,
				mark ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
	}
	cursorMoveRelative(QTextCursor::EndOfLine, mark);
}

void
RichDocumentEditor::processInputMethodEvent(QInputMethodEvent *event)
{
	Q_UNUSED(event);
	/* FIXME:
	int priorState = -1;
	bool isGettingInput = !event->commitString().isEmpty()
			|| event->preeditString() != preeditAreaText()
			|| event->replacementLength() > 0;
	bool cursorPositionChanged = false;
	bool selectionChange = false;

	if(isGettingInput) {
		// If any text is being input, remove selected text.
		removeSelectedText();
	}

	int c = m_cursor; // cursor position after insertion of commit string
	if(event->replacementStart() <= 0)
		c += event->commitString().length() - qMin(-event->replacementStart(), event->replacementLength());

	m_cursor += event->replacementStart();
	if(m_cursor < 0)
		m_cursor = 0;

	// insert commit string
	if(event->replacementLength()) {
		m_selstart = m_cursor;
		m_selend = m_selstart + event->replacementLength();
		removeSelectedText();
	}
	if(!event->commitString().isEmpty()) {
		internalInsert(event->commitString());
		cursorPositionChanged = true;
	} else {
		m_cursor = qBound(0, c, m_text.length());
	}

	for(int i = 0; i < event->attributes().size(); ++i) {
		const QInputMethodEvent::Attribute &a = event->attributes().at(i);
		if(a.type == QInputMethodEvent::Selection) {
			m_cursor = qBound(0, a.start + a.length, m_text.length());
			if(a.length) {
				m_selstart = qMax(0, qMin(a.start, m_text.length()));
				m_selend = m_cursor;
				if(m_selend < m_selstart) {
					qSwap(m_selstart, m_selend);
				}
				selectionChange = true;
			} else {
				if(m_selstart != m_selend)
					selectionChange = true;
				m_selstart = m_selend = 0;
			}
			cursorPositionChanged = true;
		}
	}
#ifndef QT_NO_IM
	setPreeditArea(m_cursor, event->preeditString());
#endif //QT_NO_IM
	const int oldPreeditCursor = m_preeditCursor;
	m_preeditCursor = event->preeditString().length();
	m_hideCursor = false;
	QVector<QTextLayout::FormatRange> formats;
	formats.reserve(event->attributes().size());
	for(int i = 0; i < event->attributes().size(); ++i) {
		const QInputMethodEvent::Attribute &a = event->attributes().at(i);
		if(a.type == QInputMethodEvent::Cursor) {
			m_preeditCursor = a.start;
			m_hideCursor = !a.length;
		} else if(a.type == QInputMethodEvent::TextFormat) {
			QTextCharFormat f = qvariant_cast<QTextFormat>(a.value).toCharFormat();
			if(f.isValid()) {
				QTextLayout::FormatRange o;
				o.start = a.start + m_cursor;
				o.length = a.length;
				o.format = f;
				formats.append(o);
			}
		}
	}
	m_textLayout.setFormats(formats);
	updateDisplayText(true);
	if(cursorPositionChanged)
		emitCursorPositionChanged();
	else if(m_preeditCursor != oldPreeditCursor)
		emit updateMicroFocus();

	if(isGettingInput)
		finishChange(priorState);

	if(selectionChange)
		emit selectionChanged();
		*/
}

void
RichDocumentEditor::draw(QPainter *painter, const QPoint &offset, const QRect &clip, int flags, int cursorPos)
{
	QVector<QTextLayout::FormatRange> selections;
	if(flags & DrawSelections) {
		const int ss = m_textCursor->selectionStart();
		const int se = m_textCursor->selectionEnd();
		QTextLayout::FormatRange o;
		if(ss < se) {
			o.start = ss;
			o.length = se - ss;
			o.format.setBackground(m_palette.brush(QPalette::Highlight));
			o.format.setForeground(m_palette.brush(QPalette::HighlightedText));
		} else {
			// mask selection
			if(m_blinkStatus){
				o.start = m_textCursor->position();
				o.length = 1;
				o.format.setBackground(m_palette.brush(QPalette::Text));
				o.format.setForeground(m_palette.brush(QPalette::Window));
			}
		}
		selections.append(o);
	}

	if(flags & DrawText)
		textLayout()->draw(painter, offset, selections, clip);

	if(flags & DrawCursor){
		int cursor = cursorPos >= 0 ? cursorPos : m_textCursor->position();
		if(m_blinkStatus)
			textLayout()->drawCursor(painter, offset, cursor, m_cursorWidth);
	}
}

void
RichDocumentEditor::selectAll()
{
	if(!m_textCursor)
		return;
	m_textCursor->select(QTextCursor::Document);
	emit selectionChanged();
	emitCursorPositionChanged();
}

void
RichDocumentEditor::selectWordAtPos(int cursor)
{
	const int n = cursor - m_textCursor->position();
	m_textCursor->movePosition(n > 0 ? QTextCursor::NextCharacter : QTextCursor::PreviousCharacter,
							   QTextCursor::MoveAnchor, qAbs(n));
	m_textCursor->select(QTextCursor::WordUnderCursor);

	emit selectionChanged();
	emitCursorPositionChanged();
}

bool
RichDocumentEditor::finishChange(bool edited)
{
	if(m_textDirty) {
		updateDisplayText();

		if(m_textDirty) {
			m_textDirty = false;
			const QString actualText = text();
			if(edited)
				emit textEdited(actualText);
			emit textChanged(actualText);
		}
	}
	if(m_selDirty) {
		m_selDirty = false;
		emit selectionChanged();
	}
	if(m_textCursor->position() == m_lastCursorPos)
		updateMicroFocus();
	emitCursorPositionChanged();
	return true;
}

void
RichDocumentEditor::internalDelete(bool wasBackspace)
{
	if(!m_document)
		return;

#ifndef QT_NO_ACCESSIBILITY
	const int pos = m_textCursor->position();
	const QChar ch = m_document->characterAt(pos);
	QAccessibleTextRemoveEvent event(accessibleObject(), pos, ch);
	QAccessible::updateAccessibility(&event);
#endif
	if(wasBackspace)
		m_textCursor->deletePreviousChar();
	else
		m_textCursor->deleteChar();
	m_textDirty = true;
}

void
RichDocumentEditor::removeSelectedText()
{
	if(!m_textCursor->hasSelection())
		return;

#ifndef QT_NO_ACCESSIBILITY
	QAccessibleTextRemoveEvent event(accessibleObject(), m_textCursor->selectionStart(), m_textCursor->selectedText());
	QAccessible::updateAccessibility(&event);
#endif
	m_textCursor->removeSelectedText();
	internalDeselect();
	m_textDirty = true;
}

QTextCharFormat
RichDocumentEditor::styleAtPosition(int pos) const
{
	if(!m_document)
		return QTextCharFormat();

	const QTextBlock &blk = m_textCursor->block();
	if(!blk.isValid())
		return QTextCharFormat();

	int offset = pos - blk.position();
	for(QTextBlock::iterator it = blk.begin(); !it.atEnd(); ++it) {
		if(!it.fragment().isValid())
			continue;
		const int len = it.fragment().length();
		if(offset >= len) {
			offset -= len;
			continue;
		}
		return it.fragment().charFormat();
	}

	return QTextCharFormat();
}

void
RichDocumentEditor::toggleBold()
{
	if(!m_document)
		return;

	QTextCharFormat fmt;
	fmt.setFontWeight(styleAtPosition(m_textCursor->position()).fontWeight() == QFont::Bold ? QFont::Normal : QFont::Bold);
	m_textCursor->mergeCharFormat(fmt);
	m_textDirty = true;
	finishChange(true);
}

void
RichDocumentEditor::toggleItalic()
{
	if(!m_document)
		return;

	QTextCharFormat fmt;
	fmt.setFontItalic(!styleAtPosition(m_textCursor->position()).fontItalic());
	m_textCursor->mergeCharFormat(fmt);
	m_textDirty = true;
	finishChange(true);
}

void
RichDocumentEditor::toggleUnderline()
{
	if(!m_document)
		return;

	QTextCharFormat fmt;
	fmt.setFontUnderline(!styleAtPosition(m_textCursor->position()).fontUnderline());
	m_textCursor->mergeCharFormat(fmt);
	m_textDirty = true;
	finishChange(true);
}

void
RichDocumentEditor::toggleStrikeOut()
{
	if(!m_document)
		return;

	QTextCharFormat fmt;
	fmt.setFontStrikeOut(!styleAtPosition(m_textCursor->position()).fontStrikeOut());
	m_textCursor->mergeCharFormat(fmt);
	m_textDirty = true;
	finishChange(true);
}

const QColor
RichDocumentEditor::textColor() const
{
	const static QColor def(0, 0, 0, 0);
	if(!m_document)
		return def;

	QBrush fg = styleAtPosition(m_textCursor->position()).foreground();
	return fg.style() == Qt::NoBrush ? def : fg.color();
}

void
RichDocumentEditor::setTextColor(const QColor &color)
{
	if(!m_document)
		return;

	QTextCharFormat fmt;
	fmt.setForeground(color.rgba() == 0 ? QBrush(Qt::NoBrush) : QBrush(color));
	m_textCursor->mergeCharFormat(fmt);
	m_textDirty = true;
	finishChange(true);
}

void
RichDocumentEditor::emitCursorPositionChanged()
{
	const int curPos = m_textCursor->position();
	if(curPos != m_lastCursorPos) {
		const int oldLast = m_lastCursorPos;
		m_lastCursorPos = curPos;
		cursorPositionChanged(oldLast, curPos);
#ifndef QT_NO_ACCESSIBILITY
		// otherwise we send a selection update which includes the cursor
		if(!hasSelection()) {
			QAccessibleTextCursorEvent event(accessibleObject(), curPos);
			QAccessible::updateAccessibility(&event);
		}
#endif
	}
}

#if QT_CONFIG(completer)
bool
RichDocumentEditor::advanceToEnabledItem(int dir)
{
	int start = m_completer->currentRow();
	if(start == -1)
		return false;
	int i = start + dir;
	if(dir == 0) dir = 1;
	do {
		if(!m_completer->setCurrentRow(i)) {
			if(!m_completer->wrapAround())
				break;
			i = i > 0 ? 0 : m_completer->completionCount() - 1;
		} else {
			QModelIndex currentIndex = m_completer->currentIndex();
			if(m_completer->completionModel()->flags(currentIndex) & Qt::ItemIsEnabled)
				return true;
			i += dir;
		}
	} while(i != start);

	m_completer->setCurrentRow(start); // restore
	return false;
}

void
RichDocumentEditor::complete(int key)
{
	if(!m_completer || isReadOnly())
		return;

	const QString &text = m_document->toPlainText();
	if(m_completer->completionMode() == QCompleter::InlineCompletion) {
		if(key == Qt::Key_Backspace)
			return;
		int n = 0;
		if(key == Qt::Key_Up || key == Qt::Key_Down) {
			if(m_textCursor->selectionEnd() == text.length())
				return;
			const QStringRef prefix = hasSelection() ? text.leftRef(m_textCursor->selectionStart()) : QStringRef(&text);
			if(text.compare(m_completer->currentCompletion(), m_completer->caseSensitivity()) != 0
				|| prefix.compare(m_completer->completionPrefix(), m_completer->caseSensitivity()) != 0) {
				m_completer->setCompletionPrefix(prefix.toString());
			} else {
				n = (key == Qt::Key_Up) ? -1 : +1;
			}
		} else {
			m_completer->setCompletionPrefix(text);
		}
		if(!advanceToEnabledItem(n))
			return;
	} else {
		if(text.isEmpty()) {
			if(auto *popup = m_completer->popup())
				popup->hide();
			return;
		}
		m_completer->setCompletionPrefix(text);
	}

	m_completer->complete();
}
#endif

void
RichDocumentEditor::setReadOnly(bool enable)
{
	if(m_readOnly == enable)
		return;

	m_readOnly = enable;
	updateCursorBlinking();
}

void
RichDocumentEditor::setBlinkingCursorEnabled(bool enable)
{
	if(m_blinkEnabled == enable)
		return;

	m_blinkEnabled = enable;

	if(enable)
		connect(QGuiApplication::styleHints(), &QStyleHints::cursorFlashTimeChanged, this, &RichDocumentEditor::updateCursorBlinking);
	else
		disconnect(QGuiApplication::styleHints(), &QStyleHints::cursorFlashTimeChanged, this, &RichDocumentEditor::updateCursorBlinking);

	updateCursorBlinking();
}

void
RichDocumentEditor::updateCursorBlinking()
{
	if(m_blinkTimer) {
		killTimer(m_blinkTimer);
		m_blinkTimer = 0;
	}

	if(m_blinkEnabled && !m_readOnly) {
		int flashTime = QGuiApplication::styleHints()->cursorFlashTime();
		if(flashTime >= 2)
			m_blinkTimer = startTimer(flashTime / 2);
	}

	m_blinkStatus = 1;
	emit updateNeeded(cursorRect());
}

void
RichDocumentEditor::timerEvent(QTimerEvent *event)
{
	if(event->timerId() == m_blinkTimer) {
		m_blinkStatus = !m_blinkStatus;
		emit updateNeeded(cursorRect());
	}
}

#ifndef QT_NO_SHORTCUT
void
RichDocumentEditor::processShortcutOverrideEvent(QKeyEvent *ke)
{
	if(ke == QKeySequence::Copy
		|| ke == QKeySequence::MoveToNextWord
		|| ke == QKeySequence::MoveToPreviousWord
		|| ke == QKeySequence::MoveToStartOfLine
		|| ke == QKeySequence::MoveToEndOfLine
		|| ke == QKeySequence::MoveToStartOfDocument
		|| ke == QKeySequence::MoveToEndOfDocument
		|| ke == QKeySequence::SelectNextWord
		|| ke == QKeySequence::SelectPreviousWord
		|| ke == QKeySequence::SelectStartOfLine
		|| ke == QKeySequence::SelectEndOfLine
		|| ke == QKeySequence::SelectStartOfBlock
		|| ke == QKeySequence::SelectEndOfBlock
		|| ke == QKeySequence::SelectStartOfDocument
		|| ke == QKeySequence::SelectAll
		|| ke == QKeySequence::SelectEndOfDocument) {
		ke->accept();
	} else if(ke == QKeySequence::Paste
			   || ke == QKeySequence::Cut
			   || ke == QKeySequence::Redo
			   || ke == QKeySequence::Undo
			   || ke == QKeySequence::DeleteCompleteLine) {
		if(!isReadOnly())
			ke->accept();
	} else if(ke->modifiers() == Qt::NoModifier || ke->modifiers() == Qt::ShiftModifier
			   || ke->modifiers() == Qt::KeypadModifier) {
		if(ke->key() < Qt::Key_Escape) {
			if(!isReadOnly())
				ke->accept();
		} else {
			switch(ke->key()) {
			case Qt::Key_Delete:
			case Qt::Key_Backspace:
				if(!isReadOnly())
					ke->accept();
				break;

			case Qt::Key_Home:
			case Qt::Key_End:
			case Qt::Key_Left:
			case Qt::Key_Right:
				ke->accept();
				break;

			default:
				break;
			}
		}
	}
}
#endif

void
RichDocumentEditor::processKeyEvent(QKeyEvent* event)
{
	bool inlineCompletionAccepted = false;

#if QT_CONFIG(completer)
	if(m_completer) {
		QCompleter::CompletionMode completionMode = m_completer->completionMode();
		auto *popup = m_completer->popup();
		if((completionMode == QCompleter::PopupCompletion || completionMode == QCompleter::UnfilteredPopupCompletion) && popup && popup->isVisible()) {
			// The following keys are forwarded by the completer to the widget
			// Ignoring the events lets the completer provide suitable default behavior
			switch(event->key()) {
			case Qt::Key_Escape:
				event->ignore();
				return;
			default:
				break; // normal key processing
			}
		} else if(completionMode == QCompleter::InlineCompletion) {
			switch(event->key()) {
			case Qt::Key_Enter:
			case Qt::Key_Return:
			case Qt::Key_F4:
				if(!m_completer->currentCompletion().isEmpty() && hasSelection() && m_textCursor->selectionEnd() == m_document->length()) {
					m_document->setPlainText(m_completer->currentCompletion());
					inlineCompletionAccepted = true;
				}
			default:
				break; // normal key processing
			}
		}
	}
#endif // QT_CONFIG(completer)

	if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
		QGuiApplication::inputMethod()->commit();
		if(inlineCompletionAccepted) {
			emit accepted();
			emit editingFinished();
		} else {
			insert(event->text(), -1, Plain);
		}
		event->accept();
		return;
	}

	bool unknown = false;
#if QT_CONFIG(shortcut)
	bool visual = cursorMoveStyle() == Qt::VisualMoveStyle;
#endif

	if(false) {
	}
#ifndef QT_NO_SHORTCUT
	else if(event == QKeySequence::SelectAll) {
		selectAll();
	}
#ifndef QT_NO_CLIPBOARD
	else if(event == QKeySequence::Copy) {
		copy();
	}
	else if(event == QKeySequence::Paste) {
		if(!isReadOnly()) {
			QClipboard::Mode mode = QClipboard::Clipboard;
			if(m_keyboardScheme == QPlatformTheme::X11KeyboardScheme
				&& event->modifiers() == (Qt::CTRL | Qt::SHIFT)
				&& event->key() == Qt::Key_Insert) {
				mode = QClipboard::Selection;
			}
			paste(mode);
		}
	}
	else if(event == QKeySequence::Cut) {
		if(!isReadOnly() && hasSelection()) {
			copy();
			eraseSelectedText();
		}
	}
	else if(event == QKeySequence::DeleteEndOfLine) {
		if(!isReadOnly()) {
			setSelection(cursor(), m_document->length());
			copy();
			eraseSelectedText();
		}
	}
#endif //QT_NO_CLIPBOARD
	else if(event == QKeySequence::MoveToStartOfLine || event == QKeySequence::MoveToStartOfBlock) {
		home(0);
	}
	else if(event == QKeySequence::MoveToEndOfLine || event == QKeySequence::MoveToEndOfBlock) {
		end(0);
	}
	else if(event == QKeySequence::SelectStartOfLine || event == QKeySequence::SelectStartOfBlock) {
		home(1);
	}
	else if(event == QKeySequence::SelectEndOfLine || event == QKeySequence::SelectEndOfBlock) {
		end(1);
	}
	else if(event == QKeySequence::MoveToNextChar) {
#if !QT_CONFIG(completer)
		const bool inlineCompletion = false;
#else
		const bool inlineCompletion = m_completer && m_completer->completionMode() == QCompleter::InlineCompletion;
#endif
		if(hasSelection() && (m_keyboardScheme != QPlatformTheme::WindowsKeyboardScheme || inlineCompletion))
			cursorSetPosition(m_textCursor->selectionEnd(), false);
		else
			cursorMovePosition(visual ? 1 : (layoutDirection() == Qt::LeftToRight ? 1 : -1), false);
	}
	else if(event == QKeySequence::SelectNextChar) {
		cursorMovePosition(visual ? 1 : (layoutDirection() == Qt::LeftToRight ? 1 : -1), true);
	}
	else if(event == QKeySequence::MoveToPreviousChar) {
#if !QT_CONFIG(completer)
		const bool inlineCompletion = false;
#else
		const bool inlineCompletion = m_completer && m_completer->completionMode() == QCompleter::InlineCompletion;
#endif
		if(hasSelection() && (m_keyboardScheme != QPlatformTheme::WindowsKeyboardScheme || inlineCompletion))
			cursorSetPosition(m_textCursor->selectionStart(), false);
		else
			cursorMovePosition(visual ? -1 : (layoutDirection() == Qt::LeftToRight ? -1 : 1), false);
	}
	else if(event == QKeySequence::SelectPreviousChar) {
		cursorMovePosition(visual ? -1 : (layoutDirection() == Qt::LeftToRight ? -1 : 1), true);
	}
	else if(event == QKeySequence::MoveToNextWord) {
		layoutDirection() == Qt::LeftToRight ? cursorWordForward(false) : cursorWordBackward(false);
	}
	else if(event == QKeySequence::MoveToPreviousWord) {
		layoutDirection() == Qt::LeftToRight ? cursorWordBackward(false) : cursorWordForward(false);
	}
	else if(event == QKeySequence::SelectNextWord) {
		layoutDirection() == Qt::LeftToRight ? cursorWordForward(true) : cursorWordBackward(true);
	}
	else if(event == QKeySequence::SelectPreviousWord) {
		layoutDirection() == Qt::LeftToRight ? cursorWordBackward(true) : cursorWordForward(true);
	}
	else if(event == QKeySequence::Delete) {
		if(!isReadOnly())
			del();
	}
	else if(event == QKeySequence::DeleteEndOfWord) {
		if(!isReadOnly()) {
			cursorWordForward(true);
			eraseSelectedText();
		}
	}
	else if(event == QKeySequence::DeleteStartOfWord) {
		if(!isReadOnly()) {
			cursorWordBackward(true);
			eraseSelectedText();
		}
	} else if(event == QKeySequence::DeleteCompleteLine) {
		if(!isReadOnly()) {
			setSelection(m_textCursor->block().position(), m_textCursor->block().length());
#ifndef QT_NO_CLIPBOARD
			copy();
#endif
			eraseSelectedText();
		}
	}
#endif // QT_NO_SHORTCUT
	else {
		bool handled = false;
		if(m_keyboardScheme == QPlatformTheme::MacKeyboardScheme && (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down)) {
			Qt::KeyboardModifiers kmods = (event->modifiers() & ~Qt::KeypadModifier);
			const bool mark = kmods & Qt::ShiftModifier;
			kmods &= ~Qt::ShiftModifier;
			if((kmods == Qt::ControlModifier || kmods == Qt::AltModifier || kmods == Qt::NoModifier))
				event->key() == Qt::Key_Up ? home(mark) : end(mark);
			handled = true;
		}
		if(event->modifiers() & Qt::ControlModifier) {
			switch(event->key()) {
			case Qt::Key_Backspace:
				if(!isReadOnly()) {
					cursorWordBackward(true);
					eraseSelectedText();
				}
				break;
#if QT_CONFIG(completer)
			case Qt::Key_Up:
			case Qt::Key_Down:
				complete(event->key());
				break;
#endif
			default:
				if(!handled)
					unknown = true;
			}
		} else { // ### check for *no* modifier
			switch(event->key()) {
			case Qt::Key_Backspace:
				if(!isReadOnly()) {
					backspace();
#if QT_CONFIG(completer)
					complete(Qt::Key_Backspace);
#endif
				}
				break;
			default:
				if(!handled)
					unknown = true;
			}
		}
	}

	if(event->key() == Qt::Key_Direction_L || event->key() == Qt::Key_Direction_R) {
		setLayoutDirection((event->key() == Qt::Key_Direction_L) ? Qt::LeftToRight : Qt::RightToLeft);
		unknown = false;
	}

	if(unknown && !isReadOnly() && isAcceptableInput(event)) {
		insert(event->text(), -1, Plain);
#if QT_CONFIG(completer)
		complete(event->key());
#endif
		event->accept();
		return;
	}

	if(unknown)
		event->ignore();
	else
		event->accept();
}

bool
RichDocumentEditor::isAcceptableInput(const QKeyEvent *event) const
{
	const QString text = event->text();
	if(text.isEmpty())
		return false;

	const QChar c = text.at(0);

	// Formatting characters such as ZWNJ, ZWJ, RLM, etc. This needs to go before the
	// next test, since CTRL+SHIFT is sometimes used to input it on Windows.
	if(c.category() == QChar::Other_Format)
		return true;

	// QTBUG-35734: ignore Ctrl/Ctrl+Shift; accept only AltGr (Alt+Ctrl) on German keyboards
	if(event->modifiers() == Qt::ControlModifier
			|| event->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier)) {
		return false;
	}

	if(c.isPrint())
		return true;

	if(c.category() == QChar::Other_PrivateUse)
		return true;

#if HANDLE_TAB_KEY
	if(c == QLatin1Char('\t'))
		return true;
#endif

	return false;
}

bool
RichDocumentEditor::isCommonTextEditShortcut(const QKeyEvent *ke)
{
	if(ke->modifiers() == Qt::NoModifier
		|| ke->modifiers() == Qt::ShiftModifier
		|| ke->modifiers() == Qt::KeypadModifier) {
		if(ke->key() < Qt::Key_Escape) {
			return true;
		} else {
			switch(ke->key()) {
				case Qt::Key_Return:
				case Qt::Key_Enter:
				case Qt::Key_Delete:
				case Qt::Key_Home:
				case Qt::Key_End:
				case Qt::Key_Backspace:
				case Qt::Key_Left:
				case Qt::Key_Right:
				case Qt::Key_Up:
				case Qt::Key_Down:
				case Qt::Key_Tab:
				return true;
			default:
				break;
			}
		}
#if QT_CONFIG(shortcut)
	} else if(ke->matches(QKeySequence::Copy)
			   || ke->matches(QKeySequence::Paste)
			   || ke->matches(QKeySequence::Cut)
			   || ke->matches(QKeySequence::Redo)
			   || ke->matches(QKeySequence::Undo)
			   || ke->matches(QKeySequence::MoveToNextWord)
			   || ke->matches(QKeySequence::MoveToPreviousWord)
			   || ke->matches(QKeySequence::MoveToStartOfDocument)
			   || ke->matches(QKeySequence::MoveToEndOfDocument)
			   || ke->matches(QKeySequence::SelectNextWord)
			   || ke->matches(QKeySequence::SelectPreviousWord)
			   || ke->matches(QKeySequence::SelectStartOfLine)
			   || ke->matches(QKeySequence::SelectEndOfLine)
			   || ke->matches(QKeySequence::SelectStartOfBlock)
			   || ke->matches(QKeySequence::SelectEndOfBlock)
			   || ke->matches(QKeySequence::SelectStartOfDocument)
			   || ke->matches(QKeySequence::SelectEndOfDocument)
			   || ke->matches(QKeySequence::SelectAll)
			  ) {
		return true;
#endif
	}
	return false;
}

void
RichDocumentEditor::setDocument(RichDocument *doc)
{
#ifndef QT_NO_IM
	if(composeMode())
		QGuiApplication::inputMethod()->reset();
#endif
	m_document = doc;
	if(m_document) {
		m_textCursor = m_document->undoableCursor();
		m_textCursor->movePosition(QTextCursor::Start);
	} else {
		m_textCursor = nullptr;
	}
}
