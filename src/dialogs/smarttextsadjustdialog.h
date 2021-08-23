#ifndef SMARTTEXTSADJUSTDIALOG_H
#define SMARTTEXTSADJUSTDIALOG_H

/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "actionwithtargetdialog.h"

QT_FORWARD_DECLARE_CLASS(QSpinBox)

namespace SubtitleComposer {
class SmartTextsAdjustDialog : public ActionWithTargetDialog
{
public:
	explicit SmartTextsAdjustDialog(unsigned minLengthForLineBreak, QWidget *parent = 0);

	unsigned minLengthForLineBreak() const;

private:
	QSpinBox *m_minLengthForLineBreakSpinBox;
};
}
#endif
