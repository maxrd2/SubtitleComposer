#ifndef UNDOSTACK_H
#define UNDOSTACK_H

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

#include <QUndoStack>

#include <QStack>

QT_FORWARD_DECLARE_CLASS(QItemSelectionModel)

namespace SubtitleComposer {

class UndoStack : public QUndoStack
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
	explicit UndoStack(QObject *parent = nullptr);
	virtual ~UndoStack();

	void clear();
	void push(QUndoCommand *cmd);

	void beginMacro(const QString &text);
	void endMacro();

public slots:
	void undo();
	void redo();

private:
	void levelIncrease(int idx);
	void levelDecrease(int idx);

private:
	int m_level;
	QStack<Selection> m_selectionStack;
};

}

#endif // UNDOSTACK_H
