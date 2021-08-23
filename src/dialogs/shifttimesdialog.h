#ifndef SHIFTTIMESDIALOG_H
#define SHIFTTIMESDIALOG_H

/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>
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

#include "actionwithtargetdialog.h"

class TimeEdit;
class KComboBox;

namespace SubtitleComposer {
class ShiftTimesDialog : public ActionWithTargetDialog
{
public:
	ShiftTimesDialog(QWidget *parent = 0);

	void resetShiftTime();

	int shiftTimeMillis() const;

private:
	KComboBox *m_directionComboBox;
	TimeEdit *m_shiftTimeEdit;
};
}
#endif
