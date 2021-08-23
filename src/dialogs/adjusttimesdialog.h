/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef ADJUSTTIMESDIALOG_H
#define ADJUSTTIMESDIALOG_H

#include "actiondialog.h"
#include "core/time.h"

class TimeEdit;

namespace SubtitleComposer {
class AdjustTimesDialog : public ActionDialog
{
public:
	AdjustTimesDialog(QWidget *parent = 0);

	Time firstLineTime() const;
	void setFirstLineTime(const Time &time);

	Time lastLineTime() const;
	void setLastLineTime(const Time &time);

private:
	TimeEdit *m_firstLineTimeEdit;
	TimeEdit *m_lastLineTimeEdit;
};
}
#endif
