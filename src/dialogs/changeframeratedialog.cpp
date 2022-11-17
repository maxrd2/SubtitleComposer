/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "changeframeratedialog.h"

#include <QLabel>
#include <KLineEdit>
#include <QGroupBox>
#include <QGridLayout>

#include <KMessageBox>
#include <KComboBox>
#include <KLocalizedString>

using namespace SubtitleComposer;

ChangeFrameRateDialog::ChangeFrameRateDialog(double fromFramesPerSecond, QWidget *parent) :
	ActionDialog(i18n("Change Frame Rate"), parent)
{
	QGroupBox *settingsGroupBox = createGroupBox(i18nc("@title:group", "Settings"));

	m_fromFramesPerSecondComboBox = new KComboBox(false, settingsGroupBox);
	m_fromFramesPerSecondComboBox->setEditable(true);
	m_fromFramesPerSecondComboBox->addItems(QStringLiteral("15 20 23.976 24 25 29.970 30").split(' '));
	m_fromFramesPerSecondComboBox->setCurrentIndex(2);
	setFromFramesPerSecond(fromFramesPerSecond);

	QLabel *fromFramesPerSecondLabel = new QLabel(settingsGroupBox);
	fromFramesPerSecondLabel->setText(i18n("Current frame rate:"));
	fromFramesPerSecondLabel->setBuddy(m_fromFramesPerSecondComboBox);

	m_toFramesPerSecondComboBox = new KComboBox(false, settingsGroupBox);
	m_toFramesPerSecondComboBox->setEditable(true);
	m_toFramesPerSecondComboBox->addItems(QStringLiteral("15 20 23.976 24 25 29.970 30").split(' '));
	m_toFramesPerSecondComboBox->setCurrentIndex(2);

	QLabel *toFramesPerSecondLabel = new QLabel(settingsGroupBox);
	toFramesPerSecondLabel->setText(i18n("New frame rate:"));
	toFramesPerSecondLabel->setBuddy(m_toFramesPerSecondComboBox);

	connect(m_fromFramesPerSecondComboBox, &QComboBox::editTextChanged, this, &ChangeFrameRateDialog::onTextChanged);
	connect(m_toFramesPerSecondComboBox, &QComboBox::editTextChanged, this, &ChangeFrameRateDialog::onTextChanged);

	QGridLayout *settingsLayout = createLayout(settingsGroupBox);
	settingsLayout->addWidget(fromFramesPerSecondLabel, 0, 0, Qt::AlignRight | Qt::AlignVCenter);
	settingsLayout->addWidget(m_fromFramesPerSecondComboBox, 0, 1);
	settingsLayout->addWidget(toFramesPerSecondLabel, 1, 0, Qt::AlignRight | Qt::AlignVCenter);
	settingsLayout->addWidget(m_toFramesPerSecondComboBox, 1, 1);

	setMinimumWidth(sizeHint().width() + 25);
}

double
ChangeFrameRateDialog::fromFramesPerSecond() const
{
	return m_fromFramesPerSecondComboBox->lineEdit()->text().toDouble();
}

void
ChangeFrameRateDialog::setFromFramesPerSecond(double framesPerSecond)
{
	m_fromFramesPerSecondComboBox->lineEdit()->setText(QString::number(framesPerSecond, 'f', 3));
}

double
ChangeFrameRateDialog::toFramesPerSecond() const
{
	return m_toFramesPerSecondComboBox->lineEdit()->text().toDouble();
}

void
ChangeFrameRateDialog::setNewFramesPerSecond(double framesPerSecond)
{
	m_toFramesPerSecondComboBox->lineEdit()->setText(QString::number(framesPerSecond, 'f', 3));
}

void
ChangeFrameRateDialog::onTextChanged()
{
	bool applyButtonEnabled;
	m_fromFramesPerSecondComboBox->lineEdit()->text().toDouble(&applyButtonEnabled);
	if(applyButtonEnabled)
		m_toFramesPerSecondComboBox->lineEdit()->text().toDouble(&applyButtonEnabled);

	m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(applyButtonEnabled);
}


