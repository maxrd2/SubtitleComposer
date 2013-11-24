/***************************************************************************
 *   Copyright (C) 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#include "shifttimesdialog.h"
#include "../../widgets/timeedit.h"

#include <QtGui/QGroupBox>
#include <QtGui/QGridLayout>

#include <KLocale>
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
