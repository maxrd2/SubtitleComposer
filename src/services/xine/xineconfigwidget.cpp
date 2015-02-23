/**
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2015 Mladen Milinkovic <max@smoothware.net>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "xineconfigwidget.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QCheckBox>

#include <KLocale>
#include <KComboBox>

using namespace SubtitleComposer;

XineConfigWidget::XineConfigWidget(QWidget *parent) :
	QWidget(parent)
{
	setupUi(this);

	kcfg_xineAudio->addItems(QString("auto alsa oss jack pulseaudio esd").split(' '));
	kcfg_xineAudio->setProperty("kcfg_property", QByteArray("currentText"));

	kcfg_xineVideo->addItems(QString("auto xv xvmc opengl xxmc sdl xshm fb XDirectFB DirectFB").split(' '));
	kcfg_xineVideo->setProperty("kcfg_property", QByteArray("currentText"));
}

XineConfigWidget::~XineConfigWidget()
{

}
