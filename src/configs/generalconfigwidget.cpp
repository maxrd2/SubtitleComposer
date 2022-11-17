/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "generalconfigwidget.h"

#include "appglobal.h"
#include "application.h"

using namespace SubtitleComposer;

GeneralConfigWidget::GeneralConfigWidget(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);

	kcfg_DefaultSubtitlesEncoding->addItems(app()->availableEncodingNames());
	kcfg_DefaultSubtitlesEncoding->setProperty("kcfg_property", QByteArray("currentText"));
}

GeneralConfigWidget::~GeneralConfigWidget()
{

}
