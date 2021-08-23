/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>

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

#include "durationlimitsdialog.h"
#include "widgets/timeedit.h"

#include <QLabel>
#include <QCheckBox>
#include <QGroupBox>
#include <QGridLayout>

using namespace SubtitleComposer;

DurationLimitsDialog::DurationLimitsDialog(const Time &minDuration, const Time &maxDuration, QWidget *parent) : ActionWithTargetDialog(i18n("Enforce Duration Limits"), parent)
{
	m_minGroupBox = createGroupBox(i18nc("@title:group", "Minimum Duration"));
	m_minGroupBox->setCheckable(true);

	m_minDurationTimeEdit = new TimeEdit(m_minGroupBox);

	QLabel *minDurationLabel = new QLabel(m_minGroupBox);
	minDurationLabel->setText(i18n("Expand duration to:"));
	minDurationLabel->setBuddy(m_minDurationTimeEdit);

	m_preventOverlapCheckBox = new QCheckBox(m_minGroupBox);
	m_preventOverlapCheckBox->setText(i18n("Prevent overlapping"));
	m_preventOverlapCheckBox->setChecked(true);

	m_maxGroupBox = createGroupBox(i18nc("@title:group", "Maximum Duration"));
	m_maxGroupBox->setCheckable(true);

	m_maxDurationTimeEdit = new TimeEdit(m_maxGroupBox);

	QLabel *maxDurationLabel = new QLabel(m_maxGroupBox);
	maxDurationLabel->setText(i18n("Shrink duration to:"));
	maxDurationLabel->setBuddy(m_maxDurationTimeEdit);

	createLineTargetsButtonGroup();

	QGridLayout *minLayout = createLayout(m_minGroupBox);
	minLayout->addWidget(minDurationLabel, 0, 0, Qt::AlignRight | Qt::AlignVCenter);
	minLayout->addWidget(m_minDurationTimeEdit, 0, 1);
	minLayout->addWidget(m_preventOverlapCheckBox, 1, 0, 1, 2);

	QGridLayout *maxLayout = createLayout(m_maxGroupBox);
	maxLayout->addWidget(maxDurationLabel, 0, 0, Qt::AlignRight | Qt::AlignVCenter);
	maxLayout->addWidget(m_maxDurationTimeEdit, 0, 1);

	connect(m_maxGroupBox, &QGroupBox::toggled, maxDurationLabel, &QWidget::setEnabled);
	connect(m_maxGroupBox, &QGroupBox::toggled, m_maxDurationTimeEdit, &QWidget::setEnabled);

	connect(m_minGroupBox, &QGroupBox::toggled, m_preventOverlapCheckBox, &QWidget::setEnabled);
	connect(m_minGroupBox, &QGroupBox::toggled, minDurationLabel, &QWidget::setEnabled);
	connect(m_minGroupBox, &QGroupBox::toggled, m_minDurationTimeEdit, &QWidget::setEnabled);

	connect(m_maxDurationTimeEdit, &TimeEdit::valueChanged, this, &DurationLimitsDialog::onMaxDurationValueChanged);
	connect(m_minDurationTimeEdit, &TimeEdit::valueChanged, this, &DurationLimitsDialog::onMinDurationValueChanged);

	m_maxDurationTimeEdit->setValue(maxDuration.toMillis());
	m_minDurationTimeEdit->setValue(minDuration.toMillis());
}

void
DurationLimitsDialog::onMaxDurationValueChanged(int value)
{
	if(m_minDurationTimeEdit->value() > value)
		m_minDurationTimeEdit->setValue(value);
}

void
DurationLimitsDialog::onMinDurationValueChanged(int value)
{
	if(m_maxDurationTimeEdit->value() < value)
		m_maxDurationTimeEdit->setValue(value);
}

Time
DurationLimitsDialog::minDuration() const
{
	return Time(m_minDurationTimeEdit->value());
}

Time
DurationLimitsDialog::maxDuration() const
{
	return Time(m_maxDurationTimeEdit->value());
}

bool
DurationLimitsDialog::enforceMaxDuration() const
{
	return m_maxGroupBox->isChecked();
}

bool
DurationLimitsDialog::enforceMinDuration() const
{
	return m_minGroupBox->isChecked();
}

bool
DurationLimitsDialog::preventOverlap() const
{
	return m_preventOverlapCheckBox->isChecked();
}


