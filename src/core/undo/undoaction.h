#ifndef UNDOACTION_H
#define UNDOACTION_H

/*
 * SPDX-FileCopyrightText: 2018-2019 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
