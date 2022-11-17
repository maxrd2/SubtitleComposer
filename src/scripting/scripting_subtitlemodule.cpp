/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "scripting_subtitlemodule.h"

#include "appglobal.h"
#include "application.h"
#include "scripting_subtitle.h"

using namespace SubtitleComposer;

Scripting::SubtitleModule::SubtitleModule(QObject *parent) :
	QObject(parent)
{}

QObject *
Scripting::SubtitleModule::instance()
{
	return appSubtitle() ? new Scripting::Subtitle(appSubtitle(), this) : nullptr;
}

bool
Scripting::SubtitleModule::translationMode()
{
	return app()->translationMode();
}


