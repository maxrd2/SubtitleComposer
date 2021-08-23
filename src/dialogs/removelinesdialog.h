/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef REMOVELINESDIALOG_H
#define REMOVELINESDIALOG_H

#include "actionwithtargetdialog.h"

namespace SubtitleComposer {
class RemoveLinesDialog : public ActionWithTextsTargetDialog
{
public:
	RemoveLinesDialog(QWidget *parent = 0);
};
}
#endif
