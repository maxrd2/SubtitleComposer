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

#include "mplayerconfigwidget.h"

#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QCheckBox>
#include <QtGui/QLabel>

#include <KLocale>
#include <KComboBox>
#include <KNumInput>
#include <kurlrequester.h>

using namespace SubtitleComposer;

MPlayerConfigWidget::MPlayerConfigWidget( QWidget* parent ):
	AppConfigGroupWidget( new MPlayerConfig(), parent )
{
	QGroupBox* generalGroupBox = createGroupBox( i18nc( "@title:group General settings", "General" ) );

	QLabel* pathLabel = new QLabel( generalGroupBox );
	pathLabel->setText( i18n( "Executable path:" ) );

	m_pathUrlRequester = new KUrlRequester( generalGroupBox );
 	m_pathUrlRequester->setWindowTitle( i18n( "Select the MPlayer executable" ) );
	m_pathUrlRequester->setMode( KFile::File|KFile::ExistingOnly|KFile::LocalOnly );
	m_pathUrlRequester->setUrl( KUrl( "mplayer" ) );

	QGroupBox* videoGroupBox = createGroupBox( i18nc( "@title:group Video settings", "Video" ) );

	m_videoOutputCheckBox = new QCheckBox( videoGroupBox );
	m_videoOutputCheckBox->setText( i18n( "Video output driver:" ) );

	m_videoOutputComboBox = new KComboBox( false, videoGroupBox );
	m_videoOutputComboBox->setEnabled( false );
	m_videoOutputComboBox->setEditable( true );
	m_videoOutputComboBox->addItems( QString( "xv xvmc x11 gl gl2 sdl tdfxfb s3fb xover dga ggi fbdev fbdev2 v4l2 bl directfb dfbmga null" ).split( ' ' ) );

	m_frameDropCheckBox = new QCheckBox( videoGroupBox );
	m_frameDropCheckBox->setText( i18n( "Allow frame dropping" ) );

	m_hardFrameDropCheckBox = new QCheckBox( videoGroupBox );
	m_hardFrameDropCheckBox->setText( i18n( "Allow hard frame dropping" ) );


	QGroupBox* audioGroupBox = createGroupBox( i18nc( "@title:group Audio settings", "Audio" ) );

	m_audioOutputCheckBox = new QCheckBox( audioGroupBox );
	m_audioOutputCheckBox->setText( i18n( "Audio output driver:" ) );

	m_audioOutputComboBox = new KComboBox( false, audioGroupBox );
	m_audioOutputComboBox->setEnabled( false );
	m_audioOutputComboBox->setEditable( true );
	m_audioOutputComboBox->addItems( QString( "alsa oss esd jack nas sdl openal v4l2 null" ).split( ' ' ) );

	m_avsyncCheckBox = new QCheckBox( audioGroupBox );
	m_avsyncCheckBox->setText( i18n( "A/V synchronization factor:" ) );

	m_avsyncSpinBox = new KIntSpinBox( audioGroupBox );
	m_avsyncSpinBox->setEnabled( false );
	m_avsyncSpinBox->setMinimum( 0 );
	m_avsyncSpinBox->setMaximum( 50 );

	m_volumeNormalizationCheckBox = new QCheckBox( audioGroupBox );
	m_volumeNormalizationCheckBox->setText( i18n( "Volume normalization" ) );


	QGridLayout* generalLayout = createGridLayout( generalGroupBox );
	generalLayout->addWidget( pathLabel, 0, 0, Qt::AlignRight|Qt::AlignVCenter );
	generalLayout->addWidget( m_pathUrlRequester, 0, 1 );

	QGridLayout* videoLayout = createGridLayout( videoGroupBox );
	videoLayout->addWidget( m_videoOutputCheckBox, 0, 0, Qt::AlignRight|Qt::AlignVCenter );
	videoLayout->addWidget( m_videoOutputComboBox, 0, 1 );
	videoLayout->addWidget( m_frameDropCheckBox, 1, 0, 1, 2 );
	videoLayout->addWidget( m_hardFrameDropCheckBox, 2, 0, 1, 2 );

	QGridLayout* audioLayout = createGridLayout( audioGroupBox );
	audioLayout->addWidget( m_audioOutputCheckBox, 0, 0, Qt::AlignRight|Qt::AlignVCenter );
	audioLayout->addWidget( m_audioOutputComboBox, 0, 1 );
	audioLayout->addWidget( m_avsyncCheckBox, 1, 0, Qt::AlignRight|Qt::AlignVCenter );
	audioLayout->addWidget( m_avsyncSpinBox, 1, 1 );
	audioLayout->addWidget( m_volumeNormalizationCheckBox, 2, 0, 1, 2 );


	connect( m_audioOutputCheckBox, SIGNAL( toggled(bool) ), m_audioOutputComboBox, SLOT( setEnabled(bool) ) );
	connect( m_avsyncCheckBox, SIGNAL( toggled(bool) ), m_avsyncSpinBox, SLOT( setEnabled(bool) ) );
	connect( m_videoOutputCheckBox, SIGNAL( toggled(bool) ), m_videoOutputComboBox, SLOT( setEnabled(bool) ) );

	connect( m_pathUrlRequester, SIGNAL( textChanged(const QString&) ), this, SIGNAL( settingsChanged() ) );
	connect( m_audioOutputCheckBox, SIGNAL( toggled(bool) ), this, SIGNAL( settingsChanged() ) );
	connect( m_avsyncCheckBox, SIGNAL( toggled(bool) ), this, SIGNAL( settingsChanged() ) );
	connect( m_volumeNormalizationCheckBox, SIGNAL( toggled(bool) ), this, SIGNAL( settingsChanged() ) );
	connect( m_videoOutputCheckBox, SIGNAL( toggled(bool) ), this, SIGNAL( settingsChanged() ) );
	connect( m_frameDropCheckBox, SIGNAL( toggled(bool) ), this, SIGNAL( settingsChanged() ) );
	connect( m_hardFrameDropCheckBox, SIGNAL( toggled(bool) ), this, SIGNAL( settingsChanged() ) );
	connect( m_audioOutputComboBox, SIGNAL( textChanged(const QString&) ), this, SIGNAL( settingsChanged() ) );
	connect( m_avsyncSpinBox, SIGNAL( valueChanged(int) ), this, SIGNAL( settingsChanged() ) );
	connect( m_videoOutputComboBox, SIGNAL( textChanged(const QString&) ), this, SIGNAL( settingsChanged() ) );

	setControlsFromConfig();
}

MPlayerConfigWidget::~MPlayerConfigWidget()
{
}

void MPlayerConfigWidget::setConfigFromControls()
{
	config()->setExecutablePath( m_pathUrlRequester->url().path() );

	config()->setAudioOutput( m_audioOutputCheckBox->isChecked() ? m_audioOutputComboBox->currentText() : QString() );
	config()->setVolumeNormalizationEnabled( m_volumeNormalizationCheckBox->isChecked() );
	config()->setAutoSyncFactor( m_avsyncCheckBox->isChecked() ? m_avsyncSpinBox->value() : -1 );

	config()->setVideoOutput( m_videoOutputCheckBox->isChecked() ? m_videoOutputComboBox->currentText() : QString() );
	config()->setFrameDropping( m_frameDropCheckBox->isChecked() );
	config()->setHardFrameDropping( m_hardFrameDropCheckBox->isChecked() );
}

void MPlayerConfigWidget::setControlsFromConfig()
{
	m_pathUrlRequester->setUrl( config()->executablePath() );

	m_audioOutputCheckBox->setChecked( config()->hasAudioOutput() );
	if ( m_audioOutputCheckBox->isChecked() )
		m_audioOutputComboBox->setEditText( config()->audioOutput() );

	m_volumeNormalizationCheckBox->setChecked( config()->volumeNormalizationEnabled() );

	m_avsyncCheckBox->setChecked( config()->hasAutoSyncFactor() );
	if ( m_avsyncCheckBox->isChecked() )
		m_avsyncSpinBox->setValue( config()->autoSyncFactor() );

	m_videoOutputCheckBox->setChecked( config()->hasVideoOutput() );
	if ( m_videoOutputCheckBox->isChecked() )
		m_videoOutputComboBox->setEditText( config()->videoOutput() );

	m_frameDropCheckBox->setChecked( config()->frameDropping() );
	m_hardFrameDropCheckBox->setChecked( config()->hardFrameDropping() );
}

#include "mplayerconfigwidget.moc"
