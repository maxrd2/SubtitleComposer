#ifndef ACTION_H
#define ACTION_H

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

#include <typeinfo>

#include <QString>

namespace SubtitleComposer {
class Action
{
	typedef enum {
		Executed = 0x1,		// Action executed at least once
		Done = 0x2			// Action executed an odd number of times (it can be undone)
	} State;

	friend class ActionManager;
	friend class CompositeAction;

public:
	Action(const QString &description = QString()) : m_description(description), m_state(0) {}
	virtual ~Action() {}

	inline const QString & description() const { return m_description; }

	/**
	 * @brief executed
	 * @return true if action has been executed at least once
	 */
	inline bool executed() { return m_state & Executed; }

	/**
	 * @brief done
	 * @return true if action can be undone
	 */
	inline bool done() { return m_state & Done; }

	void redo() { internalRedo(true); }

	void undo() { internalUndo(true); }

protected:
	/**
	 * @brief Called if both - the callee and the action parameter - have been executed.
	 *
	 * Hence, if the callee can be merged with the previous action, it must recover the state
	 * previous to execution of both actions and return true (if the actions can't be merged,
	 * just return false).
	 */
	inline virtual bool mergeWithPrevious(Action * /*action*/) { return false; }

	void internalRedo(bool emitSignals)
	{
		if(!(m_state & Done)) {
			internalPreRedo();
			internalRedo();
			if(emitSignals)
				internalEmitRedoSignals();
			m_state |= Executed | Done;
		}
	}

	void internalUndo(bool emitSignals)
	{
		if(m_state & Done) {
			internalPreUndo();
			internalUndo();
			if(emitSignals)
				internalEmitUndoSignals();
			m_state &= ~Done;
		}
	}

	virtual void internalRedo() = 0;
	virtual void internalUndo() = 0;

	virtual void internalPreRedo() {}
	virtual void internalPreUndo() {}

	virtual void internalEmitRedoSignals() {}
	virtual void internalEmitUndoSignals() {}

protected:
	QString m_description;

private:
	int m_state;
};
}

#endif
