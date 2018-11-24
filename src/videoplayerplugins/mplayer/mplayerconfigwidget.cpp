/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2018 Mladen Milinkovic <max@smoothware.net>
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

#include "mplayerconfigwidget.h"

using namespace SubtitleComposer;

MPlayerConfigWidget::MPlayerConfigWidget(QWidget *parent) :
	QWidget(parent)
{
	setupUi(this);

	kcfg_mpVideoOutput->addItems(QStringLiteral("vdpau xv gl_nosw x11 xover sdl gl gl_tiled gl2 dga fbdev fbdev2 matrixview caca v4l2 xvidix xvmc mpegpes tdfxfb s3fb ggi bl directfb dfbmga null").split(' '));
	kcfg_mpVideoOutput->setProperty("kcfg_property", QByteArray("currentText"));

	kcfg_mpAudioOutput->addItems(QStringLiteral("pulse alsa oss esd jack nas sdl openal v4l2 null").split(' '));
	kcfg_mpAudioOutput->setProperty("kcfg_property", QByteArray("currentText"));
}

MPlayerConfigWidget::~MPlayerConfigWidget()
{

}
