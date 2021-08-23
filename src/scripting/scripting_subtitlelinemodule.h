/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SCRIPTING_SUBTITLELINEMODULE_H
#define SCRIPTING_SUBTITLELINEMODULE_H

#include "core/subtitleline.h"
#include "core/subtitletarget.h"

#include <QObject>

namespace SubtitleComposer {
namespace Scripting {
class SubtitleLineModule : public QObject
{
	Q_OBJECT

	Q_ENUMS(TextTarget)
	Q_ENUMS(ErrorFlag)

public:
	using TextTarget = SubtitleComposer::SubtitleTarget;
	using ErrorFlag = SubtitleComposer::SubtitleLine::ErrorFlag;

	SubtitleLineModule(QObject *parent = 0);
};
}
}
#endif
