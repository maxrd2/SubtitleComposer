/*
 * Copyright (C) 2010-2020 Mladen Milinkovic <max@smoothware.net>
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

#include "application.h"
#include "core/subtitleiterator.h"
#include "errors/finderrorsdialog.h"
#include "errors/errorfinder.h"
#include "gui/treeview/lineswidget.h"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

using namespace SubtitleComposer;

void
Application::selectNextError()
{
	const int idx = m_linesWidget->currentLineIndex();
	if(!m_errorFinder->findNext(idx)) {
		m_lastFoundLine = nullptr;
		m_errorFinder->find(idx, false);
	}
}

void
Application::selectPreviousError()
{
	const int idx = m_linesWidget->currentLineIndex();
	if(!m_errorFinder->findPrevious(idx)) {
		m_lastFoundLine = nullptr;
		m_errorFinder->find(idx, true);
	}
}

void
Application::detectErrors()
{
	m_lastFoundLine = nullptr;
	m_errorFinder->find(m_linesWidget->currentLineIndex());
}

void
Application::clearErrors()
{
	m_subtitle->clearErrors(RangeList(Range::full()), SubtitleLine::AllErrors);
}

void
Application::toggleSelectedLinesMark()
{
	m_subtitle->toggleMarked(m_linesWidget->selectionRanges());
}

