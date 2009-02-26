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

#include "fixoverlappingtimesdialog.h"

#include <QtGui/QLabel>
#include <QtGui/QGroupBox>
#include <QtGui/QGridLayout>

#include <KLocale>
#include <KNumInput>

using namespace SubtitleComposer;

FixOverlappingTimesDialog::FixOverlappingTimesDialog( QWidget* parent ):
	ActionWithTargetDialog( i18n( "Fix Overlapping Times" ), parent )
{
	QGroupBox* settingsGroupBox = createGroupBox( i18nc( "@title:group", "Settings" ) );

	m_minIntervalSpinBox = new KIntNumInput( settingsGroupBox );
	m_minIntervalSpinBox->setSuffix( i18n( " msecs" ) );
	m_minIntervalSpinBox->setMinimum( 1 );
	m_minIntervalSpinBox->setMaximum( 1000 );
	m_minIntervalSpinBox->setValue( 50 );

	QLabel* minIntervalLabel = new QLabel( settingsGroupBox );
	minIntervalLabel->setText( i18n( "Minimum interval between lines:" ) );
	minIntervalLabel->setBuddy( m_minIntervalSpinBox );

	createLineTargetsButtonGroup();

	QGridLayout* settingsLayout = createLayout( settingsGroupBox );
	settingsLayout->addWidget( minIntervalLabel, 0, 0, Qt::AlignRight|Qt::AlignVCenter );
	settingsLayout->addWidget( m_minIntervalSpinBox, 0, 1 );
}

Time FixOverlappingTimesDialog::minimumInterval() const
{
	return Time( m_minIntervalSpinBox->value() );
}

void FixOverlappingTimesDialog::setMinimumInterval( const Time& time )
{
	m_minIntervalSpinBox->setValue( time.toMillis() );
}
