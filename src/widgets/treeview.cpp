/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2018 Mladen Milinkovic <max@smoothware.net>
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

#include "treeview.h"

#include <QTimer>
#include <QScrollBar>

TreeView::TreeView(QWidget *parent) :
	QTreeView(parent),
	m_updateGeometriesTimer(new QTimer(this)),
	m_currentModelRows(-1),
	m_instantGeometryUpdate(0)
{
	// NOTE: The updateGeometries() methods takes forever to execute (around 100ms
	// for a view with more that 1 thousand items, and more as more items are added).
	// The culprit is the updateGeometries() method in QHeaderView which takes a lot
	// of time when there are columns with ResizeToContents mode set.
	// To workaround this, we reimplement the updateGeometries method to (re)start a
	// timer that invoques the real work only on timeout (reported as BUG 234368 at Qt).
	// To further refine our hack, we only delay the geometries update when the model's
	// "has rows" (imaginary) property hasn't changed since last geometries update.
	// This works very well in our case because the columns with ResizeToContents enabled
	// have items of the same size and so their geometries only change with "has rows" state.

	m_updateGeometriesTimer->setInterval(200);
	m_updateGeometriesTimer->setSingleShot(true);

	connect(m_updateGeometriesTimer, &QTimer::timeout, this, &TreeView::onUpdateGeometriesTimeout);
}

TreeView::~TreeView()
{}

void
TreeView::setModel(QAbstractItemModel *newModel)
{
	if(model()) {
		disconnect(model(), &QAbstractItemModel::dataChanged, viewport(), QOverload<>::of(&QWidget::update));
		disconnect(model(), &QAbstractItemModel::rowsAboutToBeInserted, this, &TreeView::onRowsAboutToBeInserted);
		disconnect(model(), &QAbstractItemModel::rowsAboutToBeRemoved, this, &TreeView::onRowsAboutToBeRemoved);
	}

	QTreeView::setModel(newModel);

	if(newModel) {
		m_currentModelRows = newModel->rowCount();
		connect(newModel, &QAbstractItemModel::dataChanged, viewport(), QOverload<>::of(&QWidget::update));
		connect(newModel, &QAbstractItemModel::rowsAboutToBeInserted, this, &TreeView::onRowsAboutToBeInserted);
		connect(newModel, &QAbstractItemModel::rowsAboutToBeRemoved, this, &TreeView::onRowsAboutToBeRemoved);
	} else {
		m_currentModelRows = -1;
	}
}

void
TreeView::onRowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
	if(!parent.isValid()) {
		m_instantGeometryUpdate = m_currentModelRows == 0 ? 1 : 0;
		m_currentModelRows += end - start + 1;
	}
}

void
TreeView::onRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
	if(!parent.isValid()) {
		m_currentModelRows -= end - start + 1;
		m_instantGeometryUpdate = m_currentModelRows == 0 ? 1 : 0;
	}
}

void
TreeView::updateGeometries()
{
	if(m_instantGeometryUpdate--)
		QTreeView::updateGeometries();
	else
		m_updateGeometriesTimer->start();
}

void
TreeView::onUpdateGeometriesTimeout()
{
	QTreeView::updateGeometries();
}


