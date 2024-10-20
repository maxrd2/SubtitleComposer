/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "config.h"

#include "appglobal.h"
#include "splitsubtitledialog.h"
#include "helpers/commondefs.h"
#include "videoplayer/videoplayer.h"
#include "widgets/timeedit.h"

#include <QLabel>
#include <QGroupBox>
#include <QCheckBox>
#include <QGridLayout>
#include <QIcon>
#include <QPushButton>

#include <KLocalizedString>

using namespace SubtitleComposer;

SplitSubtitleDialog::SplitSubtitleDialog(QWidget *parent) :
	ActionDialog(i18n("Split Subtitle"), parent)
{
	QGroupBox *settingsGroupBox = createGroupBox(i18nc("@title:group", "Settings"));

	m_splitTimeFromVideoButton = new QPushButton(settingsGroupBox);
	m_splitTimeFromVideoButton->setIcon(QIcon::fromTheme(QStringLiteral("time_from_video")));
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

	connect(m_splitTimeFromVideoButton, &QAbstractButton::clicked, this, &SplitSubtitleDialog::setSplitTimeFromVideo);
}

void
SplitSubtitleDialog::setSplitTimeFromVideo()
{
	m_splitTimeEdit->setValue(static_cast<int>(videoPlayer()->duration() * 1000 + 0.5));
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
	m_splitTimeFromVideoButton->setEnabled(videoPlayer()->state() > VideoPlayer::Opening);
	if(m_splitTimeFromVideoButton->isEnabled())
		setSplitTimeFromVideo();

	ActionDialog::show();
}


