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

#include "joinsubtitlesdialog.h"
#include "../../common/commondefs.h"
#include "../../services/player.h"
#include "../../widgets/timeedit.h"

#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QCheckBox>

#include <KLocale>
#include <KPushButton>

using namespace SubtitleComposer;

JoinSubtitlesDialog::JoinSubtitlesDialog(QWidget *parent) :
	SelectableSubtitleDialog(i18n("Join Subtitles"), parent)
{
	createSubtitleGroupBox();

	m_shiftSubtitleCheckBox = new QCheckBox(m_subtitleGroupBox);
	m_shiftSubtitleCheckBox->setText(i18n("Shift subtitle forwards before append"));
	m_shiftSubtitleCheckBox->setChecked(true);

	m_shiftTimeFromVideoButton = new KPushButton(m_subtitleGroupBox);
	m_shiftTimeFromVideoButton->setIcon(KIcon("time-from-video"));
	int buttonSize = m_shiftTimeFromVideoButton->sizeHint().height();
	m_shiftTimeFromVideoButton->setFixedSize(buttonSize, buttonSize);
	m_shiftTimeFromVideoButton->setToolTip(i18n("Set from video length"));

	m_shiftTimeEdit = new TimeEdit(m_subtitleGroupBox);

	connect(m_shiftTimeFromVideoButton, SIGNAL(clicked()), SLOT(setShiftTimeFromVideo()));
	connect(m_shiftSubtitleCheckBox, SIGNAL(toggled(bool)), m_shiftTimeFromVideoButton, SLOT(setEnabled(bool)));
	connect(m_shiftSubtitleCheckBox, SIGNAL(toggled(bool)), m_shiftTimeEdit, SLOT(setEnabled(bool)));

	createTargetsGroupBox();
	createTextTargetsButtonGroup();

	QHBoxLayout *shiftTimeLayout = new QHBoxLayout();
	shiftTimeLayout->addStretch();
	shiftTimeLayout->addWidget(m_shiftTimeFromVideoButton);
	shiftTimeLayout->addWidget(m_shiftTimeEdit);

	m_subtitleLayout->addWidget(m_shiftSubtitleCheckBox, 2, 0, 1, 2);
	m_subtitleLayout->addLayout(shiftTimeLayout, 2, 2);
}

void
JoinSubtitlesDialog::setShiftTimeFromVideo()
{
	m_shiftTimeEdit->setValue((int)(Player::instance()->length() * 1000 + 0.5));
}

Time
JoinSubtitlesDialog::shiftTime() const
{
	return m_shiftSubtitleCheckBox->isChecked() ? m_shiftTimeEdit->value() : 0;
}

void
JoinSubtitlesDialog::show()
{
	m_shiftTimeFromVideoButton->setEnabled(Player::instance()->state() > Player::Opening);
	if(m_shiftTimeFromVideoButton->isEnabled())
		setShiftTimeFromVideo();

	SelectableSubtitleDialog::show();
}

#include "joinsubtitlesdialog.moc"
