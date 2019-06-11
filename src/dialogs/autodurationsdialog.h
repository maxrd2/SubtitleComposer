#ifndef AUTODURATIONSDIALOG_H
#define AUTODURATIONSDIALOG_H

/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

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

	Subtitle::TextTarget calculationMode() const;

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
