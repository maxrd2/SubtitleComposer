/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "shifttimesdialog.h"
#include "widgets/timeedit.h"

#include <QGroupBox>
#include <QGridLayout>

#include <KComboBox>

using namespace SubtitleComposer;

ShiftTimesDialog::ShiftTimesDialog(QWidget *parent) :
	ActionWithTargetDialog(i18n("Shift"), parent)
{
	QGroupBox *settingsGroupBox = createGroupBox(i18nc("@title:group", "Shifting"));

	m_directionComboBox = new KComboBox(false, settingsGroupBox);
	m_directionComboBox->clear();
	m_directionComboBox->addItem(i18n("Forwards (+)"));
	m_directionComboBox->addItem(i18n("Backwards (−)"));
	m_directionComboBox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

	m_shiftTimeEdit = new TimeEdit(settingsGroupBox);

	createLineTargetsButtonGroup();

	QGridLayout *settingsLayout = createLayout(settingsGroupBox);
	settingsLayout->addWidget(m_directionComboBox, 0, 0);
	settingsLayout->addWidget(m_shiftTimeEdit, 0, 1);
}

void
ShiftTimesDialog::resetShiftTime()
{
	m_shiftTimeEdit->setValue(0);
}

int
ShiftTimesDialog::shiftTimeMillis() const
{
	return m_directionComboBox->currentIndex() == 0 ? m_shiftTimeEdit->value() : -m_shiftTimeEdit->value();
}
