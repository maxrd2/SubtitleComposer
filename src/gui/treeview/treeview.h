/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TREEVIEW_H
#define TREEVIEW_H

#include <QTreeView>

QT_FORWARD_DECLARE_CLASS(QTimer)

class TreeView : public QTreeView
{
	Q_OBJECT

public:
	explicit TreeView(QWidget *parent);
	virtual ~TreeView();

public slots:
	void setModel(QAbstractItemModel *model) override;

protected slots:
	void onRowsAboutToBeInserted(const QModelIndex &parent, int start, int end);
	void onRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);

	void updateGeometries() override;

	void onUpdateGeometriesTimeout();

protected:
	QTimer *m_updateGeometriesTimer;
	int m_currentModelRows;
	int m_instantGeometryUpdate;
};

#endif
