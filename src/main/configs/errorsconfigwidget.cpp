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

#include "errorsconfigwidget.h"
#include "../../widgets/timeedit.h"

#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>

#include <KNumInput>

#include <KLocale>

using namespace SubtitleComposer;

ErrorsConfigWidget::ErrorsConfigWidget( QWidget* parent ):
	AppConfigGroupWidget( new ErrorsConfig(), parent )
{
	QGroupBox* textsGroupBox = createGroupBox( i18nc( "@title:group", "Texts" ) );

	QLabel* maxCharactersLabel = new QLabel( textsGroupBox );
	maxCharactersLabel->setText( i18n( "Maximum number of characters:" ) );

	m_maxCharactersSpinBox = new KIntNumInput( textsGroupBox );
	m_maxCharactersSpinBox->setMinimum( 0 );
	m_maxCharactersSpinBox->setMaximum( 200 );
	m_maxCharactersSpinBox->setSuffix( i18n( " characters" ) );

	QLabel* maxLinesLabel = new QLabel( textsGroupBox );
	maxLinesLabel->setText( i18n( "Maximum number of lines:" ) );

	m_maxLinesSpinBox = new KIntNumInput( textsGroupBox );
	m_maxLinesSpinBox->setMinimum( 1 );
	m_maxLinesSpinBox->setMaximum( 20 );
	m_maxLinesSpinBox->setSuffix( i18n( " lines" ) );


	QGroupBox* absoluteDurationsGroupBox = createGroupBox( i18nc( "@title:group", "Absolute Durations" ) );

	QLabel* minDurationLabel = new QLabel( absoluteDurationsGroupBox );
	minDurationLabel->setText( i18n( "Minimum duration:" ) );

	m_minDurationSpinBox = new KIntNumInput( absoluteDurationsGroupBox );
	m_minDurationSpinBox->setMinimum( 0 );
	m_minDurationSpinBox->setMaximum( 2000 );
	m_minDurationSpinBox->setSuffix( i18n( " msecs" ) );

	QLabel* maxDurationLabel = new QLabel( absoluteDurationsGroupBox );
	maxDurationLabel->setText( i18n( "Maximum duration:" ) );

	m_maxDurationSpinBox = new KIntNumInput( absoluteDurationsGroupBox );
	m_maxDurationSpinBox->setMinimum( 2000 );
	m_maxDurationSpinBox->setMaximum( 8000 );
	m_maxDurationSpinBox->setSuffix( i18n( " msecs" ) );


	QGroupBox* relativeDurationsGroupBox = createGroupBox( i18nc( "@title:group", "Relative Durations" ) );

	QLabel* minDurationPerCharLabel = new QLabel( relativeDurationsGroupBox );
	minDurationPerCharLabel->setText( i18n( "Minimum duration:" ) );

	m_minDurationPerCharSpinBox = new KIntNumInput( relativeDurationsGroupBox );
	m_minDurationPerCharSpinBox->setMinimum( 0 );
	m_minDurationPerCharSpinBox->setMaximum( 100 );
	m_minDurationPerCharSpinBox->setSuffix( i18n( " msecs/character" ) );

	QLabel* maxDurationPerCharLabel = new QLabel( relativeDurationsGroupBox );
	maxDurationPerCharLabel->setText( i18n( "Maximum duration:" ) );

	m_maxDurationPerCharSpinBox = new KIntNumInput( relativeDurationsGroupBox );
	m_maxDurationPerCharSpinBox->setMinimum( 0 );
	m_maxDurationPerCharSpinBox->setMaximum( 500 );
	m_maxDurationPerCharSpinBox->setSuffix( i18n( " msecs/character" ) );

	QGroupBox* behaviorGroupBox = createGroupBox( i18nc( "@title:group", "Behavior" ) );

	m_autoClearFixedCheckBox = new QCheckBox( behaviorGroupBox );
	m_autoClearFixedCheckBox->setText( i18n( "Automatically clear fixed errors" ) );


	QGridLayout* textsLayout = createGridLayout( textsGroupBox );
	textsLayout->addWidget( maxCharactersLabel, 0, 0, Qt::AlignRight|Qt::AlignVCenter );
	textsLayout->addWidget( m_maxCharactersSpinBox, 0, 1 );
	textsLayout->addWidget( maxLinesLabel, 1, 0, Qt::AlignRight|Qt::AlignVCenter );
	textsLayout->addWidget( m_maxLinesSpinBox, 1, 1 );

	QGridLayout* absoluteDurationsLayout = createGridLayout( absoluteDurationsGroupBox );
	absoluteDurationsLayout->addWidget( minDurationLabel, 0, 0, Qt::AlignRight|Qt::AlignVCenter );
	absoluteDurationsLayout->addWidget( m_minDurationSpinBox, 0, 1 );
	absoluteDurationsLayout->addWidget( maxDurationLabel, 1, 0, Qt::AlignRight|Qt::AlignVCenter );
	absoluteDurationsLayout->addWidget( m_maxDurationSpinBox, 1, 1 );

	QGridLayout* relativeDurationsLayout = createGridLayout( relativeDurationsGroupBox );
	relativeDurationsLayout->addWidget( minDurationPerCharLabel, 0, 0, Qt::AlignRight|Qt::AlignVCenter );
	relativeDurationsLayout->addWidget( m_minDurationPerCharSpinBox, 0, 1 );
	relativeDurationsLayout->addWidget( maxDurationPerCharLabel, 1, 0, Qt::AlignRight|Qt::AlignVCenter );
	relativeDurationsLayout->addWidget( m_maxDurationPerCharSpinBox, 1, 1 );

	QGridLayout* behaviorLayout = createGridLayout( behaviorGroupBox );
	behaviorLayout->addWidget( m_autoClearFixedCheckBox, 0, 0 );

	connect( m_maxCharactersSpinBox, SIGNAL(valueChanged(int)), this, SIGNAL(settingsChanged()) );
	connect( m_maxLinesSpinBox, SIGNAL(valueChanged(int)), this, SIGNAL(settingsChanged()) );
	connect( m_minDurationSpinBox, SIGNAL(valueChanged(int)), this, SIGNAL(settingsChanged()) );
	connect( m_maxDurationSpinBox, SIGNAL(valueChanged(int)), this, SIGNAL(settingsChanged()) );
	connect( m_minDurationPerCharSpinBox, SIGNAL(valueChanged(int)), this, SIGNAL(settingsChanged()) );
	connect( m_maxDurationPerCharSpinBox, SIGNAL(valueChanged(int)), this, SIGNAL(settingsChanged()) );
	connect( m_autoClearFixedCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(settingsChanged()) );

	setControlsFromConfig();
}

ErrorsConfigWidget::~ErrorsConfigWidget()
{
}

void ErrorsConfigWidget::setConfigFromControls()
{
	config()->setMaxCharacters( m_maxCharactersSpinBox->value() );
	config()->setMaxLines( m_maxLinesSpinBox->value() );
	config()->setMinDuration( m_minDurationSpinBox->value() );
	config()->setMaxDuration( m_maxDurationSpinBox->value() );
	config()->setMinDurationPerChar( m_minDurationPerCharSpinBox->value() );
	config()->setMaxDurationPerChar( m_maxDurationPerCharSpinBox->value() );
	config()->setAutoClearFixed( m_autoClearFixedCheckBox->isChecked() );
}

void ErrorsConfigWidget::setControlsFromConfig()
{
	m_maxCharactersSpinBox->setValue( config()->maxCharacters() );
	m_maxLinesSpinBox->setValue( config()->maxLines() );
	m_minDurationSpinBox->setValue( config()->minDuration() );
	m_maxDurationSpinBox->setValue( config()->maxDuration() );
	m_minDurationPerCharSpinBox->setValue( config()->minDurationPerChar() );
	m_maxDurationPerCharSpinBox->setValue( config()->maxDurationPerChar() );
	m_autoClearFixedCheckBox->setChecked( config()->autoClearFixed() );
}

#include "errorsconfigwidget.moc"
