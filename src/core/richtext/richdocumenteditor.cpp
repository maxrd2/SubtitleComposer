/*
    SPDX-FileCopyrightText: 2016 The Qt Company Ltd.
    SPDX-FileCopyrightText: 2020-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "richdocumenteditor.h"

#include <QAbstractItemView>
#include <QAccessible>
#include <QApplication>
#include <QClipboard>
#include <QGraphicsSceneEvent>
#include <QMimeData>
#include <QPainter>
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

void
RichDocumentEditor::updateDisplayText(bool forceUpdate)
{
	QString orig = text();
	QString newText;

	delete[] m_textLayouts;
	m_textLayouts = nullptr;
	m_layoutCount = 0;

	m_ascent = 0;
	m_layoutWidth = 0.;
	m_layoutHeight = 0.;

	if(m_document) {
		const RichDocumentLayout *docLayout = m_document->documentLayout();
		m_textLayouts = new QTextLayout[m_document->blockCount()];
		for(QTextBlock bi = m_document->begin(); bi != m_document->end(); bi = bi.next()) {
			const QTextLayout *bl = bi.layout();
			QTextLayout *layout = &m_textLayouts[m_layoutCount++];
			layout->setCacheEnabled(true);

			QTextOption option = bl->textOption();
			option.setTextDirection(m_layoutDirection);
			option.setFlags(QTextOption::IncludeTrailingSpaces);
			option.setWrapMode(QTextOption::NoWrap);
			layout->setTextOption(option);

			layout->setFont(m_layoutFont);

			QString text = bi.text() + QChar(QChar::LineSeparator);
			newText.push_back(text + QChar(QChar::LineSeparator));
			// replace certain non-printable characters with spaces (to avoid drawing boxes
			// when using fonts that don't have glyphs for such characters)
			QChar *uc = text.data();
			for(int i = 0; i < (int)text.length(); ++i) {
				if((uc[i].unicode() < 0x20 && uc[i].unicode() != 0x09)
				|| uc[i] == QChar::LineSeparator
				|| uc[i] == QChar::ParagraphSeparator
				|| uc[i] == QChar::ObjectReplacementCharacter)
					uc[i] = QChar(QChar::Space);
			}
			layout->setText(text);

			QVector<QTextLayout::FormatRange> ranges = m_preeditRanges;
			ranges.append(docLayout->applyCSS(bl->formats()));
			layout->setFormats(ranges);

			layout->beginLayout();
			for(;;) {
				QTextLine line = layout->createLine();
				if(!line.isValid())
					break;
				line.setLineWidth(1e9);
				line.setPosition(QPointF(m_layoutWidth, 0.));
				const int w = line.naturalTextWidth();
				m_layoutWidth += w + m_layoutSeparatorSize.width();
				line.setLineWidth(w);
				m_ascent = qMax(m_ascent, qRound(line.ascent()));
				m_layoutHeight = qMax(m_layoutHeight, line.leading() + line.height());
			}
			layout->endLayout();
		}
		if(m_layoutWidth >= m_layoutSeparatorSize.width())
			m_layoutWidth -= m_layoutSeparatorSize.width();
	}

	if(newText != orig || forceUpdate)
		emit displayTextChanged(newText);
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
	for(int i = 0; i < m_layoutCount; i++)
		m_textLayouts[i].clearFormats();
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
	int off = 0;
	for(int l = 0; l < m_layoutCount; l++) {
		const QTextLine &line = m_textLayouts[l].lineAt(0);
		QRectF lr = line.rect();
		lr.setWidth(lr.width() + m_layoutSeparatorSize.width());
		if(x < lr.right() || l == m_layoutCount - 1)
			return off + line.xToCursor(x, betweenOrOn);
		off += m_textLayouts[l].text().length();
	}
	return 0;
}

QTextLayout *
RichDocumentEditor::cursorToLayout(int *cursorPosition) const
{
	if(!m_layoutCount)
		return nullptr;
	const QTextLayout *last = m_textLayouts + (m_layoutCount - 1);
	QTextLayout *layout = m_textLayouts;
	for(;;) {
		const int len = layout->text().length();
		if(*cursorPosition <= len || layout == last)
			break;
		*cursorPosition -= len;
		layout++;
	}
	return layout;
}

qreal
RichDocumentEditor::cursorToX(int cursor) const
{
	const QTextLayout *layout = cursorToLayout(&cursor);
	if(!layout)
		return 0.;
	return layout->lineAt(0).cursorToX(cursor);
}

QRect
RichDocumentEditor::rectForPos(int pos) const
{
	const QTextLayout *layout = cursorToLayout(&pos);
	if(!layout)
		return QRect();

	QTextLine l = layout->lineForTextPosition(pos);
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
		internalDeselect();
		m_textCursor->movePosition(oper, QTextCursor::MoveAnchor, n);
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
	bool didChange = false;
	const bool isGettingInput = !event->commitString().isEmpty()
			|| event->preeditString() != preeditAreaText()
			|| event->replacementLength() > 0;

	if(isGettingInput) {
		// If any text is being input, remove selected text.
		if(m_textCursor->hasSelection()) {
			removeSelectedText();
			didChange = true;
		}
	}

	if(event->replacementStart()) {
		if(event->replacementStart() < 0)
			m_textCursor->movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, -event->replacementStart());
		else
			m_textCursor->movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, event->replacementStart());
	}

	// insert commit string
	if(event->replacementLength()) {
		m_textCursor->movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, event->replacementLength());
		removeSelectedText();
		didChange = true;
	}
	if(!event->commitString().isEmpty()) {
		m_textCursor->insertText(event->commitString());
	}

	// update selection
	for(int i = 0; i < event->attributes().size(); ++i) {
		const QInputMethodEvent::Attribute &a = event->attributes().at(i);
		if(a.type == QInputMethodEvent::Selection) {
			const int relMove = a.start - m_textCursor->position();
			m_textCursor->movePosition(relMove > 0 ? QTextCursor::NextCharacter : QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, relMove);
			m_textCursor->movePosition(a.length > 0 ? QTextCursor::NextCharacter : QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, a.length);
			if(a.length)
				m_selDirty = true;
		}
	}

	// update preedit text
	const int oldPreeditCursor = preeditAreaPosition();
#ifndef QT_NO_IM
	setPreeditArea(m_textCursor->position(), event->preeditString());
#endif //QT_NO_IM
	int preeditCursor = event->preeditString().length();
	m_preeditRanges.clear();
	m_preeditRanges.reserve(event->attributes().size());
	for(int i = 0; i < event->attributes().size(); ++i) {
		const QInputMethodEvent::Attribute &a = event->attributes().at(i);
		if(a.type == QInputMethodEvent::Cursor) {
			preeditCursor = a.start;
		} else if(a.type == QInputMethodEvent::TextFormat) {
			QTextCharFormat f = qvariant_cast<QTextFormat>(a.value).toCharFormat();
			if(f.isValid()) {
				QTextLayout::FormatRange o;
				o.start = a.start + m_textCursor->position();
				o.length = a.length;
				o.format = f;
				m_preeditRanges.append(o);
			}
		}
	}

	// finish up
	if(isGettingInput)
		m_textDirty = true;
	if(!m_selDirty && preeditCursor != oldPreeditCursor)
		emit updateMicroFocus();
	finishChange(didChange, true);
}

void
RichDocumentEditor::draw(QPainter *painter, const QPoint &offset, const QRect &clip, int flags, int cursorPos)
{
	painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

	QTextLayout::FormatRange sr;
	if(flags & DrawSelections && flags & DrawText) {
		const int ss = m_textCursor->selectionStart();
		const int se = m_textCursor->selectionEnd();
		if(ss < se) {
			sr.start = ss;
			sr.length = se;
			sr.format.setBackground(m_palette.brush(QPalette::Highlight));
			sr.format.setForeground(m_palette.brush(QPalette::HighlightedText));
		} else {
			// mask selection
			if(m_blinkStatus) {
				sr.start = m_textCursor->position();
				sr.length = sr.start + 1;
				sr.format.setBackground(m_palette.brush(QPalette::Text));
				sr.format.setForeground(m_palette.brush(QPalette::Window));
			}
		}
	}

	if(flags & (DrawText | DrawCursor)) {
		int cursor = cursorPos >= 0 ? cursorPos : m_textCursor->position();
		int off = 0;

		RichDocumentLayout *docLayout = m_document->documentLayout();
		docLayout->separatorResize(m_layoutSeparatorSize);

		painter->setPen(palette().color(QPalette::Normal, QPalette::Text));

		for(int i = 0; i < m_layoutCount; i++) {
			const int end = off + m_textLayouts[i].text().length();
			if(flags & DrawText) {
				QVector<QTextLayout::FormatRange> selections;
				if(flags & DrawSelections && off < sr.length && end > sr.start) {
					const int s = qMax(sr.start, off);
					const int l = qMin(sr.length, end) - s;
					selections.push_back(QTextLayout::FormatRange{s - off, l, sr.format});
				}
				m_textLayouts[i].draw(painter, offset, selections, clip);
				const QTextLine &tl = m_textLayouts[i].lineAt(0);
				docLayout->separatorDraw(painter, QPointF(tl.position().x() + offset.x() - m_layoutSeparatorSize.width(), offset.y() - tl.descent()));
			}
			if(flags & DrawCursor && m_blinkStatus && cursor >= off && cursor < end)
				m_textLayouts[i].drawCursor(painter, offset, cursor - off, m_cursorWidth);
			off = end;
		}
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
RichDocumentEditor::finishChange(bool edited, bool forceUpdate)
{
	if(m_textDirty) {
		updateDisplayText(forceUpdate);

		if(m_textDirty) {
			m_textDirty = false;
			const QString actualText = text();
			if(edited)
				emit textEdited(actualText);
			emit textChanged(actualText);
		}
	} else if(forceUpdate) {
		updateDisplayText(forceUpdate);
	}
	if(m_selDirty) {
		m_selDirty = false;
		emit selectionChanged();
	}
	if(m_textCursor->position() == m_lastCursorPos)
		emit updateMicroFocus();
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
		emit cursorPositionChanged(oldLast, curPos);
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
			const QStringView prefix = hasSelection() ? QStringView(text).mid(0, m_textCursor->selectionStart()) : QStringView(text);
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

QString
RichDocumentEditor::text() const
{
	QString text;
	for(int i = 0; i < m_layoutCount; i++) {
		if(i)
			text.push_back(QChar::LineSeparator);
		text.push_back(m_textLayouts[i].text());
	}
	return text;
}

#ifndef QT_NO_IM
void
RichDocumentEditor::setPreeditArea(int cursor, const QString &text)
{
	int off = 0;
	for(int i = 0; i < m_layoutCount; i++) {
		if(cursor == -1) {
			m_textLayouts[i].setPreeditArea(-1, text);
		} else if(cursor <= off) {
			m_textLayouts[i].setPreeditArea(cursor - off, text);
			break;
		}
		off += m_textLayouts[i].text().length();
	}
}
#endif

int
RichDocumentEditor::preeditAreaPosition() const
{
	if(m_textCursor) {
		int pos = m_textCursor->position();
		const QTextLayout *layout = cursorToLayout(&pos);
		if(layout)
			return layout->preeditAreaPosition();
	}
	return -1;
}

QString
RichDocumentEditor::preeditAreaText() const
{
	if(m_textCursor) {
		int pos = m_textCursor->position();
		const QTextLayout *layout = cursorToLayout(&pos);
		if(layout)
			return layout->preeditAreaText();
	}
	return QString();
}

void
RichDocumentEditor::setFont(const QFont &font)
{
	m_layoutFont = font;
	updateDisplayText();
}

