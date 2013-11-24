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

#include "splitsubtitledialog.h"
#include "../../common/commondefs.h"
#include "../../services/player.h"
#include "../../widgets/timeedit.h"

#include <QtGui/QLabel>
#include <QtGui/QGroupBox>
#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>

#include <KLocale>
#include <KPushButton>

using namespace SubtitleComposer;

SplitSubtitleDialog::SplitSubtitleDialog(QWidget *parent) :
	ActionDialog(i18n("Split Subtitle"), parent)
{
	QGroupBox *settingsGroupBox = createGroupBox(i18nc("@title:group", "Settings"));

	m_splitTimeFromVideoButton = new KPushButton(settingsGroupBox);
	m_splitTimeFromVideoButton->setIcon(KIcon("time-from-video"));
	int buttonSize = m_splitTimeFromVideoButton->sizeHint().height();
	m_splitTimeFromVideoButton->setFixedSize(buttonSize, buttonSize);
	m_splitTimeFromVideoButton->setToolTip(i18n("Set from video length"));

	m_splitTimeEdit = new TimeEdit(settingsGroupBox);

	QLabel *splitTimeLabel = new QLabel(settingsGroupBox);
	splitTimeLabel->setText(i18n("Split time:"));
	splitTimeLabel->setBuddy(m_splitTimeEdit);

	m_shiftNewSubtitleCheckBox = new QCheckBox(settingsGroupBox);
	m_shiftNewSubtitleCheckBox->setText(i18n("Shift new part backwards after split"));
	m_shiftNewSubtitleCheckBox->setChecked(true);

	QHBoxLayout *splitTimeLayout = new QHBoxLayout();
	splitTimeLayout->addWidget(m_splitTimeFromVideoButton);
	splitTimeLayout->addWidget(m_splitTimeEdit);

	QGridLayout *settingsLayout = createLayout(settingsGroupBox);
	settingsLayout->addWidget(splitTimeLabel, 0, 0, Qt::AlignRight | Qt::AlignVCenter);
	settingsLayout->addLayout(splitTimeLayout, 0, 1);
	settingsLayout->addWidget(m_shiftNewSubtitleCheckBox, 1, 0, 1, 2);

	connect(m_splitTimeFromVideoButton, SIGNAL(clicked()), SLOT(setSplitTimeFromVideo()));
}

void
SplitSubtitleDialog::setSplitTimeFromVideo()
{
	m_splitTimeEdit->setValue((int)(Player::instance()->length() * 1000 + 0.5));
}

Time
SplitSubtitleDialog::splitTime() const
{
	return m_splitTimeEdit->value();
}

bool
SplitSubtitleDialog::shiftNewSubtitle() const
{
	return m_shiftNewSubtitleCheckBox->isChecked();
}

void
SplitSubtitleDialog::show()
{
	m_splitTimeFromVideoButton->setEnabled(Player::instance()->state() > Player::Opening);
	if(m_splitTimeFromVideoButton->isEnabled())
		setSplitTimeFromVideo();

	ActionDialog::show();
}

#include "splitsubtitledialog.moc"
