#ifndef UNDOACTION_H
#define UNDOACTION_H

/*
 * Copyright (C) 2018-2019 Mladen Milinkovic <max@smoothware.net>
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

#include <QUndoCommand>

namespace SubtitleComposer {

class Subtitle;

class UndoAction : public QUndoCommand
{
	friend class Subtitle;

public:
	typedef enum {
		None,
		Primary,
		Secondary,
		Both
	} DirtyMode;

	enum {
		// subtitle actions
		SetFramesPerSecond = 1,
		InsertLines,
		RemoveLines,
		MoveLine,
		SwapLinesTexts,

		// subtitle line actions
		SetLinePrimaryText,
		SetLineSecondaryText,
		SetLineTexts,
		SetLineShowTime,
		SetLineHideTime,
		SetLineTimes,
		SetLineErrors,
	} ActionID;


	UndoAction(DirtyMode dirtyMode, Subtitle *subtitle=nullptr, const QString &desc=QString());
	virtual ~UndoAction();

	void redo() override = 0;
	void undo() override;

private:
	const DirtyMode m_dirtyMode;
	Subtitle *m_subtitle;
};

}

#endif /*UNDOACTION_H*/
