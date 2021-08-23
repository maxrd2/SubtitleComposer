#ifndef SCRIPTING_STRINGSMODULE_H
#define SCRIPTING_STRINGSMODULE_H

/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "core/sstring.h"
#include "scripting_list.h"

#include <QObject>
#include <QString>

namespace SubtitleComposer {
namespace Scripting {
class StringsModule : public QObject
{
	Q_OBJECT

	Q_ENUMS(StyleFlag)

public:
	using StyleFlag = SubtitleComposer::SString::StyleFlag;

	StringsModule(QObject *parent = 0);

public slots:
	QObject * newString(const QString &text = QString());
};
}
}
#endif
