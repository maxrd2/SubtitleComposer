#ifndef TREEVIEW_H
#define TREEVIEW_H

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

#include <QtGui/QTreeView>

class QTimer;

class TreeView : public QTreeView
{
	Q_OBJECT

public:
	explicit TreeView(QWidget *parent);
	virtual ~TreeView();

public slots:
	virtual void setModel(QAbstractItemModel *model);

protected slots:
	void onRowsAboutToBeInserted(const QModelIndex &parent, int start, int end);
	void onRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);

	virtual void updateGeometries();

	void onUpdateGeometriesTimeout();

protected:
	QTimer *m_updateGeometriesTimer;
	int m_currentModelRows;
	int m_instantGeometryUpdate;
};

#endif
