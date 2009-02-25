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

#include "playerconfigwidget.h"
#include "../../player/player.h"
#include "../../widgets/layeredwidget.h"
#include "../../widgets/textoverlaywidget.h"

#include <QtGui/QGridLayout>
#include <QtGui/QPalette>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#include <QtGui/QFontComboBox>

#include <KPushButton>
#include <KComboBox>
#include <KColorCombo>
#include <KNumInput>

#include <KLocale>

using namespace SubtitleComposer;

PlayerConfigWidget::PlayerConfigWidget( QWidget* parent ):
	AppConfigGroupWidget( new PlayerConfig(), parent )
{
	QGroupBox* generalGroupBox = createGroupBox( i18nc( "@title:group General settings", "General" ) );

	QLabel* backendLabel = new QLabel( generalGroupBox );
	backendLabel->setText( i18n( "Player backend:" ) );

	m_backendComboBox = new KComboBox( generalGroupBox );
	m_backendComboBox->addItems( Player::instance()->backendNames() );

	QLabel* seekJumpLabel = new QLabel( generalGroupBox );
	seekJumpLabel->setText( i18n( "Jump length on seek:" ) );

	m_seekJumpSecsSpinBox = new KIntNumInput( generalGroupBox );
	m_seekJumpSecsSpinBox->setMinimum( 1 );
	m_seekJumpSecsSpinBox->setMaximum( 600 );
	m_seekJumpSecsSpinBox->setSuffix( i18n( " secs" ) );

	m_showPositionTimeEditCheckBox = new QCheckBox( generalGroupBox );
	m_showPositionTimeEditCheckBox->setText( i18n( "Show editable position control" ) );


	QGroupBox* fontGroupBox = createGroupBox( i18nc( "@title:group", "Subtitles" ) );

	QLabel* fontLabel = new QLabel( fontGroupBox );
	fontLabel->setText( i18nc( "@label:chooser The font family name", "Font family:" ) );

	m_fontComboBox = new QFontComboBox( fontGroupBox );

	QLabel* sizeLabel = new QLabel( fontGroupBox );
	sizeLabel->setText( i18n( "Font size:" ) );

	m_sizeSpinBox = new KIntNumInput( fontGroupBox );
	m_sizeSpinBox->setMinimum( 4 );
	m_sizeSpinBox->setMaximum( 100 );
	m_sizeSpinBox->setSuffix( i18n( " points" ) );

	QLabel* primaryColorLabel = new QLabel( fontGroupBox );
	primaryColorLabel->setText( i18n( "Primary color:" ) );

	m_primaryColorComboBox = new KColorCombo( fontGroupBox );

	QLabel* outlineColorLabel = new QLabel( fontGroupBox );
	outlineColorLabel->setText( i18n( "Outline color:" ) );

	m_outlineColorComboBox = new KColorCombo( fontGroupBox );


	QLabel* outlineWidthLabel = new QLabel( fontGroupBox );
	outlineWidthLabel->setText( i18n( "Outline width:" ) );

	m_outlineWidthSpinBox = new KIntNumInput( fontGroupBox );
	m_outlineWidthSpinBox->setMinimum( 0 );
	m_outlineWidthSpinBox->setMaximum( 5 );
	m_outlineWidthSpinBox->setSuffix( i18n( " pixels" ) );


	QGroupBox* previewGroupBox = createGroupBox( i18nc( "@title:group", "Subtitles Preview" ) );

	QWidget* previewWidget = new LayeredWidget( previewGroupBox );
// 	previewWidget->setBackgroundColor( Qt::black );
	previewWidget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
	previewWidget->setMinimumHeight( 60 );

	m_textOverlayWidget = new TextOverlayWidget( previewWidget );
	m_textOverlayWidget->setText( i18nc(
		"Text for previewing the subtitles font settings",
		"<p>The Quick Brown Fox<br/>Jumps Over The Lazy Dog</p>"
	) );
	m_textOverlayWidget->setRichTextMode( true );
	m_textOverlayWidget->setOutlineWidth( 1 );
	m_textOverlayWidget->setAlignment( Qt::AlignHCenter|Qt::AlignVCenter );


	QGridLayout* generalLayout = createGridLayout( generalGroupBox );
	generalLayout->addWidget( backendLabel, 0, 0, Qt::AlignRight|Qt::AlignVCenter );
	generalLayout->addWidget( m_backendComboBox, 0, 1 );
	generalLayout->addWidget( seekJumpLabel, 1, 0, Qt::AlignRight|Qt::AlignVCenter );
	generalLayout->addWidget( m_seekJumpSecsSpinBox, 1, 1 );
	generalLayout->addWidget( m_showPositionTimeEditCheckBox, 2, 0, 1, 2 );

	QGridLayout* fontLayout = createGridLayout( fontGroupBox );
	fontLayout->addWidget( fontLabel, 0, 0, Qt::AlignRight|Qt::AlignVCenter );
	fontLayout->addWidget( m_fontComboBox, 0, 1 );
	fontLayout->addWidget( sizeLabel, 1, 0, Qt::AlignRight|Qt::AlignVCenter );
	fontLayout->addWidget( m_sizeSpinBox, 1, 1 );
	fontLayout->addWidget( primaryColorLabel, 2, 0, Qt::AlignRight|Qt::AlignVCenter );
	fontLayout->addWidget( m_primaryColorComboBox, 2, 1 );
	fontLayout->addWidget( outlineColorLabel, 3, 0, Qt::AlignRight|Qt::AlignVCenter );
	fontLayout->addWidget( m_outlineColorComboBox, 3, 1 );
	fontLayout->addWidget( outlineWidthLabel, 4, 0, Qt::AlignRight|Qt::AlignVCenter );
	fontLayout->addWidget( m_outlineWidthSpinBox, 4, 1 );

	QGridLayout* previewLayout = createGridLayout( previewGroupBox );
	previewLayout->addWidget( previewWidget, 0, 0 );


	connect( m_backendComboBox, SIGNAL(activated(int)), this, SIGNAL(settingsChanged()) );
	connect( m_seekJumpSecsSpinBox, SIGNAL(valueChanged(int)), this, SIGNAL(settingsChanged()) );
	connect( m_showPositionTimeEditCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(settingsChanged()) );

	connect( m_fontComboBox, SIGNAL(activated(const QString&)), this, SLOT(onFamilyChanged(const QString&)) );
	connect( m_fontComboBox, SIGNAL(textChanged(const QString&)), this, SLOT(onFamilyChanged(const QString&)) );
	connect( m_sizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onSizeChanged(int)) );
	connect( m_primaryColorComboBox, SIGNAL(activated(const QColor&)), this, SLOT(onPrimaryColorChanged(const QColor&)) );
	connect( m_outlineColorComboBox, SIGNAL(activated(const QColor&)), this, SLOT(onOutlineColorChanged(const QColor&)) );
	connect( m_outlineWidthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onOutlineWidthChanged(int)) );

	connect( m_fontComboBox, SIGNAL(activated(int)), this, SIGNAL(settingsChanged()) );
	connect( m_sizeSpinBox, SIGNAL(valueChanged(int)), this, SIGNAL(settingsChanged()) );
	connect( m_primaryColorComboBox, SIGNAL(activated(const QColor&)), this, SIGNAL(settingsChanged()) );
	connect( m_outlineColorComboBox, SIGNAL(activated(const QColor&)), this, SIGNAL(settingsChanged()) );
	connect( m_outlineWidthSpinBox, SIGNAL(valueChanged(int)), this, SIGNAL(settingsChanged()) );


	setControlsFromConfig();
}

PlayerConfigWidget::~PlayerConfigWidget()
{
}

void PlayerConfigWidget::setControlsFromConfig()
{
	m_backendComboBox->setCurrentItem( config()->backend() );
	m_seekJumpSecsSpinBox->setValue( config()->seekJumpLength() );
	m_showPositionTimeEditCheckBox->setChecked( config()->showPositionTimeEdit() );

	m_fontComboBox->setCurrentFont( config()->fontFamily() );
	onFamilyChanged( config()->fontFamily() );

	m_sizeSpinBox->setValue( config()->fontPointSize() );
	onSizeChanged( config()->fontPointSize() );

	m_primaryColorComboBox->setColor( config()->fontColor() );
	onPrimaryColorChanged( config()->fontColor() );

	m_outlineColorComboBox->setColor( config()->outlineColor() );
	onOutlineColorChanged( config()->outlineColor() );

	m_outlineWidthSpinBox->setValue( config()->outlineWidth() );
	onOutlineWidthChanged( config()->outlineWidth() );
}

void PlayerConfigWidget::setConfigFromControls()
{
	config()->setBackend( m_backendComboBox->currentText() );
	config()->setSeekJumpLength( m_seekJumpSecsSpinBox->value() );
	config()->setShowPositionTimeEdit( m_showPositionTimeEditCheckBox->isChecked() );

	config()->setFontFamily( m_textOverlayWidget->family() );
	config()->setFontPointSize( m_textOverlayWidget->pointSize() );
	config()->setFontColor( m_textOverlayWidget->primaryColor() );
	config()->setOutlineColor( m_textOverlayWidget->outlineColor() );
	config()->setOutlineWidth( (int)m_textOverlayWidget->outlineWidth() );
}

void PlayerConfigWidget::onFamilyChanged( const QString& family )
{
	if ( m_fontComboBox->findText( family ) != -1 )
		m_textOverlayWidget->setFamily( family );
}

void PlayerConfigWidget::onSizeChanged( int size )
{
	m_textOverlayWidget->setPointSize( size );
}

void PlayerConfigWidget::onPrimaryColorChanged( const QColor& color )
{
	m_textOverlayWidget->setPrimaryColor( color );
}

void PlayerConfigWidget::onOutlineColorChanged( const QColor& color )
{
	m_textOverlayWidget->setOutlineColor( color );
}

void PlayerConfigWidget::onOutlineWidthChanged( int width )
{
	m_textOverlayWidget->setOutlineWidth( width );
}

#include "playerconfigwidget.moc"
