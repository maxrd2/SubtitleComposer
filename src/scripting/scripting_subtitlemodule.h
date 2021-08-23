/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SCRIPTING_SUBTITLEMODULE_H
#define SCRIPTING_SUBTITLEMODULE_H

#include "core/subtitletarget.h"

#include <QObject>

namespace SubtitleComposer {
namespace Scripting {
class SubtitleModule : public QObject
{
	Q_OBJECT

	Q_ENUMS(TextTarget)

public:
	SubtitleModule(QObject *parent = 0);

	using TextTarget = SubtitleComposer::SubtitleTarget;

public slots:
	QObject * instance();

	bool translationMode();
};
}
}
#endif
