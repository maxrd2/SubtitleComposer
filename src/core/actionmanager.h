#ifndef ACTIONMANAGER_H
#define ACTIONMANAGER_H

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "action.h"

#include <QtCore/QObject>
#include <QtCore/QTime>
#include <QtCore/QLinkedList>

namespace SubtitleComposer {
class ActionManager : public QObject
{
	Q_OBJECT

public:
	ActionManager();
	~ActionManager();

	bool hasRedo() const;
	bool hasUndo() const;

	int redoCount() const;
	int undoCount() const;

	QString redoDescription() const;
	QString undoDescription() const;

	void execAndStore(Action *action);

public slots:
	void redo();
	void undo();
	void popUndo();
	void clearHistory();

signals:
	void actionStored();
	void actionRemoved();
	void actionRedone();
	void actionUndone();
	void historyCleared();
	void stateChanged();

private:
	ActionManager(const ActionManager &undoManager);
	ActionManager & operator=(const ActionManager &undoManager);

	mutable QLinkedList<Action *> m_undoStack;
	mutable QLinkedList<Action *> m_redoStack;

	int m_compressionThreshold;
	QTime m_timeBetweenActions;
};
}
#endif
