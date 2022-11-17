/*
    SPDX-FileCopyrightText: 2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef APPGLOBAL_H
#define APPGLOBAL_H

#include <QExplicitlySharedDataPointer>

namespace SubtitleComposer {
class Application;
class Subtitle;
class UndoStack;

class AppGlobal
{
	friend class Application;
	friend Application * app();
	friend Subtitle *appSubtitle();
	friend UndoStack * appUndoStack();

	static Application *app;
	static QExplicitlySharedDataPointer<Subtitle> subtitle;
	static UndoStack *undoStack;
};

inline Application * app() { return AppGlobal::app; }
inline Subtitle * appSubtitle() { return AppGlobal::subtitle.data(); }
inline UndoStack * appUndoStack() { return AppGlobal::undoStack; }

}

#endif // APPGLOBAL_H
