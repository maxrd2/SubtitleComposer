/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INPUTFORMAT_H
#define INPUTFORMAT_H

#include "format.h"
#include "formatmanager.h"

namespace SubtitleComposer {
class InputFormat : public Format
{
public:
	bool readSubtitle(Subtitle &subtitle, bool primary, const QString &data) const
	{
		QExplicitlySharedDataPointer<Subtitle> newSubtitle(new Subtitle());

		if(!parseSubtitles(*newSubtitle, data))
			return false;

		if(primary)
			subtitle.setPrimaryData(*newSubtitle, true);
		else
			subtitle.setSecondaryData(*newSubtitle, true);

		return true;
	}

	virtual bool isBinary() const { return false; }
	virtual FormatManager::Status readBinary(Subtitle &, const QUrl &) { return FormatManager::ERROR; }

protected:
	virtual bool parseSubtitles(Subtitle &subtitle, const QString &data) const = 0;

	InputFormat(const QString &name, const QStringList &extensions) : Format(name, extensions) {}
};
}

#endif
