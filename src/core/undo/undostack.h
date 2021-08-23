/*
    SPDX-FileCopyrightText: 2020 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef UNDOSTACK_H
#define UNDOSTACK_H

#include <QUndoStack>

#include <QStack>

QT_FORWARD_DECLARE_CLASS(QItemSelectionModel)

namespace SubtitleComposer {
class UndoAction;

class UndoStack : private QUndoStack
{
	Q_OBJECT

	struct Selection {
		Selection();
		Selection(QItemSelectionModel *sel);
		int preCurrentRow;
		int postCurrentRow;
		QList<std::pair<int, int>> preSelection;
		QList<std::pair<int, int>> postSelection;
	};

public:
	typedef enum {
		None = 0,
		Primary = 1,
		Secondary = 2,
		Both = Primary | Secondary,
		Invalid = -1
	} DirtyMode;

	explicit UndoStack(QObject *parent = nullptr);
	virtual ~UndoStack();

	void clear();
	void push(UndoAction *cmd);

	void beginMacro(const QString &text);
	void endMacro(DirtyMode dirtyOverride = Invalid);

	using QUndoStack::canUndo;
	using QUndoStack::canRedo;
	using QUndoStack::undoText;
	using QUndoStack::redoText;

	using QUndoStack::count;
	using QUndoStack::index;
	using QUndoStack::text;

	inline QAction *undoAction() const { return m_undoAction; }
	inline QAction *redoAction() const { return m_redoAction; }

	// Subtitle handles/implements these
//	using QUndoStack::isActive;
//	using QUndoStack::isClean;
//	using QUndoStack::cleanIndex;

	using QUndoStack::beginMacro;
	using QUndoStack::endMacro;

	inline DirtyMode dirtyMode(int index) const { return m_dirtyStack.at(index); }
	using QUndoStack::command;

public slots:
	void undo();
	void redo();

private:
	void levelIncrease(int idx);
	void levelDecrease(int idx);

private:
	int m_level;
	QStack<Selection> m_selectionStack;
	QStack<DirtyMode> m_dirtyStack;
	QAction *m_undoAction;
	QAction *m_redoAction;
};

}

#endif // UNDOSTACK_H
