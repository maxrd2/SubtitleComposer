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

#include "fixpunctuationdialog.h"

#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>
#include <QtGui/QGridLayout>

#include <KLocale>

using namespace SubtitleComposer;

FixPunctuationDialog::FixPunctuationDialog(QWidget *parent) :
	ActionWithTargetDialog(i18n("Fix Punctuation"), parent)
{
	QGroupBox *settingsGroupBox = createGroupBox(i18nc("@title:group", "Settings"));

	m_spacesCheckBox = new QCheckBox(settingsGroupBox);
	m_spacesCheckBox->setText(i18n("Fix and cleanup spaces"));
	m_spacesCheckBox->setChecked(true);

	m_quotesCheckBox = new QCheckBox(settingsGroupBox);
	m_quotesCheckBox->setText(i18n("Fix quotes and double quotes"));
	m_quotesCheckBox->setChecked(true);

	m_englishICheckBox = new QCheckBox(settingsGroupBox);
	m_englishICheckBox->setText(i18n("Fix English 'I' pronoun"));
	m_englishICheckBox->setChecked(true);

	m_ellipsisCheckBox = new QCheckBox(settingsGroupBox);
	m_ellipsisCheckBox->setText(i18n("Add ellipisis indicating non finished lines"));
	m_ellipsisCheckBox->setChecked(true);

	createLineTargetsButtonGroup();
	createTextTargetsButtonGroup();

	QGridLayout *settingsLayout = createLayout(settingsGroupBox);
	settingsLayout->addWidget(m_spacesCheckBox, 0, 0);
	settingsLayout->addWidget(m_quotesCheckBox, 1, 0);
	settingsLayout->addWidget(m_englishICheckBox, 2, 0);
	settingsLayout->addWidget(m_ellipsisCheckBox, 3, 0);
}

bool
FixPunctuationDialog::spaces() const
{
	return m_spacesCheckBox->isChecked();
}

bool
FixPunctuationDialog::quotes() const
{
	return m_quotesCheckBox->isChecked();
}

bool
FixPunctuationDialog::englishI() const
{
	return m_englishICheckBox->isChecked();
}

bool
FixPunctuationDialog::ellipisis() const
{
	return m_ellipsisCheckBox->isChecked();
}
