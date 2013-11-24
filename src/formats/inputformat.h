#ifndef INPUTFORMAT_H
#define INPUTFORMAT_H

/***************************************************************************
 *   Copyright (C) 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "format.h"

namespace SubtitleComposer {
class InputFormat : public Format
{
public:
	virtual ~InputFormat() {}

	bool readSubtitle(Subtitle &subtitle, bool primary, const QString &data) const
	{
		Subtitle newSubtitle;

		if(!parseSubtitles(newSubtitle, data))
			return false;

		if(primary)
			subtitle.setPrimaryData(newSubtitle, true);
		else
			subtitle.setSecondaryData(newSubtitle, true);

		return true;
	}

protected:
	virtual bool parseSubtitles(Subtitle &subtitle, const QString &data) const = 0;

	InputFormat(const QString &name, const QStringList &extensions) : Format(name, extensions) {}
};
}

#endif
