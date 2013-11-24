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

#include "checkerrorsdialog.h"

#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>
#include <QtGui/QGridLayout>

#include <KLocale>

using namespace SubtitleComposer;

CheckErrorsDialog::CheckErrorsDialog(QWidget *parent) :
	ActionWithErrorTargetsDialog(i18n("Check Errors"), parent)
{
	createErrorsGroupBox(i18nc("@title:group", "Errors to Check"));
	createErrorsButtons(false, translationMode());

	QGroupBox *miscGroupBox = createGroupBox(i18nc("@title:group Miscellaneous settings", "Miscellaneous"));

	m_clearOtherErrorsCheckBox = new QCheckBox(miscGroupBox);
	m_clearOtherErrorsCheckBox->setText(i18n("Clear other errors"));
	m_clearOtherErrorsCheckBox->setChecked(true);

	m_clearMarksCheckBox = new QCheckBox(miscGroupBox);
	m_clearMarksCheckBox->setText(i18n("Clear user marks"));
	m_clearMarksCheckBox->setChecked(false);

	createLineTargetsButtonGroup();
	createTextTargetsButtonGroup();

	QGridLayout *miscLayout = createLayout(miscGroupBox);
	miscLayout->addWidget(m_clearOtherErrorsCheckBox, 0, 0);
	miscLayout->addWidget(m_clearMarksCheckBox, 1, 0);
}

void
CheckErrorsDialog::setTranslationMode(bool value)
{
	ActionWithErrorTargetsDialog::setTranslationMode(value);
	createErrorsButtons(false, value);
}

bool
CheckErrorsDialog::clearOtherErrors() const
{
	return m_clearOtherErrorsCheckBox->isChecked();
}

bool
CheckErrorsDialog::clearMarks() const
{
	return m_clearMarksCheckBox->isChecked();
}

#include "checkerrorsdialog.moc"
