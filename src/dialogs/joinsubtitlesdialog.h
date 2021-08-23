/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef JOINSUBTITLESDIALOG_H
#define JOINSUBTITLESDIALOG_H

#include "selectablesubtitledialog.h"
#include "core/time.h"

QT_FORWARD_DECLARE_CLASS(QCheckBox)
QT_FORWARD_DECLARE_CLASS(QPushButton)
class TimeEdit;

namespace SubtitleComposer {
class JoinSubtitlesDialog : public SelectableSubtitleDialog
{
	Q_OBJECT

public:
	explicit JoinSubtitlesDialog(QWidget *parent = 0);

	Time shiftTime() const;

	void show() override;

private slots:
	void setShiftTimeFromVideo();

private:
	TimeEdit *m_shiftTimeEdit;
	QPushButton *m_shiftTimeFromVideoButton;
	QCheckBox *m_shiftSubtitleCheckBox;
};
}
#endif
