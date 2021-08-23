/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef AUTODURATIONSDIALOG_H
#define AUTODURATIONSDIALOG_H

#include "actionwithtargetdialog.h"

QT_FORWARD_DECLARE_CLASS(QCheckBox)
QT_FORWARD_DECLARE_CLASS(QSpinBox)
QT_FORWARD_DECLARE_CLASS(QButtonGroup)

namespace SubtitleComposer {
class AutoDurationsDialog : public ActionWithTargetDialog
{
public:
	AutoDurationsDialog(unsigned charMillis, unsigned wordMillis, unsigned lineMillis, QWidget *parent = 0);

	unsigned charMillis() const;
	unsigned wordMillis() const;
	unsigned lineMillis() const;

	bool preventOverlap() const;

	SubtitleTarget calculationMode() const;

	bool translationMode() const;
	void setTranslationMode(bool enabled) override;

private:
	QSpinBox *m_charMillisSpinBox;
	QSpinBox *m_wordMillisSpinBox;
	QSpinBox *m_lineMillisSpinBox;

	QCheckBox *m_preventOverlapCheckBox;

	bool m_translationMode;
	QButtonGroup *m_calculationButtonGroup;
};
}
#endif
