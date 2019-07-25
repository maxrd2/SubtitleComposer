/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2019 Mladen Milinkovic <max@smoothware.net>
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

#include "gstreamerconfigwidget.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QCheckBox>

using namespace SubtitleComposer;

GStreamerConfigWidget::GStreamerConfigWidget(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);

	kcfg_audioSink->addItems(QStringLiteral("autoaudiosink alsasink osssink esdsink gconfaudiosink pulsesink fakesink").split(' '));
	kcfg_audioSink->setProperty("kcfg_property", QByteArray("currentText"));

	kcfg_videoSink->addItems(QStringLiteral("autovideosink glimagesink xvimagesink ximagesink fakesink").split(' '));
	kcfg_videoSink->setProperty("kcfg_property", QByteArray("currentText"));
}

GStreamerConfigWidget::~GStreamerConfigWidget()
{

}
