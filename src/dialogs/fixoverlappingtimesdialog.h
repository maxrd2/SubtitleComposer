#ifndef FIXOVERLAPPINGTIMESDIALOG_H
#define FIXOVERLAPPINGTIMESDIALOG_H

/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "actionwithtargetdialog.h"
#include "core/time.h"

QT_FORWARD_DECLARE_CLASS(QSpinBox)

namespace SubtitleComposer {
class FixOverlappingTimesDialog : public ActionWithTargetDialog
{
public:
	FixOverlappingTimesDialog(QWidget *parent = 0);

	Time minimumInterval() const;
	void setMinimumInterval(const Time &time);

private:
	QSpinBox *m_minIntervalSpinBox;
};
}
#endif
