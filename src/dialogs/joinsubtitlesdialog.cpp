/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "config.h"

#include "joinsubtitlesdialog.h"
#include "helpers/commondefs.h"
#include "videoplayer/videoplayer.h"
#include "widgets/timeedit.h"

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

	connect(m_shiftTimeFromVideoButton, &QAbstractButton::clicked, this, &JoinSubtitlesDialog::setShiftTimeFromVideo);
	connect(m_shiftSubtitleCheckBox, &QAbstractButton::toggled, m_shiftTimeFromVideoButton, &QWidget::setEnabled);
	connect(m_shiftSubtitleCheckBox, &QAbstractButton::toggled, m_shiftTimeEdit, &QWidget::setEnabled);

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
	m_shiftTimeEdit->setValue((int)(VideoPlayer::instance()->duration() * 1000 + 0.5));
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


