/*
    SPDX-FileCopyrightText: 2010-2020 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "config.h"

#include "application.h"
#include "core/subtitleiterator.h"
#include "errors/finderrorsdialog.h"
#include "errors/errorfinder.h"
#include "gui/treeview/lineswidget.h"

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

