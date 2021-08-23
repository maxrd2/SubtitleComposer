#ifndef CHANGETEXTSCASEDIALOG_H
#define CHANGETEXTSCASEDIALOG_H

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

QT_FORWARD_DECLARE_CLASS(QCheckBox)
QT_FORWARD_DECLARE_CLASS(QRadioButton)

namespace SubtitleComposer {
class ChangeTextsCaseDialog : public ActionWithTargetDialog
{
	Q_OBJECT

public:
	typedef enum { Upper = 0, Lower, Title, Sentence } CaseOp;

	ChangeTextsCaseDialog(QWidget *parent = 0);

	CaseOp caseOperation() const;
	bool lowerFirst() const;

private slots:
	void onCaseButtonGroupClicked(int id);

private:
	QCheckBox *m_lowerFirstCheckBox;

	QRadioButton *m_lowerRadioButton;
	QRadioButton *m_upperRadioButton;
	QRadioButton *m_titleRadioButton;
	QRadioButton *m_sentenceRadioButton;
};
}
#endif
