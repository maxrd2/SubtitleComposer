/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2020 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "finderrorsdialog.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>

using namespace SubtitleComposer;

FindErrorsDialog::FindErrorsDialog(QWidget *parent)
	: ActionWithTargetDialog(i18n("Find Error"), parent),
	  m_errorsGroupBox(nullptr),
	  m_errorsCheckBox(nullptr),
	  m_errorsLayout(nullptr)
{
	createErrorsGroupBox(i18nc("@title:group", "Errors to Find"));
	createErrorsButtons(true, translationMode());




	QGroupBox *miscGroupBox = createGroupBox(i18nc("@title:group Miscellaneous settings", "Miscellaneous"));

	m_clearOtherErrorsCheckBox = new QCheckBox(miscGroupBox);
	m_clearOtherErrorsCheckBox->setText(i18n("Clear other errors"));
	m_clearOtherErrorsCheckBox->setChecked(true);

	m_clearMarksCheckBox = new QCheckBox(miscGroupBox);
	m_clearMarksCheckBox->setText(i18n("Clear user marks"));
	m_clearMarksCheckBox->setChecked(false);

	QGridLayout *miscLayout = createLayout(miscGroupBox);
	miscLayout->addWidget(m_clearOtherErrorsCheckBox, 0, 0);
	miscLayout->addWidget(m_clearMarksCheckBox, 1, 0);

	createTargetsGroupBox("Find In");
	createLineTargetsButtonGroup();
	createTextTargetsButtonGroup();
}

FindErrorsDialog::~FindErrorsDialog()
{
	delete[] m_errorsCheckBox;
}

void
FindErrorsDialog::setTranslationMode(bool value)
{
	ActionWithTargetDialog::setTranslationMode(value);
	createErrorsButtons(true, value);
}
QGroupBox *
FindErrorsDialog::createErrorsGroupBox(const QString &title)
{
	m_errorsGroupBox = createGroupBox(title);
	m_errorsLayout = createLayout(m_errorsGroupBox);
	return m_errorsGroupBox;
}

void
FindErrorsDialog::createErrorsButtons(bool showUserMarks, bool showMissingTranslation)
{
	if(m_errorsCheckBox) {
		// no need to recreate everything if the configuration to show has not changed
		if((m_errorsCheckBox[SubtitleLine::UserMarkID] != nullptr) == showUserMarks
		&& (m_errorsCheckBox[SubtitleLine::UntranslatedTextID] != nullptr) == showMissingTranslation)
			return;
	} else {
		m_errorsCheckBox = new QCheckBox *[SubtitleLine::ErrorSIZE];
	}

	if(m_errorsGroupBox) {
		for(QLayoutItem *child = m_errorsLayout->takeAt(0); child != 0; child = m_errorsLayout->takeAt(0))
			delete child;

		QList<QWidget *> children = m_errorsGroupBox->findChildren<QWidget *>();
		for(QList<QWidget *>::ConstIterator it = children.constBegin(), end = children.constEnd(); it != end; ++it)
			delete *it;
	} else {
		createErrorsGroupBox(i18n("Available errors"));
	}

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

	QPushButton *selectAllButton = new QPushButton(m_errorsGroupBox);
	selectAllButton->setText(i18n("Select All"));
	QPushButton *selectNoneButton = new QPushButton(m_errorsGroupBox);
	selectNoneButton->setText(i18n("Select None"));

	connect(selectAllButton, &QAbstractButton::clicked, this, &FindErrorsDialog::selectAllErrorFlags);
	connect(selectNoneButton, &QAbstractButton::clicked, this, &FindErrorsDialog::deselectAllErrorFlags);

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
FindErrorsDialog::selectAllErrorFlags()
{
	for(int errorId = 0; errorId < SubtitleLine::ErrorSIZE; ++errorId)
		if(m_errorsCheckBox[errorId])
			m_errorsCheckBox[errorId]->setChecked(true);
}

void
FindErrorsDialog::deselectAllErrorFlags()
{
	for(int errorId = 0; errorId < SubtitleLine::ErrorSIZE; ++errorId)
		if(m_errorsCheckBox[errorId])
			m_errorsCheckBox[errorId]->setChecked(false);
}

int
FindErrorsDialog::selectedErrorFlags() const
{
	int errorFlags = 0;
	for(int errorId = 0; errorId < SubtitleLine::ErrorSIZE; ++errorId)
		if(m_errorsCheckBox[errorId] && m_errorsCheckBox[errorId]->isChecked())
			errorFlags |= SubtitleLine::errorFlag((SubtitleLine::ErrorID)errorId);

	switch(selectedTextsTarget()) {
	case Primary:
		return errorFlags;
	case Secondary: {
		const int secondaryErrorFlags = (errorFlags &SubtitleLine::PrimaryOnlyErrors) << 1;
		errorFlags = errorFlags & ~SubtitleLine::PrimaryOnlyErrors;
		return errorFlags | secondaryErrorFlags;
	}
	case Both:
	default: {
		const int secondaryErrorFlags = (errorFlags &SubtitleLine::PrimaryOnlyErrors) << 1;
		return errorFlags | secondaryErrorFlags;
	}
	}
}
