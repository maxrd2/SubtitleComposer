/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2020 Mladen Milinkovic <max@smoothware.net>
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

#include "lineswidget.h"
#include "application.h"
#include "actions/useractionnames.h"
#include "dialogs/actionwithtargetdialog.h"
#include "gui/linesitemdelegate.h"
#ifdef USE_PERSISTENT_SELECTION_MODEL
#include "gui/linesselectionmodel.h"
#endif

#include <QHeaderView>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>

#include <KConfigGroup>
#include <KSharedConfig>

using namespace SubtitleComposer;

LinesWidget::LinesWidget(QWidget *parent)
	: TreeView(parent),
	  m_scrollFollowsModel(true),
	  m_translationMode(false),
	  m_showingContextMenu(false),
	  m_plainTextDelegate(new LinesItemDelegate(true, true, false, this)),
	  m_richTextDelegate(new LinesItemDelegate(true, true, true, this))
{
	setModel(new LinesModel(this));
#ifdef USE_PERSISTENT_SELECTION_MODEL
	selectionModel()->deleteLater();
	setSelectionModel(new LinesSelectionModel(model()));
#endif

	for(int column = 0, columnCount = model()->columnCount(); column < columnCount; ++column)
		setItemDelegateForColumn(column, column < LinesModel::Text ? m_plainTextDelegate : m_richTextDelegate);

	QHeaderView *header = this->header();
	header->setSectionsClickable(false);
	header->setSectionsMovable(false);
	header->setSectionResizeMode(LinesModel::Number, QHeaderView::ResizeToContents);
	header->setSectionResizeMode(LinesModel::ShowTime, QHeaderView::ResizeToContents);
	header->setSectionResizeMode(LinesModel::HideTime, QHeaderView::ResizeToContents);
	header->setSectionResizeMode(LinesModel::Text, QHeaderView::Interactive);
	header->setSectionResizeMode(LinesModel::Translation, QHeaderView::Interactive);
	header->setSectionHidden(LinesModel::Translation, true);

	setUniformRowHeights(true);
	setItemsExpandable(false);
	setAutoExpandDelay(-1);
	setExpandsOnDoubleClick(false);
	setRootIsDecorated(false);
	setAllColumnsShowFocus(true);
	setSortingEnabled(false);

	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setSelectionBehavior(QAbstractItemView::SelectRows);

	m_gridPen.setColor(palette().mid().color().lighter(120));

	setAcceptDrops(true);
	viewport()->installEventFilter(this);

	connect(selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(onCurrentRowChanged()));
}

LinesWidget::~LinesWidget()
{
}

void
LinesWidget::closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
	LinesItemDelegate::ExtendedEditHint ehint = (LinesItemDelegate::ExtendedEditHint)hint;
	QModelIndex editorIndex = currentIndex();

	if(ehint == LinesItemDelegate::EditUpperItem)
		TreeView::closeEditor(editor, QAbstractItemDelegate::EditPreviousItem);
	else if(ehint == LinesItemDelegate::EditLowerItem)
		TreeView::closeEditor(editor, QAbstractItemDelegate::EditNextItem);
	else if(m_translationMode) {
		if(ehint == LinesItemDelegate::EditNextItem)
			TreeView::closeEditor(editor, editorIndex.column() != LinesModel::Text ? QAbstractItemDelegate::EditNextItem : QAbstractItemDelegate::NoHint);
		else if(ehint == LinesItemDelegate::EditPreviousItem)
			TreeView::closeEditor(editor, editorIndex.column() == LinesModel::Text ? QAbstractItemDelegate::EditPreviousItem : QAbstractItemDelegate::NoHint);
		else
			TreeView::closeEditor(editor, hint);
	} else
		TreeView::closeEditor(editor, hint);

	switch(ehint) {
	case LinesItemDelegate::NoHint:
	case LinesItemDelegate::SubmitModelCache:
	case LinesItemDelegate::RevertModelCache:
		break;
	case LinesItemDelegate::EditUpperItem:
		if(editorIndex.row() > 0)
			editCurrentLineInPlace(editorIndex.column() == LinesModel::Text);
		break;
	case LinesItemDelegate::EditLowerItem:
		if(editorIndex.row() < model()->rowCount() - 1)
			editCurrentLineInPlace(editorIndex.column() == LinesModel::Text);
		break;

	case LinesItemDelegate::EditPreviousItem:
		if(editorIndex.row() > 0 || (m_translationMode && editorIndex.column() == LinesModel::Translation))
			editCurrentLineInPlace(editorIndex.column() != LinesModel::Text);
		break;

	case LinesItemDelegate::EditNextItem:
		if(editorIndex.row() < model()->rowCount() - 1 || (m_translationMode && editorIndex.column() == LinesModel::Text))
			editCurrentLineInPlace(editorIndex.column() != LinesModel::Text);
		break;
	}
}

void
LinesWidget::editCurrentLineInPlace(bool primaryText)
{
	QModelIndex currentIndex = this->currentIndex();
	if(currentIndex.isValid()) {
		const int col = primaryText || !m_translationMode ? LinesModel::Text : LinesModel::Translation;
		currentIndex = model()->index(currentIndex.row(), col);
		setCurrentIndex(currentIndex);
		edit(currentIndex);
	}
}

bool
LinesWidget::showingContextMenu()
{
	return m_showingContextMenu;
}

SubtitleLine *
LinesWidget::currentLine() const
{
	const QModelIndex idx = currentIndex();
	return idx.isValid() ? model()->subtitle()->line(idx.row()) : nullptr;
}

int
LinesWidget::currentLineIndex() const
{
	const QModelIndex idx = currentIndex();
	return idx.isValid() ? idx.row() : -1;
}

int
LinesWidget::firstSelectedIndex() const
{
	int row = -1;
	const QItemSelection &selection = selectionModel()->selection();
	for(const QItemSelectionRange &r : selection) {
		if(row == -1 || r.top() < row)
			row = r.top();
	}
	return row;
}

int
LinesWidget::lastSelectedIndex() const
{
	int row = -1;
	const QItemSelection &selection = selectionModel()->selection();
	for(const QItemSelectionRange &r : selection) {
		if(r.bottom() > row)
			row = r.bottom();
	}
	return row;
}

bool
LinesWidget::selectionHasMultipleRanges() const
{
	const QItemSelection &selection = selectionModel()->selection();
	return selection.size() > 1;
}

RangeList
LinesWidget::selectionRanges() const
{
	RangeList ranges;

	const QItemSelection &selection = selectionModel()->selection();
	for(const QItemSelectionRange &r : selection) {
		ranges << Range(r.top(), r.bottom());
	}

	return ranges;
}

RangeList
LinesWidget::targetRanges(int target) const
{
	switch(target) {
	case ActionWithTargetDialog::AllLines:
		return Range::full();
	case ActionWithTargetDialog::Selection: return selectionRanges();
	case ActionWithTargetDialog::FromSelected: {
		int index = firstSelectedIndex();
		return index < 0 ? RangeList() : Range::upper(index);
	}
	case ActionWithTargetDialog::UpToSelected: {
		int index = lastSelectedIndex();
		return index < 0 ? RangeList() : Range::lower(index);
	}
	default:
		return RangeList();
	}
}

void
LinesWidget::loadConfig()
{
	KConfigGroup group(KSharedConfig::openConfig()->group("LinesWidget Settings"));

	QByteArray state;
	QStringList strState = group.readXdgListEntry("Columns State", QStringList() << QString());
	for(QStringList::ConstIterator it = strState.constBegin(), end = strState.constEnd(); it != end; ++it)
		state.append((char)(*it).toInt());
	header()->restoreState(state);
}

void
LinesWidget::saveConfig()
{
	KConfigGroup group(KSharedConfig::openConfig()->group("LinesWidget Settings"));

	QStringList strState;
	QByteArray state = header()->saveState();
	for(int index = 0, size = state.size(); index < size; ++index)
		strState.append(QString::number(state[index]));
	group.writeXdgListEntry("Columns State", strState);
}

void
LinesWidget::setSubtitle(Subtitle *subtitle)
{
	model()->setSubtitle(subtitle);
}

void
LinesWidget::setTranslationMode(bool enabled)
{
	if(m_translationMode != enabled) {
		m_translationMode = enabled;

		QHeaderView *header = this->header();
		if(m_translationMode) {
			int textColumnWidth = header->sectionSize(LinesModel::Text);
			header->resizeSection(LinesModel::Text, textColumnWidth / 2);
			header->setSectionHidden(LinesModel::Translation, false);
			header->resizeSection(LinesModel::Translation, textColumnWidth / 2);
		} else {
			int textColumnWidth = header->sectionSize(LinesModel::Text);
			int translationColumnSize = header->sectionSize(LinesModel::Translation);
			header->setSectionHidden(LinesModel::Translation, true);
			header->resizeSection(LinesModel::Text, textColumnWidth + translationColumnSize);
		}
	}
}

void
LinesWidget::setCurrentLine(SubtitleLine *line, bool clearSelection)
{
	if(!line)
		return;

	selectionModel()->setCurrentIndex(model()->index(line->index(), 0),
									  QItemSelectionModel::Select | QItemSelectionModel::Rows
									  | (clearSelection ? QItemSelectionModel::Clear : QItemSelectionModel::NoUpdate));
}

void
LinesWidget::setPlayingLine(SubtitleLine *line)
{
	model()->setPlayingLine(line);
}

void
LinesWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
	QModelIndex index = indexAt(viewport()->mapFromGlobal(e->globalPos()));
	if(index.isValid())
		emit lineDoubleClicked(model()->subtitle()->line(index.row()));
}

bool
LinesWidget::eventFilter(QObject *object, QEvent *event)
{
	if(object == viewport()) {
		switch(event->type()) {
		case QEvent::DragEnter:
		case QEvent::Drop:
			foreach(const QUrl &url, static_cast<QDropEvent *>(event)->mimeData()->urls()) {
				if(url.scheme() == QLatin1String("file")) {
					event->accept();
					if(event->type() == QEvent::Drop) {
						app()->openSubtitle(url);
					}
					return true; // eat event
				}
			}
			event->ignore();
			return true;

		case QEvent::DragMove:
			return true; // eat event

		default:
			;
		}
	}
	// standard event processing
	return TreeView::eventFilter(object, event);
}

void
LinesWidget::contextMenuEvent(QContextMenuEvent *e)
{
	SubtitleLine *referenceLine = nullptr;
	QItemSelectionModel *selection = selectionModel();
	for(int row = 0, rowCount = model()->rowCount(); row < rowCount; ++row) {
		if(selection->isSelected(model()->index(row, 0))) {
			referenceLine = model()->subtitle()->line(row);
			break;
		}
	}
	if(!referenceLine)
		return;

	QList<QAction *> checkableActions;
	auto appAction = [&](const char *actionName, bool checkable=false, bool checked=false) -> QAction * {
		QAction *action = app()->action(actionName);
		if(checkable) {
			checkableActions.append(action);
			action->setCheckable(true);
			action->setChecked(checked);
		}
		return action;
	};

	QMenu menu;
	menu.addAction(appAction(ACT_SELECT_ALL_LINES));
	menu.addAction(appAction(ACT_EDIT_CURRENT_LINE_IN_PLACE));
	menu.addSeparator();
	menu.addAction(appAction(ACT_INSERT_BEFORE_CURRENT_LINE));
	menu.addAction(appAction(ACT_INSERT_AFTER_CURRENT_LINE));
	menu.addAction(appAction(ACT_REMOVE_SELECTED_LINES));
	menu.addSeparator();
	menu.addAction(appAction(ACT_JOIN_SELECTED_LINES));
	menu.addAction(appAction(ACT_SPLIT_SELECTED_LINES));
	menu.addSeparator();
	menu.addAction(appAction(ACT_ANCHOR_TOGGLE));
	menu.addAction(appAction(ACT_ANCHOR_REMOVE_ALL));
	menu.addSeparator();

	QMenu textsMenu(i18n("Texts"));
	textsMenu.addAction(appAction(ACT_ADJUST_TEXTS));
	textsMenu.addAction(appAction(ACT_UNBREAK_TEXTS));
	textsMenu.addAction(appAction(ACT_SIMPLIFY_SPACES));
	textsMenu.addAction(appAction(ACT_CHANGE_CASE));
	textsMenu.addAction(appAction(ACT_FIX_PUNCTUATION));
	textsMenu.addAction(appAction(ACT_TRANSLATE));
	textsMenu.addSeparator();
	textsMenu.addAction(appAction(ACT_SPELL_CHECK));
	menu.addMenu(&textsMenu);

	QMenu stylesMenu(i18n("Styles"));
	const int styleFlags = referenceLine->primaryText().cummulativeStyleFlags() | referenceLine->secondaryText().cummulativeStyleFlags();
	stylesMenu.addAction(appAction(ACT_TOGGLE_SELECTED_LINES_BOLD, true, styleFlags & SString::Bold));
	stylesMenu.addAction(appAction(ACT_TOGGLE_SELECTED_LINES_ITALIC, true, styleFlags & SString::Italic));
	stylesMenu.addAction(appAction(ACT_TOGGLE_SELECTED_LINES_UNDERLINE, true, styleFlags & SString::Underline));
	stylesMenu.addAction(appAction(ACT_TOGGLE_SELECTED_LINES_STRIKETHROUGH, true, styleFlags & SString::StrikeThrough));
	stylesMenu.addAction(appAction(ACT_CHANGE_SELECTED_LINES_TEXT_COLOR));
	menu.addMenu(&stylesMenu);

	QMenu timesMenu(i18n("Times"));
	timesMenu.addAction(appAction(ACT_SHIFT_SELECTED_LINES_FORWARDS));
	timesMenu.addAction(appAction(ACT_SHIFT_SELECTED_LINES_BACKWARDS));
	timesMenu.addSeparator();
	timesMenu.addAction(appAction(ACT_SHIFT));
	timesMenu.addAction(appAction(ACT_SORT_LINES));
	timesMenu.addAction(appAction(ACT_DURATION_LIMITS));
	timesMenu.addAction(appAction(ACT_AUTOMATIC_DURATIONS));
	timesMenu.addAction(appAction(ACT_MAXIMIZE_DURATIONS));
	timesMenu.addAction(appAction(ACT_FIX_OVERLAPPING_LINES));
	menu.addMenu(&timesMenu);

	QMenu errorsMenu(i18n("Errors"));
	errorsMenu.addAction(appAction(ACT_TOGGLE_SELECTED_LINES_MARK, true, referenceLine->errorFlags() & SubtitleLine::UserMark));
	errorsMenu.addSeparator();
	errorsMenu.addAction(appAction(ACT_DETECT_ERRORS));
	errorsMenu.addAction(appAction(ACT_CLEAR_ERRORS));
	errorsMenu.addSeparator();
	errorsMenu.addAction(appAction(ACT_SHOW_ERRORS));
	menu.addMenu(&errorsMenu);

	m_showingContextMenu = true;
	menu.exec(e->globalPos());
	m_showingContextMenu = false;

	foreach(QAction *action, checkableActions)
		action->setCheckable(false);

	e->ignore();

	TreeView::contextMenuEvent(e);
}

void
LinesWidget::onCurrentRowChanged()
{
	QModelIndex current = this->currentIndex();
	emit currentLineChanged(current.isValid() ? model()->subtitle()->line(current.row()) : nullptr);
}

void
LinesWidget::drawHorizontalDotLine(QPainter *painter, int x1, int x2, int y)
{
	static int aux;

	if(x1 > x2) {
		aux = x1;
		x1 = x2;
		x2 = aux;
	}

	for(int x = x1 + 1; x <= x2; x += 2)
		painter->drawPoint(x, y);
}

void
LinesWidget::drawVerticalDotLine(QPainter *painter, int x, int y1, int y2)
{
	static int aux;

	if(y1 > y2) {
		aux = y1;
		y1 = y2;
		y2 = aux;
	}

	for(int y = y1 + 1; y <= y2; y += 2)
		painter->drawPoint(x, y);
}

void
LinesWidget::drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	TreeView::drawRow(painter, option, index);

	const int visibleColumns = m_translationMode ? LinesModel::ColumnCount : LinesModel::ColumnCount - 1;
	const int row = index.row();
	const bool rowSelected = selectionModel()->isSelected(index);
	const QPalette palette = this->palette();
	const QRect rowRect = QRect(visualRect(model()->index(row, 0)).topLeft(),
								visualRect(model()->index(row, visibleColumns - 1)).bottomRight());

	// draw row grid
	painter->setPen(m_gridPen);
	if(!rowSelected)
		drawHorizontalDotLine(painter, rowRect.left(), rowRect.right(), rowRect.bottom());
	for(int column = 0; column < visibleColumns - 1; ++column) {
		const QRect cellRect = visualRect(model()->index(row, column));
		drawVerticalDotLine(painter, cellRect.right(), rowRect.top(), rowRect.bottom());
	}
	if(index.row() == currentIndex().row()) {
		painter->setPen(palette.windowText().color());

		drawHorizontalDotLine(painter, rowRect.left(), rowRect.right(), rowRect.top());
		drawHorizontalDotLine(painter, rowRect.left(), rowRect.right(), rowRect.bottom());

		drawVerticalDotLine(painter, rowRect.left(), rowRect.top(), rowRect.bottom());
		drawVerticalDotLine(painter, rowRect.right(), rowRect.top(), rowRect.bottom());
	}
}

void
LinesWidget::changeEvent(QEvent *event)
{
	if(event->type() == QEvent::FontChange)
		m_richTextDelegate->updateStyle();
}
