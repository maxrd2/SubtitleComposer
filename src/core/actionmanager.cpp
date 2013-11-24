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

#include "actionmanager.h"

using namespace SubtitleComposer;

ActionManager::ActionManager() :
	m_undoStack(),
	m_redoStack(),
	m_compressionThreshold(350)
{
	m_timeBetweenActions.start();

	connect(this, SIGNAL(actionStored()), this, SIGNAL(stateChanged()));
	connect(this, SIGNAL(actionRemoved()), this, SIGNAL(stateChanged()));
	connect(this, SIGNAL(actionRedone()), this, SIGNAL(stateChanged()));
	connect(this, SIGNAL(actionUndone()), this, SIGNAL(stateChanged()));
	connect(this, SIGNAL(historyCleared()), this, SIGNAL(stateChanged()));
}

ActionManager::~ActionManager()
{
	disconnect();

	qDeleteAll(m_undoStack);
	qDeleteAll(m_redoStack);
}

void
ActionManager::execAndStore(Action *action)
{
	if(action != 0) {
		action->redo();

		bool compressed = false;
		if(m_timeBetweenActions.restart() < m_compressionThreshold && m_redoStack.isEmpty()) {
			Action *previousAction = m_undoStack.isEmpty() ? 0 : m_undoStack.first();
			if(previousAction && action->mergeWithPrevious(previousAction)) {
				delete m_undoStack.takeFirst();
				compressed = true;
			}
		}

		qDeleteAll(m_redoStack);
		m_redoStack.clear();

		m_undoStack.prepend(action);

		if(!compressed)
			emit actionStored();

		emit actionRedone();
	}
}

void
ActionManager::popUndo()
{
	if(m_undoStack.isEmpty())
		return; // nothing to undo

	delete m_undoStack.takeFirst();
	m_redoStack.clear();

	emit actionRemoved();
}

bool
ActionManager::hasRedo() const
{
	return !m_redoStack.isEmpty();
}

bool
ActionManager::hasUndo() const
{
	return !m_undoStack.isEmpty();
}

int
ActionManager::redoCount() const
{
	return m_redoStack.count();
}

int
ActionManager::undoCount() const
{
	return m_undoStack.count();
}

QString
ActionManager::redoDescription() const
{
	return m_redoStack.isEmpty() ? QString() : m_redoStack.first()->description();
}

QString
ActionManager::undoDescription() const
{
	return m_undoStack.isEmpty() ? QString() : m_undoStack.first()->description();
}

void
ActionManager::redo()
{
	if(m_redoStack.isEmpty())
		return; // nothing to redo

	Action *action = m_redoStack.takeFirst();
	action->redo();
	m_undoStack.prepend(action);

	emit actionRedone();
}

void
ActionManager::undo()
{
	if(m_undoStack.isEmpty())
		return; // nothing to undo

	Action *action = m_undoStack.takeFirst();
	action->undo();
	m_redoStack.prepend(action);

	emit actionUndone();
}

void
ActionManager::clearHistory()
{
	m_undoStack.clear();

	qDeleteAll(m_redoStack);
	m_redoStack.clear();

	emit historyCleared();
}

#include "actionmanager.moc"
