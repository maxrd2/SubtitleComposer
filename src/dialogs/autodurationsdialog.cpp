/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "autodurationsdialog.h"

#include <QLabel>
#include <QCheckBox>
#include <QGroupBox>
#include <QButtonGroup>
#include <QRadioButton>
#include <QGridLayout>
#include <QSpinBox>

using namespace SubtitleComposer;

AutoDurationsDialog::AutoDurationsDialog(unsigned charMillis, unsigned wordMillis, unsigned lineMillis, QWidget *parent)
	: ActionWithTargetDialog(i18n("Set Automatic Durations"), parent),
	m_translationMode(false)
{
	QGroupBox *settingsGroupBox = createGroupBox(i18nc("@title:group", "Settings"));

	m_lineMillisSpinBox = new QSpinBox(settingsGroupBox);
	m_lineMillisSpinBox->setMinimum(0);
	m_lineMillisSpinBox->setMaximum(1000);
	m_lineMillisSpinBox->setValue(lineMillis);

	QLabel *lineMillisLabel = new QLabel(settingsGroupBox);
	lineMillisLabel->setText(i18n("Milliseconds per line:"));
	lineMillisLabel->setBuddy(m_lineMillisSpinBox);

	m_wordMillisSpinBox = new QSpinBox(settingsGroupBox);
	m_wordMillisSpinBox->setMinimum(0);
	m_wordMillisSpinBox->setMaximum(1000);
	m_wordMillisSpinBox->setValue(wordMillis);

	QLabel *wordMillisLabel = new QLabel(settingsGroupBox);
	wordMillisLabel->setText(i18n("Milliseconds per word:"));
	wordMillisLabel->setBuddy(m_wordMillisSpinBox);

	m_charMillisSpinBox = new QSpinBox(settingsGroupBox);
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
	for(int index = 0; index < SubtitleTargetSize; ++index)
		m_calculationButtonGroup->addButton(new QRadioButton(calculationGroupBox), index);

	m_calculationButtonGroup->button(Primary)->setText(i18n("Use primary text"));
	m_calculationButtonGroup->button(Secondary)->setText(i18n("Use translation text"));
	m_calculationButtonGroup->button(Both)->setText(i18n("Calculate both and use maximum"));

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
	for(int index = 0; index < SubtitleTargetSize; ++index)
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
			QAbstractButton *radioButton = m_calculationButtonGroup->button(Both);
			radioButton->setEnabled(true);
			radioButton->setChecked(true);

			radioButton->parentWidget()->show();
		} else {
			QAbstractButton *radioButton = m_calculationButtonGroup->button(Primary);

			radioButton->setEnabled(true);
			radioButton->setChecked(true);

			radioButton->parentWidget()->hide();
		}
	}
}

SubtitleTarget
AutoDurationsDialog::calculationMode() const
{
	int checkedId = m_calculationButtonGroup->checkedId();
	return checkedId == -1 ? SubtitleTargetSize : SubtitleTarget(checkedId);
}
