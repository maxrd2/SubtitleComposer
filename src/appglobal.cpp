/*
    SPDX-FileCopyrightText: 2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "appglobal.h"

#include "core/subtitle.h"

using namespace SubtitleComposer;

Application * AppGlobal::app = nullptr;
QExplicitlySharedDataPointer<Subtitle> AppGlobal::subtitle;
UndoStack * AppGlobal::undoStack = nullptr;
