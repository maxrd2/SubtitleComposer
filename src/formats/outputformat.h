/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef OUTPUTFORMAT_H
#define OUTPUTFORMAT_H

#include "format.h"

namespace SubtitleComposer {
class OutputFormat : public Format
{
public:

	QString writeSubtitle(const Subtitle &subtitle, bool primary) const
	{
		return dumpSubtitles(subtitle, primary);
	}

protected:
	virtual QString dumpSubtitles(const Subtitle &subtitle, bool primary) const = 0;

	OutputFormat(const QString &name, const QStringList &extensions) : Format(name, extensions) {}
};
}

#endif
