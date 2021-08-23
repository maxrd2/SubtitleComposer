/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef FIXPUNCTUATIONDIALOG_H
#define FIXPUNCTUATIONDIALOG_H

#include "actionwithtargetdialog.h"

QT_FORWARD_DECLARE_CLASS(QCheckBox)

namespace SubtitleComposer {
class FixPunctuationDialog : public ActionWithTargetDialog
{
public:
	FixPunctuationDialog(QWidget *parent = 0);

	bool spaces() const;
	bool quotes() const;
	bool englishI() const;
	bool ellipsis() const;

private:
	QCheckBox *m_spacesCheckBox;
	QCheckBox *m_quotesCheckBox;
	QCheckBox *m_englishICheckBox;
	QCheckBox *m_ellipsisCheckBox;
};
}
#endif
