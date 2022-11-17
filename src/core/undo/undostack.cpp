/*
    SPDX-FileCopyrightText: 2020-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "undostack.h"

#include "appglobal.h"
#include "application.h"
#include "actions/useraction.h"
#include "actions/useractionnames.h"
#include "core/undo/undoaction.h"
#include "gui/treeview/lineswidget.h"
#include "gui/treeview/linesmodel.h"

using namespace SubtitleComposer;

UndoStack::UndoStack(QObject *parent)
	: QUndoStack(parent),
	  m_level(0),
	  m_undoAction(QUndoStack::createUndoAction(UserActionManager::instance())),
	  m_redoAction(QUndoStack::createRedoAction(UserActionManager::instance()))
{
	m_selectionStack.push(Selection(app()->linesWidget()->selectionModel()));

	connect(this, &UndoStack::undoTextChanged, undoAction(), &QAction::setToolTip);
	connect(this, &UndoStack::redoTextChanged, redoAction(), &QAction::setToolTip);
	connect(this, &UndoStack::indexChanged, parent, [](){ if(Subtitle *s = appSubtitle()) s->updateState(); });
	connect(this, &UndoStack::cleanChanged, parent, [](){ if(Subtitle *s = appSubtitle()) s->updateState(); });
}

UndoStack::~UndoStack()
{

}

void
UndoStack::clear()
{
	m_selectionStack.clear();
	m_selectionStack.push(Selection(app()->linesWidget()->selectionModel()));
	QUndoStack::clear();
}


inline static void
restoreSelection(int current, const QList<std::pair<int, int>> &selection)
{
	LinesWidget *lw = app()->linesWidget();
	LinesModel *lm = lw->model();
	QItemSelectionModel *sm = lw->selectionModel();

	// make sure that selection has valid iterators
	lm->processSelectionUpdate();

	// restore selected ranges
	QItemSelection itemSel;
	const int lastCol = lm->columnCount() - 1;
	for(const std::pair<int, int> &r : selection)
		itemSel.push_back(QItemSelectionRange(lm->index(r.first), lm->index(r.second, lastCol)));
	if(sm->selection() != itemSel)
		sm->select(itemSel, QItemSelectionModel::ClearAndSelect);

	// restore current item
	const QModelIndex idx = lm->index(current);
	if(sm->currentIndex().row() != idx.row()) {
		sm->setCurrentIndex(idx, QItemSelectionModel::Rows);
		if(lw->scrollFollowsModel())
			lw->scrollTo(idx, QAbstractItemView::EnsureVisible);
	}
}

inline static void
saveSelection(int *current, QList<std::pair<int, int>> *selection)
{
	LinesWidget *lw = app()->linesWidget();
	const QItemSelectionModel *sm = lw->selectionModel();

	// make sure that selection has valid iterators
	lw->model()->processSelectionUpdate();

	*current = sm->currentIndex().row();

	selection->clear();
	const QItemSelection &is = sm->selection();
	for(const QItemSelectionRange &r : is)
		selection->push_back(std::pair<int, int>(r.top(), r.bottom()));
}

void
UndoStack::levelIncrease(int idx)
{
	if(m_level++ == 0) {
		while(m_dirtyStack.size() < idx)
			m_dirtyStack.push(DirtyMode::None);
		while(m_selectionStack.size() <= idx)
			m_selectionStack.push(Selection());
		Selection &sel = m_selectionStack[idx];
		saveSelection(&sel.preCurrentRow, &sel.preSelection);
	}
}

void
UndoStack::levelDecrease(int idx)
{
	if(--m_level == 0) {
		Selection &sel = m_selectionStack[idx];
		saveSelection(&sel.postCurrentRow, &sel.postSelection);
	}
}

void
UndoStack::push(UndoAction *cmd)
{
	const int idx = index();
	const int idx1 = idx + 1;
	levelIncrease(idx1);
	m_dirtyStack[idx] = static_cast<DirtyMode>(m_dirtyStack.at(idx) | cmd->m_dirtyMode);
	QUndoStack::push(cmd); // NOTE: cmd can/will be deleted after push()
	levelDecrease(idx1);
}

void
UndoStack::beginMacro(const QString &text)
{
	QUndoStack::beginMacro(text);
	levelIncrease(index() + 1);
}

void
UndoStack::endMacro(DirtyMode dirtyOverride)
{
	const int idx = index();
	levelDecrease(idx + 1);
	if(dirtyOverride != Invalid)
		m_dirtyStack[idx] = dirtyOverride;
	QUndoStack::endMacro();
}

void
UndoStack::undo()
{
	QUndoStack::undo();

	const Selection &sel = m_selectionStack.at(index() + 1);
	restoreSelection(sel.preCurrentRow, sel.preSelection);
}

void
UndoStack::redo()
{
	QUndoStack::redo();

	const Selection &sel = m_selectionStack.at(index());
	restoreSelection(sel.postCurrentRow, sel.postSelection);
}


UndoStack::Selection::Selection()
	: preCurrentRow(-1)
{
}

UndoStack::Selection::Selection(QItemSelectionModel *sel)
	: preCurrentRow(sel->currentIndex().row()),
	  postCurrentRow(sel->currentIndex().row())
{
	const QItemSelection &is = sel->selection();
	for(const QItemSelectionRange &r : is) {
		preSelection.push_back(std::pair<int, int>(r.top(), r.bottom()));
		postSelection.push_back(std::pair<int, int>(r.top(), r.bottom()));
	}
}
