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

#include "actionwitherrortargetsdialog.h"

#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>
#include <QtGui/QGridLayout>

#include <KLocale>
#include <KPushButton>

using namespace SubtitleComposer;

ActionWithErrorTargetsDialog::ActionWithErrorTargetsDialog(const QString &title, QWidget *parent) :
	ActionWithTargetDialog(title, parent),
	m_errorsGroupBox(0),
	m_errorsCheckBox(0),
	m_errorsLayout(0)
{}

ActionWithErrorTargetsDialog::~ActionWithErrorTargetsDialog()
{
	delete[] m_errorsCheckBox;
}

QGroupBox *
ActionWithErrorTargetsDialog::createErrorsGroupBox(const QString &title)
{
	m_errorsGroupBox = createGroupBox(title);
	m_errorsLayout = createLayout(m_errorsGroupBox);
	return m_errorsGroupBox;
}

void
ActionWithErrorTargetsDialog::createErrorsButtons(bool showUserMarks, bool showMissingTranslation)
{
	if(m_errorsCheckBox) {
		// no need to recreate everything if the configuration to show has not changed
		if((m_errorsCheckBox[SubtitleLine::UserMarkID] != 0) == showUserMarks && (m_errorsCheckBox[SubtitleLine::UntranslatedTextID] != 0) == showMissingTranslation)
			return;
	} else
		m_errorsCheckBox = new QCheckBox *[SubtitleLine::ErrorSIZE];

	if(m_errorsGroupBox) {
		for(QLayoutItem *child = m_errorsLayout->takeAt(0); child != 0; child = m_errorsLayout->takeAt(0))
			delete child;

		QList<QWidget *> children = m_errorsGroupBox->findChildren<QWidget *>();
		for(QList<QWidget *>::ConstIterator it = children.begin(), end = children.end(); it != end; ++it)
			delete *it;
	} else
		createErrorsGroupBox(i18n("Available errors"));

	int excludedErrorFlags = SubtitleLine::SecondaryOnlyErrors;

	if(showUserMarks)
		excludedErrorFlags &= ~SubtitleLine::UserMark;
	else
		excludedErrorFlags |= SubtitleLine::UserMark;

	if(showMissingTranslation)
		excludedErrorFlags &= ~SubtitleLine::UntranslatedText;
	else
		excludedErrorFlags |= SubtitleLine::UntranslatedText;

	int errorCount = 0;
	for(int errorId = 0; errorId < SubtitleLine::ErrorSIZE; ++errorId) {
		if((0x1 << errorId) & excludedErrorFlags)
			m_errorsCheckBox[errorId] = 0;
		else {
			m_errorsCheckBox[errorId] = new QCheckBox(m_errorsGroupBox);
			m_errorsCheckBox[errorId]->setText(SubtitleLine::simpleErrorText((SubtitleLine::ErrorID)errorId));
			m_errorsCheckBox[errorId]->setChecked(true);
			errorCount++;
		}
	}

	KPushButton *selectAllButton = new KPushButton(m_errorsGroupBox);
	selectAllButton->setText(i18n("Select All"));
	KPushButton *selectNoneButton = new KPushButton(m_errorsGroupBox);
	selectNoneButton->setText(i18n("Select None"));

	connect(selectAllButton, SIGNAL(clicked()), this, SLOT(selectAllErrorFlags()));
	connect(selectNoneButton, SIGNAL(clicked()), this, SLOT(deselectAllErrorFlags()));

	int row = 0, col = 0;
	for(int errorId = 0; errorId < SubtitleLine::ErrorSIZE; ++errorId) {
		if(m_errorsCheckBox[errorId]) {
			m_errorsLayout->addWidget(m_errorsCheckBox[errorId], row++, col);
			if(row > (errorCount - 1) / 2) {
				row = 0;
				col = 1;
			}
		}
	}

	QHBoxLayout *buttonsLayout = new QHBoxLayout();
	buttonsLayout->addStretch();
	buttonsLayout->addWidget(selectAllButton);
	buttonsLayout->addWidget(selectNoneButton);

	m_errorsLayout->addLayout(buttonsLayout, errorCount / 2 + 1, 0, 1, 2);
}

void
ActionWithErrorTargetsDialog::selectAllErrorFlags()
{
	for(int errorId = 0; errorId < SubtitleLine::ErrorSIZE; ++errorId)
		if(m_errorsCheckBox[errorId])
			m_errorsCheckBox[errorId]->setChecked(true);
}

void
ActionWithErrorTargetsDialog::deselectAllErrorFlags()
{
	for(int errorId = 0; errorId < SubtitleLine::ErrorSIZE; ++errorId)
		if(m_errorsCheckBox[errorId])
			m_errorsCheckBox[errorId]->setChecked(false);
}

int
ActionWithErrorTargetsDialog::selectedErrorFlags() const
{
	int errorFlags = 0;
	for(int errorId = 0; errorId < SubtitleLine::ErrorSIZE; ++errorId)
		if(m_errorsCheckBox[errorId] && m_errorsCheckBox[errorId]->isChecked())
			errorFlags |= SubtitleLine::errorFlag((SubtitleLine::ErrorID)errorId);

	switch(selectedTextsTarget()) {
	case SubtitleLine::Primary: {
		return errorFlags;
	} case SubtitleLine::Secondary: {
		int secondaryErrorFlags = (errorFlags &SubtitleLine::PrimaryOnlyErrors) << 1;
		errorFlags = errorFlags & ~SubtitleLine::PrimaryOnlyErrors;
		return errorFlags | secondaryErrorFlags;
	}
	case SubtitleLine::Both:
	default: {
		int secondaryErrorFlags = (errorFlags &SubtitleLine::PrimaryOnlyErrors) << 1;
		return errorFlags | secondaryErrorFlags;
	}
	}
}

#include "actionwitherrortargetsdialog.moc"
