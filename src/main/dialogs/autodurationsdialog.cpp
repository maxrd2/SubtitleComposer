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

#include "autodurationsdialog.h"

#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>
#include <QtGui/QButtonGroup>
#include <QtGui/QRadioButton>
#include <QtGui/QGridLayout>

#include <KLocale>
#include <KNumInput>

using namespace SubtitleComposer;

AutoDurationsDialog::AutoDurationsDialog(unsigned charMillis, unsigned wordMillis, unsigned lineMillis, QWidget *parent) :
	ActionWithTargetDialog(i18n("Set Automatic Durations"), parent),
	m_translationMode(false)
{
	QGroupBox *settingsGroupBox = createGroupBox(i18nc("@title:group", "Settings"));

	m_lineMillisSpinBox = new KIntSpinBox(settingsGroupBox);
	m_lineMillisSpinBox->setMinimum(0);
	m_lineMillisSpinBox->setMaximum(1000);
	m_lineMillisSpinBox->setValue(lineMillis);

	QLabel *lineMillisLabel = new QLabel(settingsGroupBox);
	lineMillisLabel->setText(i18n("Milliseconds per line:"));
	lineMillisLabel->setBuddy(m_lineMillisSpinBox);

	m_wordMillisSpinBox = new KIntSpinBox(settingsGroupBox);
	m_wordMillisSpinBox->setMinimum(0);
	m_wordMillisSpinBox->setMaximum(1000);
	m_wordMillisSpinBox->setValue(wordMillis);

	QLabel *wordMillisLabel = new QLabel(settingsGroupBox);
	wordMillisLabel->setText(i18n("Milliseconds per word:"));
	wordMillisLabel->setBuddy(m_wordMillisSpinBox);

	m_charMillisSpinBox = new KIntSpinBox(settingsGroupBox);
	m_charMillisSpinBox->setMinimum(0);
	m_charMillisSpinBox->setMaximum(1000);
	m_charMillisSpinBox->setValue(charMillis);

	QLabel *charMillisLabel = new QLabel(settingsGroupBox);
	charMillisLabel->setText(i18n("Milliseconds per character:"));
	charMillisLabel->setBuddy(m_charMillisSpinBox);

	m_preventOverlapCheckBox = new QCheckBox(settingsGroupBox);
	m_preventOverlapCheckBox->setText(i18n("Prevent overlapping"));
	m_preventOverlapCheckBox->setChecked(true);

	QGroupBox *calculationGroupBox = createGroupBox(i18nc("@title:group", "Duration Calculation"));

	m_calculationButtonGroup = new QButtonGroup(this);
	for(int index = 0; index < SubtitleLine::TextTargetSIZE; ++index)
		m_calculationButtonGroup->addButton(new QRadioButton(calculationGroupBox), index);

	m_calculationButtonGroup->button(SubtitleLine::Primary)->setText(i18n("Use primary text"));
	m_calculationButtonGroup->button(SubtitleLine::Secondary)->setText(i18n("Use translation text"));
	m_calculationButtonGroup->button(SubtitleLine::Both)->setText(i18n("Calculate both and use maximum"));

	createLineTargetsButtonGroup();

	QGridLayout *settingsGroupBoxLayout = createLayout(settingsGroupBox);
	settingsGroupBoxLayout->addWidget(lineMillisLabel, 0, 0, Qt::AlignRight | Qt::AlignVCenter);
	settingsGroupBoxLayout->addWidget(m_lineMillisSpinBox, 0, 1);
	settingsGroupBoxLayout->addWidget(wordMillisLabel, 1, 0, Qt::AlignRight | Qt::AlignVCenter);
	settingsGroupBoxLayout->addWidget(m_wordMillisSpinBox, 1, 1);
	settingsGroupBoxLayout->addWidget(charMillisLabel, 2, 0, Qt::AlignRight | Qt::AlignVCenter);
	settingsGroupBoxLayout->addWidget(m_charMillisSpinBox, 2, 1);
	settingsGroupBoxLayout->addWidget(m_preventOverlapCheckBox, 3, 0, 1, 2);

	QGridLayout *calculationLayout = createLayout(calculationGroupBox);
	for(int index = 0; index < SubtitleLine::TextTargetSIZE; ++index)
		calculationLayout->addWidget(m_calculationButtonGroup->button(index), index, 0);
}

unsigned
AutoDurationsDialog::charMillis() const
{
	return m_charMillisSpinBox->value();
}

unsigned
AutoDurationsDialog::wordMillis() const
{
	return m_wordMillisSpinBox->value();
}

unsigned
AutoDurationsDialog::lineMillis() const
{
	return m_lineMillisSpinBox->value();
}

bool
AutoDurationsDialog::preventOverlap() const
{
	return m_preventOverlapCheckBox->isChecked();
}

bool
AutoDurationsDialog::translationMode() const
{
	return m_translationMode;
}

void
AutoDurationsDialog::setTranslationMode(bool enabled)
{
	if(m_translationMode != enabled) {
		m_translationMode = enabled;

		if(m_translationMode) {
			QAbstractButton *radioButton = m_calculationButtonGroup->button(SubtitleLine::Both);
			radioButton->setEnabled(true);
			radioButton->setChecked(true);

			radioButton->parentWidget()->show();
		} else {
			QAbstractButton *radioButton = m_calculationButtonGroup->button(SubtitleLine::Primary);

			radioButton->setEnabled(true);
			radioButton->setChecked(true);

			radioButton->parentWidget()->hide();
		}
	}
}

Subtitle::TextTarget
AutoDurationsDialog::calculationMode() const
{
	int checkedId = m_calculationButtonGroup->checkedId();
	return checkedId == -1 ? Subtitle::TextTargetSIZE : (Subtitle::TextTarget)checkedId;
}
