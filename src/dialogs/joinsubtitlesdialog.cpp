/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "joinsubtitlesdialog.h"
#include "helpers/commondefs.h"
#include "videoplayer/videoplayer.h"
#include "widgets/timeedit.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <QIcon>
#include <QGridLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QPushButton>

using namespace SubtitleComposer;

JoinSubtitlesDialog::JoinSubtitlesDialog(QWidget *parent) :
	SelectableSubtitleDialog(i18n("Join Subtitles"), parent)
{
	createSubtitleGroupBox();

	m_shiftSubtitleCheckBox = new QCheckBox(m_subtitleGroupBox);
	m_shiftSubtitleCheckBox->setText(i18n("Shift subtitle forwards before append"));
	m_shiftSubtitleCheckBox->setChecked(true);

	m_shiftTimeFromVideoButton = new QPushButton(m_subtitleGroupBox);
	m_shiftTimeFromVideoButton->setIcon(QIcon::fromTheme(QStringLiteral("time_from_video")));
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
	m_shiftTimeEdit->setValue((int)(VideoPlayer::instance()->length() * 1000 + 0.5));
}

Time
JoinSubtitlesDialog::shiftTime() const
{
	return m_shiftSubtitleCheckBox->isChecked() ? m_shiftTimeEdit->value() : 0;
}

void
JoinSubtitlesDialog::show()
{
	m_shiftTimeFromVideoButton->setEnabled(VideoPlayer::instance()->state() > VideoPlayer::Opening);
	if(m_shiftTimeFromVideoButton->isEnabled())
		setShiftTimeFromVideo();

	SelectableSubtitleDialog::show();
}


