/*
    SPDX-FileCopyrightText: 2020-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "richlineedit.h"

#include "appglobal.h"
#include "application.h"
#include "actions/useractionnames.h"
#include "core/richtext/richdocumenteditor.h"
#include "dialogs/subtitlecolordialog.h"

#include <QDrag>
#include <QMimeData>
#include <QPainter>
#include <QStyleHints>

#include <KLocalizedString>


using namespace SubtitleComposer;

RichLineEdit::RichLineEdit(const QStyleOptionViewItem &styleOption, QWidget *parent)
	: QWidget(parent),
	  m_lineStyle(styleOption),
	  m_control(new RichDocumentEditor())
{
	setupActions();

	setCursor(QCursor(Qt::IBeamCursor));

	m_control->setAccessibleObject(this);
	m_control->setCursorWidth(style()->pixelMetric(QStyle::PM_TextCursorWidth));
	m_control->setLineSeparatorSize(QSizeF(.5 * styleOption.rect.height(), styleOption.rect.height()));

	setFocusPolicy(Qt::StrongFocus);
	setAttribute(Qt::WA_InputMethodEnabled);
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed, QSizePolicy::LineEdit));
	setBackgroundRole(QPalette::Base);
	setAttribute(Qt::WA_KeyCompression);
	setMouseTracking(true);
	setAcceptDrops(true);

	setAttribute(Qt::WA_MacShowFocusRect);

#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
	m_mouseYThreshold = 0;
#else
	m_mouseYThreshold = QGuiApplication::styleHints()->mouseQuickSelectionThreshold();
#endif

	connect(m_control, &RichDocumentEditor::cursorPositionChanged, this, QOverload<>::of(&RichLineEdit::update));
	connect(m_control, &RichDocumentEditor::selectionChanged, this, QOverload<>::of(&RichLineEdit::update));
	connect(m_control, &RichDocumentEditor::displayTextChanged, this, QOverload<>::of(&RichLineEdit::update));
	connect(m_control, &RichDocumentEditor::updateNeeded, this, [this](QRect r){ r.setBottom(rect().bottom()); update(r); });
	update();
}

RichLineEdit::~RichLineEdit()
{
	delete m_control;
}

static void
setupActionCommon(QAction *act, const char *appActionId)
{
	QAction *appAction = qobject_cast<QAction *>(app()->action(appActionId));
	QObject::connect(appAction, &QAction::changed, act, [act, appAction](){ act->setShortcut(appAction->shortcut()); });
	act->setShortcuts(appAction->shortcuts());
}

void
RichLineEdit::setupActions()
{
	m_actions.push_back(app()->action(ACT_UNDO));
	m_actions.push_back(app()->action(ACT_REDO));

	QAction *act;
#ifndef QT_NO_CLIPBOARD
	act = new QAction(this);
	act->setIcon(QIcon::fromTheme("edit-cut"));
	act->setText(i18n("Cut"));
	act->setShortcuts(KStandardShortcut::cut());
	connect(act, &QAction::triggered, this, [this](){ m_control->cut(); });
	m_actions.push_back(act);

	act = new QAction(this);
	act->setIcon(QIcon::fromTheme("edit-copy"));
	act->setText(i18n("Copy"));
	act->setShortcuts(KStandardShortcut::copy());
	connect(act, &QAction::triggered, this, [this](){ m_control->copy(); });
	m_actions.push_back(act);

	act = new QAction(this);
	act->setIcon(QIcon::fromTheme("edit-paste"));
	act->setText(i18n("Paste"));
	act->setShortcuts(KStandardShortcut::paste());
	connect(act, &QAction::triggered, this, [this](){ m_control->paste(); });
	m_actions.push_back(act);
#endif

	act = new QAction(this);
	act->setIcon(QIcon::fromTheme("edit-clear"));
	act->setText(i18nc("@action:inmenu Clear all text", "Clear"));
	connect(act, &QAction::triggered, this, [this](){ m_control->clear(); });
	m_actions.push_back(act);

	act = new QAction(this);
	act->setIcon(QIcon::fromTheme("edit-select-all"));
	act->setText(i18n("Select All"));
	setupActionCommon(act, ACT_SELECT_ALL_LINES);
	connect(act, &QAction::triggered, this, [this](){ m_control->selectAll(); });
	m_actions.push_back(act);

	act = new QAction(this);
	act->setIcon(QIcon::fromTheme("format-text-bold"));
	act->setText(i18nc("@action:inmenu Toggle bold style", "Bold"));
	act->setCheckable(true);
	setupActionCommon(act, ACT_TOGGLE_SELECTED_LINES_BOLD);
	connect(act, &QAction::triggered, this, [this](){ m_control->toggleBold(); });
	m_actions.push_back(act);

	act = new QAction(this);
	act->setIcon(QIcon::fromTheme("format-text-italic"));
	act->setText(i18nc("@action:inmenu Toggle italic style", "Italic"));
	act->setCheckable(true);
	setupActionCommon(act, ACT_TOGGLE_SELECTED_LINES_ITALIC);
	connect(act, &QAction::triggered, this, [this](){ m_control->toggleItalic(); });
	m_actions.push_back(act);

	act = new QAction(this);
	act->setIcon(QIcon::fromTheme("format-text-underline"));
	act->setText(i18nc("@action:inmenu Toggle underline style", "Underline"));
	act->setCheckable(true);
	setupActionCommon(act, ACT_TOGGLE_SELECTED_LINES_UNDERLINE);
	connect(act, &QAction::triggered, this, [this](){ m_control->toggleUnderline(); });
	m_actions.push_back(act);

	act = new QAction(this);
	act->setIcon(QIcon::fromTheme("format-text-strikethrough"));
	act->setText(i18nc("@action:inmenu Toggle strike through style", "Strike Through"));
	act->setCheckable(true);
	setupActionCommon(act, ACT_TOGGLE_SELECTED_LINES_STRIKETHROUGH);
	connect(act, &QAction::triggered, this, [this](){ m_control->toggleStrikeOut(); });
	m_actions.push_back(act);

	act = new QAction(this);
	act->setIcon(QIcon::fromTheme("format-text-color"));
	act->setText(i18nc("@action:inmenu Change Text Color", "Text Color"));
	setupActionCommon(act, ACT_CHANGE_SELECTED_LINES_TEXT_COLOR);
	connect(act, &QAction::triggered, this, &RichLineEdit::changeTextColor);
	m_actions.push_back(act);
}

void
RichLineEdit::setDocument(RichDocument *document)
{
	m_document = document;
	m_control->setDocument(m_document);
	m_control->setFont(m_lineStyle.font);
	m_control->setLayoutDirection(m_lineStyle.direction);
}

void
RichLineEdit::changeTextColor()
{
	QColor color = SubtitleColorDialog::getColor(m_control->textColor(), this);
	if(!color.isValid())
		return;
	m_control->setTextColor(color);
}

bool
RichLineEdit::event(QEvent *e)
{
	if(e->type() == QEvent::ShortcutOverride) {
		QKeyEvent *ke = static_cast<QKeyEvent *>(e);
		const QKeySequence key(ke->modifiers() | ke->key());

		for(const QAction *act: m_actions) {
			if(act->shortcuts().contains(key)) {
				e->accept();
				return true;
			}
		}

		m_control->processShortcutOverrideEvent(ke);
	}

	return QWidget::event(e);
}

void
RichLineEdit::mousePressEvent(QMouseEvent* e)
{
	m_mousePressPos = e->pos();

	if(sendMouseEventToInputContext(e))
		return;
	if(e->button() == Qt::RightButton)
		return;
	if(m_tripleClickTimer.isActive() && (e->pos() - m_tripleClickPos).manhattanLength() < QApplication::startDragDistance()) {
		m_control->selectAll();
		return;
	}
	bool mark = e->modifiers() & Qt::ShiftModifier;
	int cursor = m_control->xToPos(e->pos().x());
#if QT_CONFIG(draganddrop)
	if(!mark && e->button() == Qt::LeftButton && m_control->inSelection(e->pos().x())) {
		if(!m_dndTimer.isActive())
			m_dndTimer.start(QApplication::startDragTime(), this);
	} else
#endif
	{
		m_control->cursorSetPosition(cursor, mark);
	}
}

bool
RichLineEdit::sendMouseEventToInputContext(QMouseEvent *e)
{
#if !defined QT_NO_IM
	if(m_control->composeMode()) {
		int tmp_cursor = m_control->xToPos(e->pos().x());
		int mousePos = tmp_cursor - m_control->cursor();
		if(mousePos < 0 || mousePos > m_control->preeditAreaText().length())
			mousePos = -1;
		if(mousePos >= 0) {
			if(e->type() == QEvent::MouseButtonRelease)
				QGuiApplication::inputMethod()->invokeAction(QInputMethod::Click, mousePos);
			return true;
		}
	}
#else
	Q_UNUSED(e);
#endif

	return false;
}

void
RichLineEdit::mouseMoveEvent(QMouseEvent * e)
{
	if(e->buttons() & Qt::LeftButton) {
#if QT_CONFIG(draganddrop)
		if(m_dndTimer.isActive()) {
			if((m_mousePressPos - e->pos()).manhattanLength() > QApplication::startDragDistance()) {
				m_dndTimer.stop();
				QMimeData *mime = new QMimeData();
				const QTextDocumentFragment &s = m_control->selection();
				mime->setHtml(s.toHtml());
				mime->setText(s.toPlainText());
				QDrag *drag = new QDrag(this);
				drag->setMimeData(mime);
				Qt::DropAction action = drag->exec(Qt::MoveAction);
				if(action == Qt::MoveAction && !m_control->isReadOnly() && drag->target() != this)
					m_control->eraseSelectedText();
			}
		} else
#endif
		{
			const bool select = true;
#ifndef QT_NO_IM
			if(m_mouseYThreshold > 0 && e->pos().y() > m_mousePressPos.y() + m_mouseYThreshold) {
				if(layoutDirection() == Qt::RightToLeft)
					m_control->home(select);
				else
					m_control->end(select);
			} else if(m_mouseYThreshold > 0 && e->pos().y() + m_mouseYThreshold < m_mousePressPos.y()) {
				if(layoutDirection() == Qt::RightToLeft)
					m_control->end(select);
				else
					m_control->home(select);
			} else if(m_control->composeMode() && select) {
				int startPos = m_control->xToPos(m_mousePressPos.x());
				int currentPos = m_control->xToPos(e->pos().x());
				if(startPos != currentPos)
					m_control->setSelection(startPos, currentPos - startPos);
			} else
#endif
			{
				m_control->cursorSetPosition(m_control->xToPos(e->pos().x()), select);
			}
		}
	}

	sendMouseEventToInputContext(e);
}

void
RichLineEdit::mouseReleaseEvent(QMouseEvent* e)
{
	if(sendMouseEventToInputContext(e))
		return;
#if QT_CONFIG(draganddrop)
	if(e->button() == Qt::LeftButton) {
		if(m_dndTimer.isActive()) {
			m_dndTimer.stop();
			m_control->deselect();
			return;
		}
	}
#endif
#ifndef QT_NO_CLIPBOARD
	if(QApplication::clipboard()->supportsSelection()) {
		if(e->button() == Qt::LeftButton) {
			m_control->copy(QClipboard::Selection);
		} else if(!m_control->isReadOnly() && e->button() == Qt::MiddleButton) {
			m_control->deselect();
			m_control->paste(QClipboard::Selection);
		}
	}
#endif
}

void
RichLineEdit::mouseDoubleClickEvent(QMouseEvent* e)
{
	if(e->button() == Qt::LeftButton) {
		int position = m_control->xToPos(e->pos().x());

		// exit composition mode
#ifndef QT_NO_IM
		if(m_control->composeMode()) {
			int preeditPos = m_control->cursor();
			int posInPreedit = position - m_control->cursor();
			int preeditLength = m_control->preeditAreaText().length();
			bool positionOnPreedit = false;

			if(posInPreedit >= 0 && posInPreedit <= preeditLength)
				positionOnPreedit = true;

			int textLength = m_control->end();
			m_control->commitPreedit();
			int sizeChange = m_control->end() - textLength;

			if(positionOnPreedit) {
				if(sizeChange == 0)
					position = -1; // cancel selection, word disappeared
				else
					// ensure not selecting after preedit if event happened there
					position = qBound(preeditPos, position, preeditPos + sizeChange);
			} else if(position > preeditPos) {
				// adjust positions after former preedit by how much text changed
				position += (sizeChange - preeditLength);
			}
		}
#endif

		if(position >= 0)
			m_control->selectWordAtPos(position);

		m_tripleClickTimer.start(QApplication::doubleClickInterval(), this);
		m_tripleClickPos = e->pos();
	} else {
		sendMouseEventToInputContext(e);
	}
}

void
RichLineEdit::keyPressEvent(QKeyEvent *event)
{
	const QKeySequence key(event->modifiers() | event->key());

	for(QAction *act: m_actions) {
		if(act->shortcuts().contains(key)) {
			act->trigger();
			m_control->updateDisplayText();
			return;
		}
	}

	m_control->processKeyEvent(event);
	if(event->isAccepted()) {
		if(layoutDirection() != m_control->layoutDirection())
			setLayoutDirection(m_control->layoutDirection());
		m_control->updateCursorBlinking();
		return;
	}

	QWidget::keyPressEvent(event);
}

void
RichLineEdit::inputMethodEvent(QInputMethodEvent *e)
{
	if(m_control->isReadOnly()) {
		e->ignore();
		return;
	}

	m_control->processInputMethodEvent(e);
}

QVariant
RichLineEdit::inputMethodQuery(Qt::InputMethodQuery property) const
{
	switch(property) {
	case Qt::ImCursorRectangle:
		return m_control->cursorRect();
	case Qt::ImAnchorRectangle:
		return m_control->anchorRect();
	case Qt::ImFont:
		return font();
	case Qt::ImCursorPosition: {
		return QVariant(m_control->cursor()); }
	case Qt::ImSurroundingText:
		return QVariant(m_control->text());
	case Qt::ImCurrentSelection:
		return QVariant(m_control->selectedText());
	case Qt::ImAnchorPosition:
		if(m_control->selectionStart() == m_control->selectionEnd())
			return QVariant(m_control->cursor());
		else if(m_control->selectionStart() == m_control->cursor())
			return QVariant(m_control->selectionEnd());
		else
			return QVariant(m_control->selectionStart());
	default:
		return QWidget::inputMethodQuery(property);
	}
}

void
RichLineEdit::focusInEvent(QFocusEvent *e)
{
	if(e->reason() == Qt::TabFocusReason || e->reason() == Qt::BacktabFocusReason || e->reason() == Qt::ShortcutFocusReason) {
		if(!m_control->hasSelection())
			m_control->selectAll();
	} else if(e->reason() == Qt::MouseFocusReason) {
		// no need to handle this yet
	}
	m_control->setBlinkingCursorEnabled(true);
#if QT_CONFIG(completer)
	if(m_control->completer()) {
		m_control->completer()->setWidget(this);
		// FIXME: completion
//		QObject::connect(m_control->completer(), &QCompleter::activated, this, &RichLineEdit::setText);
//		QObject::connect(m_control->completer(), &QCompleter::highlighted, this, &RichLineEdit::_q_completionHighlighted);
	}
#endif
	update();
}

void
RichLineEdit::focusOutEvent(QFocusEvent *e)
{
	Qt::FocusReason reason = e->reason();
	if(reason != Qt::ActiveWindowFocusReason && reason != Qt::PopupFocusReason)
		m_control->deselect();

	m_control->setBlinkingCursorEnabled(false);
	if(reason != Qt::PopupFocusReason || !(QApplication::activePopupWidget() && QApplication::activePopupWidget()->parentWidget() == this)) {
//		if(hasAcceptableInput() || m_control->fixup())
//			emit editingFinished();
	}
#if QT_CONFIG(completer)
	if(m_control->completer()) {
		QObject::disconnect(m_control->completer(), 0, this, 0);
	}
#endif
	QWidget::focusOutEvent(e);
}

void
RichLineEdit::changeEvent(QEvent *e)
{
	switch(e->type())
	{
	case QEvent::ActivationChange:
		if(!palette().isEqual(QPalette::Active, QPalette::Inactive))
			update();
		break;
	case QEvent::FontChange:
		m_control->setFont(font());
		break;
	case QEvent::StyleChange:
		update();
		break;
	default:
		break;
	}
	QWidget::changeEvent(e);
}


#if QT_CONFIG(draganddrop)
void
RichLineEdit::dragMoveEvent(QDragMoveEvent *e)
{
	if(!m_control->isReadOnly() && (e->mimeData()->hasText() || e->mimeData()->hasHtml())) {
		e->acceptProposedAction();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		m_dndCursor = m_control->xToPos(e->pos().x());
#else
		m_dndCursor = m_control->xToPos(e->position().x());
#endif
		update();
	}
}

void
RichLineEdit::dragEnterEvent(QDragEnterEvent *e)
{
	dragMoveEvent(e);
}

void
RichLineEdit::dragLeaveEvent(QDragLeaveEvent *)
{
	if(m_dndCursor >= 0) {
		m_dndCursor = -1;
		update();
	}
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#define position pos
#endif

void
RichLineEdit::dropEvent(QDropEvent *e)
{
	QString str = e->mimeData()->html();
	RichDocumentEditor::TextType strType = RichDocumentEditor::HTML;
	if(str.isEmpty()) {
		str = e->mimeData()->text();
		strType = RichDocumentEditor::Plain;
	}
	if(!str.isNull() && !m_control->isReadOnly()) {
		int dropPos = m_control->xToPos(e->position().x());
		m_dndCursor = -1;
		if(e->source() == this && e->dropAction() == Qt::MoveAction && m_control->selectionContains(dropPos)) {
			e->ignore();
			return;
		}
		if(e->source() == this && e->dropAction() == Qt::CopyAction)
			m_control->eraseSelectedText();
		e->acceptProposedAction();
		const int len = m_control->insert(str, dropPos, strType);
		if(e->source() == this)
			m_control->setSelection(m_control->cursor() - len, len);
	} else {
		e->ignore();
		update();
	}
}
#endif // QT_CONFIG(draganddrop)

void
RichLineEdit::paintEvent(QPaintEvent *e)
{
	QPainter p(this);
	p.setClipRect(e->rect());

	const QColor textColor = m_lineStyle.palette.color(QPalette::Normal, (m_lineStyle.state & QStyle::State_Selected) ? QPalette::HighlightedText : QPalette::Text);
	const QStyle *style = m_lineStyle.widget ? m_lineStyle.widget->style() : QApplication::style();

	const int hMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, nullptr, m_lineStyle.widget);
	const int vMargin = style->pixelMetric(QStyle::PM_FocusFrameVMargin, nullptr, m_lineStyle.widget);

	p.setPen(textColor);

	QRect textRect = rect();
	p.fillRect(textRect, m_lineStyle.palette.color(QPalette::Normal, QPalette::Highlight));

	textRect.adjust(hMargin, vMargin, -hMargin, -vMargin);
	p.fillRect(textRect, m_lineStyle.palette.color(QPalette::Normal, QPalette::Window));

	textRect.adjust(1, 1, -1, -1);

	QPoint textPos(textRect.topLeft());
	textPos.ry() += (qreal(textRect.height()) - m_control->height()) / 2.;

	int flags = RichDocumentEditor::DrawText | RichDocumentEditor::DrawCursor;
	if(m_control->hasSelection())
		flags |= RichDocumentEditor::DrawSelections;
	m_control->draw(&p, textPos, rect(), flags, m_dndCursor);
}
