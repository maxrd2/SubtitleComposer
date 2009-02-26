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

#include "durationlimitsdialog.h"
#include "../../widgets/timeedit.h"

#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>
#include <QtGui/QGridLayout>

#include <KLocale>

using namespace SubtitleComposer;

DurationLimitsDialog::DurationLimitsDialog( const Time& minDuration, const Time& maxDuration, QWidget* parent ):
	ActionWithTargetDialog( i18n( "Enforce Duration Limits" ), parent )
{
	m_minGroupBox = createGroupBox( i18nc( "@title:group", "Minimum Duration" ) );
	m_minGroupBox->setCheckable( true );

	m_minDurationTimeEdit = new TimeEdit( m_minGroupBox );

	QLabel* minDurationLabel = new QLabel( m_minGroupBox );
	minDurationLabel->setText( i18n( "Expand duration to:" ) );
	minDurationLabel->setBuddy( m_minDurationTimeEdit );

	m_preventOverlapCheckBox = new QCheckBox( m_minGroupBox );
	m_preventOverlapCheckBox->setText( i18n( "Prevent overlapping" ) );
	m_preventOverlapCheckBox->setChecked( true );

	m_maxGroupBox = createGroupBox( i18nc( "@title:group", "Maximum Duration" ) );
	m_maxGroupBox->setCheckable( true );

	m_maxDurationTimeEdit = new TimeEdit( m_maxGroupBox );

	QLabel* maxDurationLabel = new QLabel( m_maxGroupBox );
	maxDurationLabel->setText( i18n( "Shrink duration to:" ) );
	maxDurationLabel->setBuddy( m_maxDurationTimeEdit );

	createLineTargetsButtonGroup();

	QGridLayout* minLayout = createLayout( m_minGroupBox );
	minLayout->addWidget( minDurationLabel, 0, 0, Qt::AlignRight|Qt::AlignVCenter );
	minLayout->addWidget( m_minDurationTimeEdit, 0, 1 );
	minLayout->addWidget( m_preventOverlapCheckBox, 1, 0, 1, 2 );

	QGridLayout* maxLayout = createLayout( m_maxGroupBox );
	maxLayout->addWidget( maxDurationLabel, 0, 0, Qt::AlignRight|Qt::AlignVCenter );
	maxLayout->addWidget( m_maxDurationTimeEdit, 0, 1 );

	connect( m_maxGroupBox, SIGNAL( toggled(bool) ), maxDurationLabel, SLOT( setEnabled(bool) ) );
	connect( m_maxGroupBox, SIGNAL( toggled(bool) ), m_maxDurationTimeEdit, SLOT( setEnabled(bool) ) );

	connect( m_minGroupBox, SIGNAL( toggled(bool) ), m_preventOverlapCheckBox, SLOT( setEnabled(bool) ) );
	connect( m_minGroupBox, SIGNAL( toggled(bool) ), minDurationLabel, SLOT( setEnabled(bool) ) );
	connect( m_minGroupBox, SIGNAL( toggled(bool) ), m_minDurationTimeEdit, SLOT( setEnabled(bool) ) );

	connect( m_maxDurationTimeEdit, SIGNAL( valueChanged(int) ), this, SLOT( onMaxDurationValueChanged(int) ) );
	connect( m_minDurationTimeEdit, SIGNAL( valueChanged(int) ), this, SLOT( onMinDurationValueChanged(int) ) );

	m_maxDurationTimeEdit->setValue( maxDuration.toMillis() );
	m_minDurationTimeEdit->setValue( minDuration.toMillis() );
}

void DurationLimitsDialog::onMaxDurationValueChanged( int value )
{
	if ( m_minDurationTimeEdit->value() > value )
		m_minDurationTimeEdit->setValue( value );
}

void DurationLimitsDialog::onMinDurationValueChanged( int value )
{
	if ( m_maxDurationTimeEdit->value() < value )
		m_maxDurationTimeEdit->setValue( value );
}

Time DurationLimitsDialog::minDuration() const
{
	return Time( m_minDurationTimeEdit->value() );
}

Time DurationLimitsDialog::maxDuration() const
{
	return Time( m_maxDurationTimeEdit->value() );
}

bool DurationLimitsDialog::enforceMaxDuration() const
{
	return m_maxGroupBox->isChecked();
}

bool DurationLimitsDialog::enforceMinDuration() const
{
	return m_minGroupBox->isChecked();
}

bool DurationLimitsDialog::preventOverlap() const
{
	return m_preventOverlapCheckBox->isChecked();
}

#include "durationlimitsdialog.moc"
