/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "scripting_subtitlemodule.h"
#include "scripting_subtitle.h"
#include "application.h"
#include "core/subtitletarget.h"

using namespace SubtitleComposer;

Scripting::SubtitleModule::SubtitleModule(QObject *parent) :
	QObject(parent)
{}

QObject *
Scripting::SubtitleModule::instance()
{
	return app()->subtitle() ? new Scripting::Subtitle(app()->subtitle(), this) : 0;
}

bool
Scripting::SubtitleModule::translationMode()
{
	return app()->translationMode();
}


