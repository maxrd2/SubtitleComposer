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

#include "generalconfigwidget.h"
#include "../application.h"

#include <QtCore/QTextCodec>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QCheckBox>
#include <QtGui/QLabel>

#include <KLocale>
#include <KComboBox>
#include <KNumInput>

using namespace SubtitleComposer;

GeneralConfigWidget::GeneralConfigWidget( QWidget* parent ):
	AppConfigGroupWidget( new GeneralConfig(), parent )
{
	QGroupBox* generalGroupBox = createGroupBox( i18nc( "@title:group General settings", "General" ) );

	QLabel* defaultEncodingLabel = new QLabel( generalGroupBox );
	defaultEncodingLabel->setText( i18n( "Default encoding for opening subtitles:" ) );

	m_defaultEncodingComboBox = new KComboBox( generalGroupBox );
	m_defaultEncodingComboBox->addItems( app()->availableEncodingNames() );

	QLabel* relativeSeekPositionLabel = new QLabel( generalGroupBox );
	relativeSeekPositionLabel->setText( i18n( "On line double click, jump to show time minus:" ) );

	m_relativeSeekPositionSpinBox = new KIntNumInput( generalGroupBox );
	m_relativeSeekPositionSpinBox->setMinimum( 0 );
	m_relativeSeekPositionSpinBox->setMaximum( 10000 );
	m_relativeSeekPositionSpinBox->setSuffix( i18n( " msecs" ) );

	m_autoLoadVideoCheckBox = new QCheckBox( generalGroupBox );
	m_autoLoadVideoCheckBox->setText( i18n( "Automatically load video file when opening subtitle" ) );


	QGroupBox* quickActionsGroupBox = createGroupBox( i18nc( "@title:group", "Quick Actions" ) );

	QLabel* shiftMillisecondsLabel = new QLabel( quickActionsGroupBox );
	shiftMillisecondsLabel->setText( i18n( "Time shift for selected lines:" ) );

	m_shiftMsecsSpinBox = new KIntNumInput( quickActionsGroupBox );
	m_shiftMsecsSpinBox->setMinimum( 1 );
	m_shiftMsecsSpinBox->setMaximum( 1000 );
	m_shiftMsecsSpinBox->setSuffix( i18n( " msecs" ) );

	QLabel* setTimeCompMsecsLabel = new QLabel( quickActionsGroupBox );
	setTimeCompMsecsLabel->setText( i18n( "Compensation for captured video times:" ) );

	m_videoPosCompMsecsSpinBox = new KIntNumInput( quickActionsGroupBox );
	m_videoPosCompMsecsSpinBox->setMinimum( 1 );
	m_videoPosCompMsecsSpinBox->setMaximum( 1000 );
	m_videoPosCompMsecsSpinBox->setSuffix( i18n( " msecs" ) );


	QGridLayout* generalLayout = createGridLayout( generalGroupBox );
	generalLayout->addWidget( defaultEncodingLabel, 1, 0, Qt::AlignRight|Qt::AlignVCenter );
	generalLayout->addWidget( m_defaultEncodingComboBox, 1, 1 );
	generalLayout->addWidget( relativeSeekPositionLabel, 2, 0, Qt::AlignRight|Qt::AlignVCenter );
	generalLayout->addWidget( m_relativeSeekPositionSpinBox, 2, 1 );
	generalLayout->addWidget( m_autoLoadVideoCheckBox, 3, 0, 1, 2 );

	QGridLayout* quickActionsLayout = createGridLayout( quickActionsGroupBox );
	quickActionsLayout->addWidget( shiftMillisecondsLabel, 0, 0, Qt::AlignRight|Qt::AlignVCenter );
	quickActionsLayout->addWidget( m_shiftMsecsSpinBox, 0, 1 );
	quickActionsLayout->addWidget( setTimeCompMsecsLabel, 1, 0, Qt::AlignRight|Qt::AlignVCenter );
	quickActionsLayout->addWidget( m_videoPosCompMsecsSpinBox, 1, 1 );

	connect( m_defaultEncodingComboBox, SIGNAL(activated(int)), this, SIGNAL(settingsChanged()) );
	connect( m_relativeSeekPositionSpinBox, SIGNAL(valueChanged(int)), this, SIGNAL(settingsChanged()) );
	connect( m_autoLoadVideoCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(settingsChanged()) );
	connect( m_shiftMsecsSpinBox, SIGNAL(valueChanged(int)), this, SIGNAL(settingsChanged()) );
	connect( m_videoPosCompMsecsSpinBox, SIGNAL(valueChanged(int)), this, SIGNAL(settingsChanged()) );

	setControlsFromConfig();
}

GeneralConfigWidget::~GeneralConfigWidget()
{
}

void GeneralConfigWidget::setConfigFromControls()
{
	config()->setDefaultSubtitlesEncoding( m_defaultEncodingComboBox->currentText() );
	config()->setSeekOffsetOnDoubleClick( m_relativeSeekPositionSpinBox->value() );
	config()->setAutomaticVideoLoad( m_autoLoadVideoCheckBox->isChecked() );
	config()->setLinesQuickShiftAmount( m_shiftMsecsSpinBox->value() );
	config()->setGrabbedPositionCompensation( m_videoPosCompMsecsSpinBox->value() );
}

void GeneralConfigWidget::setControlsFromConfig()
{
	m_defaultEncodingComboBox->setCurrentItem( config()->defaultSubtitlesEncoding() );
	m_relativeSeekPositionSpinBox->setValue( config()->seekOffsetOnDoubleClick() );
	m_autoLoadVideoCheckBox->setChecked( config()->automaticVideoLoad() );
	m_shiftMsecsSpinBox->setValue( config()->linesQuickShiftAmount() );
	m_videoPosCompMsecsSpinBox->setValue( config()->grabbedPositionCompensation() );
}

#include "generalconfigwidget.moc"
