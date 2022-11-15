/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "scripting_stringsmodule.h"
#include "scripting_richstring.h"

using namespace SubtitleComposer;

Scripting::StringsModule::StringsModule(QObject *parent) :
	QObject(parent)
{}

QObject *
Scripting::StringsModule::newString(const QString &text)
{
	return new RichString(SubtitleComposer::RichString(text), this);
}


