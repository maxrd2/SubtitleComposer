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

MPVConfigWidget::MPVConfigWidget(QWidget *parent) :
	AppConfigGroupWidget(new MPVConfig(), parent)
{
	QGroupBox *generalGroupBox = createGroupBox(i18nc("@title:group General settings", "General"));

	m_avsyncCheckBox = new QCheckBox(generalGroupBox);
	m_avsyncCheckBox->setText(i18n("A/V synchronization factor:"));

	m_avsyncSpinBox = new KIntSpinBox(generalGroupBox);
	m_avsyncSpinBox->setEnabled(false);
	m_avsyncSpinBox->setMinimum(0);
	m_avsyncSpinBox->setMaximum(50);

	QGroupBox *videoGroupBox = createGroupBox(i18nc("@title:group Video settings", "Video"));

	m_videoOutputCheckBox = new QCheckBox(videoGroupBox);
	m_videoOutputCheckBox->setText(i18n("Video output driver:"));

	m_videoOutputComboBox = new KComboBox(false, videoGroupBox);
	m_videoOutputComboBox->setEnabled(false);
	m_videoOutputComboBox->setEditable(true);
	m_videoOutputComboBox->addItems(QString("vdpau vaapi xv wayland opengl opengl-hq opengl-old x11 null").split(' '));

	m_hwDecodeCheckBox = new QCheckBox(videoGroupBox);
	m_hwDecodeCheckBox->setText(i18n("Hardware video decoding:"));

	m_hwDecodeComboBox = new KComboBox(false, videoGroupBox);
	m_hwDecodeComboBox->setEnabled(false);
	m_hwDecodeComboBox->setEditable(true);
	m_hwDecodeComboBox->addItems(QString("auto vdpau vaapi vaapi-copy").split(' '));

	m_frameDropCheckBox = new QCheckBox(videoGroupBox);
	m_frameDropCheckBox->setText(i18n("Allow frame dropping"));

	QGroupBox *audioGroupBox = createGroupBox(i18nc("@title:group Audio settings", "Audio"));

	m_audioOutputCheckBox = new QCheckBox(audioGroupBox);
	m_audioOutputCheckBox->setText(i18n("Audio output driver:"));

	m_audioOutputComboBox = new KComboBox(false, audioGroupBox);
	m_audioOutputComboBox->setEnabled(false);
	m_audioOutputComboBox->setEditable(true);
	m_audioOutputComboBox->addItems(QString("pulse alsa oss portaudio jack null").split(' '));

	m_audioChannelsCheckBox = new QCheckBox(audioGroupBox);
	m_audioChannelsCheckBox->setText(i18n("Audio channels:"));

	m_audioChannelsSpinBox = new KIntSpinBox(audioGroupBox);
	m_audioChannelsSpinBox->setEnabled(false);
	m_audioChannelsSpinBox->setMinimum(1);
	m_audioChannelsSpinBox->setMaximum(6);

	m_volumeAmplificationCheckBox = new QCheckBox(audioGroupBox);
	m_volumeAmplificationCheckBox->setText(i18n("Volume amplification:"));

	m_volumeAmplificationSpinBox = new KIntSpinBox(audioGroupBox);
	m_volumeAmplificationSpinBox->setEnabled(false);
	m_volumeAmplificationSpinBox->setMinimum(1);
	m_volumeAmplificationSpinBox->setMaximum(1000);
	m_volumeAmplificationSpinBox->setSuffix(" %");

	m_volumeNormalizationCheckBox = new QCheckBox(audioGroupBox);
	m_volumeNormalizationCheckBox->setText(i18n("Volume normalization"));

	QGridLayout *generalLayout = createGridLayout(generalGroupBox);
	generalLayout->addWidget(m_avsyncCheckBox, 1, 0, Qt::AlignRight | Qt::AlignVCenter);
	generalLayout->addWidget(m_avsyncSpinBox, 1, 1);

	QGridLayout *videoLayout = createGridLayout(videoGroupBox);
	videoLayout->addWidget(m_videoOutputCheckBox, 0, 0, Qt::AlignRight | Qt::AlignVCenter);
	videoLayout->addWidget(m_videoOutputComboBox, 0, 1);
	videoLayout->addWidget(m_hwDecodeCheckBox, 1, 0, Qt::AlignRight | Qt::AlignVCenter);
	videoLayout->addWidget(m_hwDecodeComboBox, 1, 1);
	videoLayout->addWidget(m_frameDropCheckBox, 2, 0, 1, 2);

	QGridLayout *audioLayout = createGridLayout(audioGroupBox);
	audioLayout->addWidget(m_audioOutputCheckBox, 0, 0, Qt::AlignRight | Qt::AlignVCenter);
	audioLayout->addWidget(m_audioOutputComboBox, 0, 1);
	audioLayout->addWidget(m_audioChannelsCheckBox, 1, 0, Qt::AlignRight | Qt::AlignVCenter);
	audioLayout->addWidget(m_audioChannelsSpinBox, 1, 1);
	audioLayout->addWidget(m_volumeAmplificationCheckBox, 2, 0, Qt::AlignRight | Qt::AlignVCenter);
	audioLayout->addWidget(m_volumeAmplificationSpinBox, 2, 1);
	audioLayout->addWidget(m_volumeNormalizationCheckBox, 3, 0, 1, 2);

	connect(m_avsyncCheckBox, SIGNAL(toggled(bool)), m_avsyncSpinBox, SLOT(setEnabled(bool)));
	connect(m_videoOutputCheckBox, SIGNAL(toggled(bool)), m_videoOutputComboBox, SLOT(setEnabled(bool)));
	connect(m_hwDecodeCheckBox, SIGNAL(toggled(bool)), m_hwDecodeComboBox, SLOT(setEnabled(bool)));
	connect(m_audioOutputCheckBox, SIGNAL(toggled(bool)), m_audioOutputComboBox, SLOT(setEnabled(bool)));
	connect(m_audioChannelsCheckBox, SIGNAL(toggled(bool)), m_audioChannelsSpinBox, SLOT(setEnabled(bool)));
	connect(m_volumeAmplificationCheckBox, SIGNAL(toggled(bool)), m_volumeAmplificationSpinBox, SLOT(setEnabled(bool)));

	connect(m_avsyncCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(settingsChanged()));
	connect(m_avsyncSpinBox, SIGNAL(valueChanged(int)), this, SIGNAL(settingsChanged()));

	connect(m_videoOutputCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(settingsChanged()));
	connect(m_videoOutputComboBox, SIGNAL(textChanged(const QString &)), this, SIGNAL(settingsChanged()));
	connect(m_hwDecodeCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(settingsChanged()));
	connect(m_hwDecodeComboBox, SIGNAL(textChanged(const QString &)), this, SIGNAL(settingsChanged()));
	connect(m_frameDropCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(settingsChanged()));

	connect(m_audioOutputCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(settingsChanged()));
	connect(m_audioOutputComboBox, SIGNAL(textChanged(const QString &)), this, SIGNAL(settingsChanged()));
	connect(m_audioChannelsCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(settingsChanged()));
	connect(m_audioChannelsSpinBox, SIGNAL(valueChanged(int)), this, SIGNAL(settingsChanged()));
	connect(m_volumeAmplificationCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(settingsChanged()));
	connect(m_volumeAmplificationSpinBox, SIGNAL(valueChanged(int)), this, SIGNAL(settingsChanged()));
	connect(m_volumeNormalizationCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(settingsChanged()));

	setControlsFromConfig();
}

MPVConfigWidget::~MPVConfigWidget()
{}

void
MPVConfigWidget::setConfigFromControls()
{
	config()->setAutoSyncFactor(m_avsyncCheckBox->isChecked() ? m_avsyncSpinBox->value() : -1);

	config()->setVideoOutput(m_videoOutputCheckBox->isChecked() ? m_videoOutputComboBox->currentText() : QString());
	config()->setHwDecode(m_hwDecodeCheckBox->isChecked() ? m_hwDecodeComboBox->currentText() : QString());
	config()->setFrameDropping(m_frameDropCheckBox->isChecked());

	config()->setAudioOutput(m_audioOutputCheckBox->isChecked() ? m_audioOutputComboBox->currentText() : QString());
	config()->setAudioChannels(m_audioChannelsCheckBox->isChecked() ? m_audioChannelsSpinBox->value() : 0);
	config()->setVolumeAplification(m_volumeAmplificationCheckBox->isChecked() ? m_volumeAmplificationSpinBox->value() : 0);
	config()->setVolumeNormalization(m_volumeNormalizationCheckBox->isChecked());
}

void
MPVConfigWidget::setControlsFromConfig()
{
	m_avsyncCheckBox->setChecked(config()->hasAutoSyncFactor());
	if(m_avsyncCheckBox->isChecked())
		m_avsyncSpinBox->setValue(config()->autoSyncFactor());

	m_videoOutputCheckBox->setChecked(config()->hasVideoOutput());
	if(m_videoOutputCheckBox->isChecked())
		m_videoOutputComboBox->setEditText(config()->videoOutput());
	m_hwDecodeCheckBox->setChecked(config()->hasHwDecode());
	if(m_hwDecodeCheckBox->isChecked())
		m_hwDecodeComboBox->setEditText(config()->hwDecode());
	m_frameDropCheckBox->setChecked(config()->frameDropping());

	m_audioOutputCheckBox->setChecked(config()->hasAudioOutput());
	if(m_audioOutputCheckBox->isChecked())
		m_audioOutputComboBox->setEditText(config()->audioOutput());
	m_audioChannelsCheckBox->setChecked(config()->hasAudioChannels());
	if(m_audioChannelsCheckBox->isChecked())
		m_audioChannelsSpinBox->setValue(config()->audioChannels());

	m_volumeAmplificationCheckBox->setChecked(config()->hasVolumeAmplification());
	if(m_volumeAmplificationCheckBox->isChecked())
		m_volumeAmplificationSpinBox->setValue(config()->volumeAmplification());

	m_volumeNormalizationCheckBox->setChecked(config()->volumeNormalization());
}


