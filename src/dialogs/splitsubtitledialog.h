/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SPLITSUBTITLEDIALOG_H
#define SPLITSUBTITLEDIALOG_H

#include "actiondialog.h"
#include "core/time.h"

QT_FORWARD_DECLARE_CLASS(QCheckBox)
QT_FORWARD_DECLARE_CLASS(QPushButton)
class TimeEdit;

namespace SubtitleComposer {
class SplitSubtitleDialog : public ActionDialog
{
	Q_OBJECT

public:
	SplitSubtitleDialog(QWidget *parent = 0);

	Time splitTime() const;
	bool shiftNewSubtitle() const;

	void show() override;

private slots:
	void setSplitTimeFromVideo();

private:
	TimeEdit *m_splitTimeEdit;
	QPushButton *m_splitTimeFromVideoButton;
	QCheckBox *m_shiftNewSubtitleCheckBox;
};
}
#endif
