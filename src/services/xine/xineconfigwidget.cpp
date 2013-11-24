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

#include "xineconfigwidget.h"

#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QCheckBox>

#include <KLocale>
#include <KComboBox>

using namespace SubtitleComposer;

XineConfigWidget::XineConfigWidget(QWidget *parent) :
	AppConfigGroupWidget(new XineConfig(), parent)
{
	QGroupBox *driversGroupBox = createGroupBox(i18nc("@title:group Xine settings", "Options"));

	m_audioDriverCheckBox = new QCheckBox(driversGroupBox);
	m_audioDriverCheckBox->setText(i18n("Audio driver:"));

	m_audioDriverComboBox = new KComboBox(false, driversGroupBox);
	m_audioDriverComboBox->setEnabled(false);
	m_audioDriverComboBox->setEditable(true);
	m_audioDriverComboBox->addItems(QString("auto alsa oss jack pulseaudio esd").split(' '));

	m_videoDriverCheckBox = new QCheckBox(driversGroupBox);
	m_videoDriverCheckBox->setText(i18n("Video driver:"));

	m_videoDriverComboBox = new KComboBox(false, driversGroupBox);
	m_videoDriverComboBox->setEnabled(false);
	m_videoDriverComboBox->setEditable(true);
	m_videoDriverComboBox->addItems(QString("auto xv xvmc opengl xxmc sdl xshm fb XDirectFB DirectFB").split(' '));

	QGridLayout *driverLayout = createGridLayout(driversGroupBox);
	driverLayout->addWidget(m_audioDriverCheckBox, 0, 0, Qt::AlignRight | Qt::AlignVCenter);
	driverLayout->addWidget(m_audioDriverComboBox, 0, 1);
	driverLayout->addWidget(m_videoDriverCheckBox, 1, 0, Qt::AlignRight | Qt::AlignVCenter);
	driverLayout->addWidget(m_videoDriverComboBox, 1, 1);

	connect(m_audioDriverCheckBox, SIGNAL(toggled(bool)), m_audioDriverComboBox, SLOT(setEnabled(bool)));
	connect(m_videoDriverCheckBox, SIGNAL(toggled(bool)), m_videoDriverComboBox, SLOT(setEnabled(bool)));
	connect(m_audioDriverCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(settingsChanged()));
	connect(m_videoDriverCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(settingsChanged()));
	connect(m_audioDriverComboBox, SIGNAL(textChanged(const QString &)), this, SIGNAL(settingsChanged()));
	connect(m_videoDriverComboBox, SIGNAL(textChanged(const QString &)), this, SIGNAL(settingsChanged()));

	setControlsFromConfig();
}

XineConfigWidget::~XineConfigWidget()
{}

void
XineConfigWidget::setConfigFromControls()
{
	config()->setAudioDriver(m_audioDriverCheckBox->isChecked() ? m_audioDriverComboBox->currentText() : QString());
	config()->setVideoDriver(m_videoDriverCheckBox->isChecked() ? m_videoDriverComboBox->currentText() : QString());
}

void
XineConfigWidget::setControlsFromConfig()
{
	m_audioDriverCheckBox->setChecked(config()->hasAudioDriver());
	if(m_audioDriverCheckBox->isChecked())
		m_audioDriverComboBox->setEditText(config()->audioDriver());

	m_videoDriverCheckBox->setChecked(config()->hasVideoDriver());
	if(m_videoDriverCheckBox->isChecked())
		m_videoDriverComboBox->setEditText(config()->videoDriver());
}

#include "xineconfigwidget.moc"
