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

#include "gstreamerconfigwidget.h"

#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QCheckBox>

#include <KLocale>
#include <KComboBox>

using namespace SubtitleComposer;

GStreamerConfigWidget::GStreamerConfigWidget(QWidget *parent) :
	AppConfigGroupWidget(new GStreamerConfig(), parent)
{
	QGroupBox *sinksGroupBox = createGroupBox(i18nc("@title:group GStreamer settings", "Options"));

	m_audioSinkCheckBox = new QCheckBox(sinksGroupBox);
	m_audioSinkCheckBox->setText(i18n("Audio sink:"));

	m_audioSinkComboBox = new KComboBox(false, sinksGroupBox);
	m_audioSinkComboBox->setEnabled(false);
	m_audioSinkComboBox->setEditable(true);
	m_audioSinkComboBox->addItems(QString("autoaudiosink alsasink osssink esdsink gconfaudiosink pulsesink fakesink").split(' '));

	m_videoSinkCheckBox = new QCheckBox(sinksGroupBox);
	m_videoSinkCheckBox->setText(i18n("Video sink:"));

	m_videoSinkComboBox = new KComboBox(false, sinksGroupBox);
	m_videoSinkComboBox->setEnabled(false);
	m_videoSinkComboBox->setEditable(true);
	m_videoSinkComboBox->addItems(QString("autovideosink ximagesink xvimagesink gconfvideosink fakesink").split(' '));

	QGridLayout *sinksLayout = createGridLayout(sinksGroupBox);
	sinksLayout->addWidget(m_audioSinkCheckBox, 0, 0, Qt::AlignRight | Qt::AlignVCenter);
	sinksLayout->addWidget(m_audioSinkComboBox, 0, 1);
	sinksLayout->addWidget(m_videoSinkCheckBox, 1, 0, Qt::AlignRight | Qt::AlignVCenter);
	sinksLayout->addWidget(m_videoSinkComboBox, 1, 1);

	connect(m_audioSinkCheckBox, SIGNAL(toggled(bool)), m_audioSinkComboBox, SLOT(setEnabled(bool)));
	connect(m_videoSinkCheckBox, SIGNAL(toggled(bool)), m_videoSinkComboBox, SLOT(setEnabled(bool)));
	connect(m_audioSinkCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(settingsChanged()));
	connect(m_videoSinkCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(settingsChanged()));
	connect(m_audioSinkComboBox, SIGNAL(textChanged(const QString &)), this, SIGNAL(settingsChanged()));
	connect(m_videoSinkComboBox, SIGNAL(textChanged(const QString &)), this, SIGNAL(settingsChanged()));

	setControlsFromConfig();
}

GStreamerConfigWidget::~GStreamerConfigWidget()
{}

void
GStreamerConfigWidget::setConfigFromControls()
{
	config()->setAudioSink(m_audioSinkCheckBox->isChecked() ? m_audioSinkComboBox->currentText() : QString());
	config()->setVideoSink(m_videoSinkCheckBox->isChecked() ? m_videoSinkComboBox->currentText() : QString());
}

void
GStreamerConfigWidget::setControlsFromConfig()
{
	m_audioSinkCheckBox->setChecked(config()->hasAudioSink());
	if(m_audioSinkCheckBox->isChecked())
		m_audioSinkComboBox->setEditText(config()->audioSink());

	m_videoSinkCheckBox->setChecked(config()->hasVideoSink());
	if(m_videoSinkCheckBox->isChecked())
		m_videoSinkComboBox->setEditText(config()->videoSink());
}

#include "gstreamerconfigwidget.moc"
