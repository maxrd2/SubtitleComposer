/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "syncsubtitlesdialog.h"

#include <QGroupBox>
#include <QRadioButton>
#include <QGridLayout>

using namespace SubtitleComposer;

SyncSubtitlesDialog::SyncSubtitlesDialog(QWidget *parent) :
	SelectableSubtitleDialog(i18n("Synchronize with Subtitle"), parent)
{
	createSubtitleGroupBox(i18n("Reference Subtitle"));

	QGroupBox *syncModeGroupBox = createGroupBox(i18nc("@title:group", "Synchronization Mode"));

	m_adjustRadioButton = new QRadioButton(syncModeGroupBox);
	m_adjustRadioButton->setText(i18n("Adjust to reference's first and last lines"));
	m_adjustRadioButton->setChecked(true);

	m_synchronizeRadioButton = new QRadioButton(syncModeGroupBox);
	m_synchronizeRadioButton->setText(i18n("Copy timing information from reference line by line"));

	QGridLayout *syncModeLayout = createLayout(syncModeGroupBox);
	syncModeLayout->addWidget(m_adjustRadioButton, 0, 0);
	syncModeLayout->addWidget(m_synchronizeRadioButton, 1, 0);
}

bool
SyncSubtitlesDialog::adjustToReferenceSubtitle() const
{
	return m_adjustRadioButton->isChecked();
}

bool
SyncSubtitlesDialog::synchronizeToReferenceTimes() const
{
	return m_synchronizeRadioButton->isChecked();
}


