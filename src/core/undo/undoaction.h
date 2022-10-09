/*
    SPDX-FileCopyrightText: 2018-2019 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef UNDOACTION_H
#define UNDOACTION_H

#include <QExplicitlySharedDataPointer>
#include <QUndoCommand>

#include "core/undo/undostack.h"

namespace SubtitleComposer {

class Subtitle;

class UndoAction : public QUndoCommand
{
	friend class UndoStack;

public:
	enum {
		// subtitle actions
		SetFramesPerSecond = 1,
		InsertLines,
		RemoveLines,
		MoveLine,
		SwapLinesTexts,
		ChangeStylesheet,

		// subtitle line actions
		SetLinePrimaryText,
		SetLineSecondaryText,
		SetLineTexts,
		SetLineShowTime,
		SetLineHideTime,
		SetLineTimes,
		SetLineErrors,
	} ActionID;


	UndoAction(UndoStack::DirtyMode dirtyMode, Subtitle *subtitle=nullptr, const QString &desc=QString());
	virtual ~UndoAction();

	void redo() override = 0;
	void undo() override;

protected:
	const UndoStack::DirtyMode m_dirtyMode;
	QExplicitlySharedDataPointer<Subtitle> m_subtitle;
};

}

#endif /*UNDOACTION_H*/
