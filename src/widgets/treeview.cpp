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

#include "treeview.h"

#include <QtCore/QTimer>
#include <QtGui/QScrollBar>

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

	connect(m_updateGeometriesTimer, SIGNAL(timeout()), this, SLOT(onUpdateGeometriesTimeout()));
}

TreeView::~TreeView()
{}

void
TreeView::setModel(QAbstractItemModel *model)
{
	if(this->model()) {
		if(this->model()->metaObject()->indexOfSignal("dataChanged()") != -1)
			disconnect(this->model(), SIGNAL(dataChanged()), this->viewport(), SLOT(update()));

		disconnect(this->model(), SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)), this, SLOT(onRowsAboutToBeInserted(const QModelIndex &, int, int)));
		disconnect(this->model(), SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)), this, SLOT(onRowsAboutToBeRemoved(const QModelIndex &, int, int)));
	}

	QTreeView::setModel(model);

	if(model) {
		m_currentModelRows = model->rowCount();

		if(model->metaObject()->indexOfSignal("dataChanged()") != -1)
			connect(model, SIGNAL(dataChanged()), this->viewport(), SLOT(update()));

		connect(this->model(), SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)), this, SLOT(onRowsAboutToBeInserted(const QModelIndex &, int, int)));
		connect(this->model(), SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)), this, SLOT(onRowsAboutToBeRemoved(const QModelIndex &, int, int)));
	} else
		m_currentModelRows = -1;
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

#include "treeview.moc"
