#ifndef TRANSLATEDIALOG_H
#define TRANSLATEDIALOG_H

/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "actionwithtargetdialog.h"
#include "utils/language.h"

QT_FORWARD_DECLARE_CLASS(QComboBox)

namespace SubtitleComposer {
class TranslateDialog : public ActionWithTargetDialog
{
public:
	TranslateDialog(QWidget *parent = 0);

	Language::Value inputLanguage() const;
	Language::Value outputLanguage() const;

private:
	QComboBox *m_inputLanguageComboBox;
	QComboBox *m_outputLanguageComboBox;
};
}
#endif
