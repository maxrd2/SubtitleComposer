/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "translatedialog.h"

#include <QFile>
#include <QLabel>
#include <QComboBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QIcon>

using namespace SubtitleComposer;

TranslateDialog::TranslateDialog(QWidget *parent) :
	ActionWithTargetDialog(i18n("Translate"), parent)
{
	QGroupBox *settingsGroupBox = createGroupBox(i18nc("@title:group", "Settings"));

	m_inputLanguageComboBox = new QComboBox(settingsGroupBox);
	m_inputLanguageComboBox->setEditable(false);
	m_inputLanguageComboBox->setIconSize(QSize(21, 13));
	int index = 0;
	for(QList<Language::Value>::ConstIterator it = Language::input().begin(), end = Language::input().end(); it != end; ++it) {
		m_inputLanguageComboBox->addItem(Language::name(*it));
		QString flagPath = Language::flagPath(*it);
		if(!flagPath.isEmpty())
			m_inputLanguageComboBox->setItemIcon(index, QIcon(flagPath));
		index++;
	}

	QLabel *inputLanguageLabel = new QLabel(settingsGroupBox);
	inputLanguageLabel->setText(i18n("Input language:"));
	inputLanguageLabel->setBuddy(m_inputLanguageComboBox);

	m_outputLanguageComboBox = new QComboBox(settingsGroupBox);
	m_outputLanguageComboBox->setEditable(false);
	m_outputLanguageComboBox->setIconSize(QSize(21, 13));
	index = 0;
	for(QList<Language::Value>::ConstIterator it = Language::output().begin(), end = Language::output().end(); it != end; ++it) {
		m_outputLanguageComboBox->addItem(Language::name(*it));
		QString flagPath = Language::flagPath(*it);
		if(!flagPath.isEmpty())
			m_outputLanguageComboBox->setItemIcon(index, QIcon(flagPath));
		index++;
	}

	QLabel *outputLanguageLabel = new QLabel(settingsGroupBox);
	outputLanguageLabel->setText(i18n("Output language:"));
	outputLanguageLabel->setBuddy(m_outputLanguageComboBox);

	createLineTargetsButtonGroup();
	createTextTargetsButtonGroup();

	setTextsTargetEnabled(Both, false);

	QGridLayout *settingsLayout = createLayout(settingsGroupBox);
	settingsLayout->addWidget(inputLanguageLabel, 0, 0, Qt::AlignRight | Qt::AlignVCenter);
	settingsLayout->addWidget(m_inputLanguageComboBox, 0, 1);
	settingsLayout->addWidget(outputLanguageLabel, 1, 0, Qt::AlignRight | Qt::AlignVCenter);
	settingsLayout->addWidget(m_outputLanguageComboBox, 1, 1);
}

Language::Value
TranslateDialog::inputLanguage() const
{
	return Language::input().at(m_inputLanguageComboBox->currentIndex());
}

Language::Value
TranslateDialog::outputLanguage() const
{
	return Language::output().at(m_outputLanguageComboBox->currentIndex());
}
