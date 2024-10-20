/*
    SPDX-FileCopyrightText: 2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "appglobal.h"

#include "core/subtitle.h"

#include "application.h"
#include "gui/playerwidget.h"
#include "mainwindow.h"

using namespace SubtitleComposer;

Application * AppGlobal::app = nullptr;
QExplicitlySharedDataPointer<Subtitle> AppGlobal::subtitle;
UndoStack * AppGlobal::undoStack = nullptr;

VideoPlayer *
SubtitleComposer::videoPlayer()
{
	return app()->mainWindow()->m_playerWidget->m_videoPlayer;
}
