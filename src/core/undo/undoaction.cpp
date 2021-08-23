/*
    SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "core/undo/undoaction.h"
#include "core/subtitle.h"

using namespace SubtitleComposer;

UndoAction::UndoAction(UndoStack::DirtyMode dirtyMode, Subtitle *subtitle, const QString &desc)
	: QUndoCommand(desc),
	  m_dirtyMode(dirtyMode),
	  m_subtitle(subtitle)
{
}

UndoAction::~UndoAction()
{
}

void
UndoAction::undo()
{
	redo();
}
