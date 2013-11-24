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

#include "adjusttimesdialog.h"
#include "../../widgets/timeedit.h"

#include <QtGui/QLabel>
#include <QtGui/QGroupBox>
#include <QtGui/QGridLayout>

#include <KLocale>

using namespace SubtitleComposer;

AdjustTimesDialog::AdjustTimesDialog(QWidget *parent) :
	ActionDialog(i18n("Adjust"), parent)
{
	QGroupBox *settingsGroupBox = createGroupBox(i18nc("@title:group", "New Times"));

	m_firstLineTimeEdit = new TimeEdit(settingsGroupBox);

	QLabel *firstLineLabel = new QLabel(settingsGroupBox);
	firstLineLabel->setText(i18n("First spoken line:"));
	firstLineLabel->setBuddy(m_firstLineTimeEdit);

	m_lastLineTimeEdit = new TimeEdit(settingsGroupBox);

	QLabel *lastLineLabel = new QLabel(settingsGroupBox);
	lastLineLabel->setText(i18n("Last spoken line:"));
	lastLineLabel->setBuddy(m_lastLineTimeEdit);

	QGridLayout *settingsLayout = createLayout(settingsGroupBox);
	settingsLayout->addWidget(firstLineLabel, 0, 0, Qt::AlignRight | Qt::AlignVCenter);
	settingsLayout->addWidget(m_firstLineTimeEdit, 0, 1);
	settingsLayout->addWidget(lastLineLabel, 1, 0, Qt::AlignRight | Qt::AlignVCenter);
	settingsLayout->addWidget(m_lastLineTimeEdit, 1, 1);
}

Time
AdjustTimesDialog::firstLineTime() const
{
	return Time(m_firstLineTimeEdit->value());
}

void
AdjustTimesDialog::setFirstLineTime(const Time &time)
{
	m_firstLineTimeEdit->setValue(time.toMillis());
}

Time
AdjustTimesDialog::lastLineTime() const
{
	return Time(m_lastLineTimeEdit->value());
}

void
AdjustTimesDialog::setLastLineTime(const Time &time)
{
	m_lastLineTimeEdit->setValue(time.toMillis());
}
