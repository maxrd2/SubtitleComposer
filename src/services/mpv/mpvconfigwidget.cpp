/***************************************************************************
 *   Copyright (C) 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)  *
 *   Copyright (C) 2010-2015 Mladen Milinkovic <max@smoothware.net>        *
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

#include "mpvconfigwidget.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QLabel>

#include <KLocale>
#include <KComboBox>
#include <KNumInput>
#include <kurlrequester.h>

using namespace SubtitleComposer;

MPVConfigWidget::MPVConfigWidget(QWidget *parent)
	: AppConfigGroupWidget(new MPVConfig(), parent, false)
{
	setupUi(this);

	m_videoOutputValue->addItems(QString("vdpau vaapi xv wayland opengl opengl-hq opengl-old x11 null").split(' '));
	m_hwDecodeValue->addItems(QString("auto vdpau vaapi vaapi-copy").split(' '));
	m_audioOutputValue->addItems(QString("pulse alsa oss portaudio jack null").split(' '));

	connect(m_avSyncCheckbox, SIGNAL(toggled(bool)), this, SIGNAL(settingsChanged()));
	connect(m_avSyncValue, SIGNAL(valueChanged(int)), this, SIGNAL(settingsChanged()));

	connect(m_videoOutputEnabled, SIGNAL(toggled(bool)), this, SIGNAL(settingsChanged()));
	connect(m_videoOutputValue, SIGNAL(textChanged(const QString &)), this, SIGNAL(settingsChanged()));
	connect(m_hwDecodeEnabled, SIGNAL(toggled(bool)), this, SIGNAL(settingsChanged()));
	connect(m_hwDecodeValue, SIGNAL(textChanged(const QString &)), this, SIGNAL(settingsChanged()));
	connect(m_frameDropEnabled, SIGNAL(toggled(bool)), this, SIGNAL(settingsChanged()));

	connect(m_audioOutputEnabled, SIGNAL(toggled(bool)), this, SIGNAL(settingsChanged()));
	connect(m_audioOutputValue, SIGNAL(textChanged(const QString &)), this, SIGNAL(settingsChanged()));
	connect(m_audioChannelsEnabled, SIGNAL(toggled(bool)), this, SIGNAL(settingsChanged()));
	connect(m_audioChannelsValue, SIGNAL(valueChanged(int)), this, SIGNAL(settingsChanged()));
	connect(m_volumeAmplificationEnabled, SIGNAL(toggled(bool)), this, SIGNAL(settingsChanged()));
	connect(m_volumeAmplificationValue, SIGNAL(valueChanged(int)), this, SIGNAL(settingsChanged()));
	connect(m_volumeNormalizationEnabled, SIGNAL(toggled(bool)), this, SIGNAL(settingsChanged()));

	setControlsFromConfig();
}

MPVConfigWidget::~MPVConfigWidget()
{}

void
MPVConfigWidget::setConfigFromControls()
{
	config()->setAutoSyncFactor(m_avSyncCheckbox->isChecked() ? m_avSyncValue->value() : -1);

	config()->setVideoOutput(m_videoOutputEnabled->isChecked() ? m_videoOutputValue->currentText() : QString());
	config()->setHwDecode(m_hwDecodeEnabled->isChecked() ? m_hwDecodeValue->currentText() : QString());
	config()->setFrameDropping(m_frameDropEnabled->isChecked());

	config()->setAudioOutput(m_audioOutputEnabled->isChecked() ? m_audioOutputValue->currentText() : QString());
	config()->setAudioChannels(m_audioChannelsEnabled->isChecked() ? m_audioChannelsValue->value() : 0);
	config()->setVolumeAplification(m_volumeAmplificationEnabled->isChecked() ? m_volumeAmplificationValue->value() : 0);
	config()->setVolumeNormalization(m_volumeNormalizationEnabled->isChecked());
}

void
MPVConfigWidget::setControlsFromConfig()
{
	m_avSyncCheckbox->setChecked(config()->hasAutoSyncFactor());
	m_avSyncValue->setEnabled(config()->hasAutoSyncFactor());
	if(m_avSyncCheckbox->isChecked())
		m_avSyncValue->setValue(config()->autoSyncFactor());

	m_videoOutputEnabled->setChecked(config()->hasVideoOutput());
	m_videoOutputValue->setEnabled(m_videoOutputEnabled->isChecked());
	if(m_videoOutputEnabled->isChecked())
		m_videoOutputValue->setEditText(config()->videoOutput());

	m_hwDecodeEnabled->setChecked(config()->hasHwDecode());
	m_hwDecodeValue->setEnabled(m_hwDecodeEnabled->isChecked());
	if(m_hwDecodeEnabled->isChecked())
		m_hwDecodeValue->setEditText(config()->hwDecode());
	m_frameDropEnabled->setChecked(config()->frameDropping());

	m_audioOutputEnabled->setChecked(config()->hasAudioOutput());
	m_audioOutputValue->setEnabled(m_audioOutputEnabled->isChecked());
	if(m_audioOutputEnabled->isChecked())
		m_audioOutputValue->setEditText(config()->audioOutput());

	m_audioChannelsEnabled->setChecked(config()->hasAudioChannels());
	m_audioChannelsValue->setEnabled(m_audioChannelsEnabled->isChecked());
	if(m_audioChannelsEnabled->isChecked())
		m_audioChannelsValue->setValue(config()->audioChannels());
	m_audioChannelsLabel->setNum(m_audioChannelsValue->value());

	m_volumeAmplificationEnabled->setChecked(config()->hasVolumeAmplification());
	m_volumeAmplificationValue->setEnabled(m_volumeAmplificationEnabled->isChecked());
	if(m_volumeAmplificationEnabled->isChecked())
		m_volumeAmplificationValue->setValue(config()->volumeAmplification());
	m_volumeAmplificationLabel->setNum(m_volumeAmplificationValue->value());

	m_volumeNormalizationEnabled->setChecked(config()->volumeNormalization());
}
