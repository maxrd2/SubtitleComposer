#ifndef DURATIONLIMITSDIALOG_H
#define DURATIONLIMITSDIALOG_H

/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "actionwithtargetdialog.h"
#include "core/time.h"

QT_FORWARD_DECLARE_CLASS(QGroupBox)
QT_FORWARD_DECLARE_CLASS(QCheckBox)
QT_FORWARD_DECLARE_CLASS(QCheckBox)
class TimeEdit;

namespace SubtitleComposer {
class DurationLimitsDialog : public ActionWithTargetDialog
{
	Q_OBJECT

public:
	DurationLimitsDialog(const Time &minDuration, const Time &maxDuration, QWidget *parent = 0);

	Time minDuration() const;
	Time maxDuration() const;

	bool enforceMaxDuration() const;
	bool enforceMinDuration() const;
	bool preventOverlap() const;

private slots:
	void onMinDurationValueChanged(int value);
	void onMaxDurationValueChanged(int value);

private:
	QGroupBox *m_maxGroupBox;
	TimeEdit *m_maxDurationTimeEdit;

	QGroupBox *m_minGroupBox;
	QCheckBox *m_preventOverlapCheckBox;
	TimeEdit *m_minDurationTimeEdit;
};
}
#endif
