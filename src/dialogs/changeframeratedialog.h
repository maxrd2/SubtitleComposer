/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CHANGEFRAMERATEDIALOG_H
#define CHANGEFRAMERATEDIALOG_H

#include "actiondialog.h"

class KComboBox;

namespace SubtitleComposer {
class ChangeFrameRateDialog : public ActionDialog
{
	Q_OBJECT

public:
	explicit ChangeFrameRateDialog(double fromFramesPerSecond, QWidget *parent = 0);

	double fromFramesPerSecond() const;
	void setFromFramesPerSecond(double framesPerSecond);

	double toFramesPerSecond() const;
	void setNewFramesPerSecond(double framesPerSecond);

private slots:
	void onTextChanged();

private:
	KComboBox *m_fromFramesPerSecondComboBox;
	KComboBox *m_toFramesPerSecondComboBox;
};
}
#endif
