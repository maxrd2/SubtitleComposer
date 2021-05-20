/*
 * Copyright (C) 2020 Mladen Milinkovic <max@smoothware.net>
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

#include "application.h"
#include "actions/useractionnames.h"
#include "core/undo/undoaction.h"
#include "core/undo/undostack.h"
#include "gui/treeview/lineswidget.h"
#include "gui/treeview/linesmodel.h"

using namespace SubtitleComposer;

UndoStack::UndoStack(QObject *parent)
	: QUndoStack(parent),
	  m_level(0)
{
	m_selectionStack.push(Selection(app()->linesWidget()->selectionModel()));

	connect(this, &UndoStack::undoTextChanged, app()->action(ACT_UNDO), &QAction::setToolTip);
	connect(this, &UndoStack::redoTextChanged, app()->action(ACT_REDO), &QAction::setToolTip);
	connect(this, &UndoStack::indexChanged, parent, [](){ if(Subtitle *s = app()->subtitle()) s->updateState(); });
	connect(this, &UndoStack::cleanChanged, parent, [](){ if(Subtitle *s = app()->subtitle()) s->updateState(); });
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
	const int idx = index() + 1;
	levelIncrease(idx);
	QUndoStack::push(cmd);
	levelDecrease(idx);
}

void
UndoStack::beginMacro(const QString &text)
{
	QUndoStack::beginMacro(text);
	levelIncrease(index() + 1);
}

void
UndoStack::endMacro()
{
	levelDecrease(index() + 1);
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
