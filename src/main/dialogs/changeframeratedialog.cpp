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

#include "changeframeratedialog.h"

#include <QtGui/QLabel>
#include <KLineEdit>
#include <QtGui/QGroupBox>
#include <QtGui/QGridLayout>

#include <KLocale>
#include <KMessageBox>
#include <KComboBox>

using namespace SubtitleComposer;

ChangeFrameRateDialog::ChangeFrameRateDialog(double fromFramesPerSecond, QWidget *parent) :
	ActionDialog(i18n("Change Frame Rate"), parent)
{
	QGroupBox *settingsGroupBox = createGroupBox(i18nc("@title:group", "Settings"));

	m_fromFramesPerSecondComboBox = new KComboBox(false, settingsGroupBox);
	m_fromFramesPerSecondComboBox->setEditable(true);
	m_fromFramesPerSecondComboBox->addItems(QString("15 20 23.976 24 25 29.970 30").split(' '));
	m_fromFramesPerSecondComboBox->setCurrentIndex(2);
	setFromFramesPerSecond(fromFramesPerSecond);

	QLabel *fromFramesPerSecondLabel = new QLabel(settingsGroupBox);
	fromFramesPerSecondLabel->setText(i18n("Current frame rate:"));
	fromFramesPerSecondLabel->setBuddy(m_fromFramesPerSecondComboBox);

	m_toFramesPerSecondComboBox = new KComboBox(false, settingsGroupBox);
	m_toFramesPerSecondComboBox->setEditable(true);
	m_toFramesPerSecondComboBox->addItems(QString("15 20 23.976 24 25 29.970 30").split(' '));
	m_toFramesPerSecondComboBox->setCurrentIndex(2);

	QLabel *toFramesPerSecondLabel = new QLabel(settingsGroupBox);
	toFramesPerSecondLabel->setText(i18n("New frame rate:"));
	toFramesPerSecondLabel->setBuddy(m_toFramesPerSecondComboBox);

	connect(m_fromFramesPerSecondComboBox, SIGNAL(editTextChanged(const QString &)), this, SLOT(onTextChanged()));
	connect(m_toFramesPerSecondComboBox, SIGNAL(editTextChanged(const QString &)), this, SLOT(onTextChanged()));

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

	enableButton(KDialog::Ok, applyButtonEnabled);
}

#include "changeframeratedialog.moc"
