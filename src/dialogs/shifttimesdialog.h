/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SHIFTTIMESDIALOG_H
#define SHIFTTIMESDIALOG_H

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
