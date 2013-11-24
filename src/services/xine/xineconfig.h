#ifndef XINECONFIG_H
#define XINECONFIG_H

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

#include "../../config/appconfiggroup.h"

namespace SubtitleComposer {
class XineConfig : public AppConfigGroup
{
	friend class XinePlayerBackend;
	friend class XineDecoderBackend;
	friend class XineConfigWidget;

public:

	virtual AppConfigGroup * clone() const
	{
		return new XineConfig(*this);
	}

	bool hasAudioDriver() const
	{
		return !option(keyAudioDriver()).isEmpty();
	}

	QString audioDriver() const
	{
		return option(keyAudioDriver());
	}

	void setAudioDriver(const QString &audioDriver)
	{
		setOption(keyAudioDriver(), audioDriver);
	}

	bool hasVideoDriver() const
	{
		return !option(keyVideoDriver()).isEmpty();
	}

	QString videoDriver() const
	{
		return option(keyVideoDriver());
	}

	void setVideoDriver(const QString &videoDriver)
	{
		setOption(keyVideoDriver(), videoDriver);
	}

	static const QString & keyAudioDriver()
	{
		static const QString key("AudioDriver");
		return key;
	}

	static const QString & keyVideoDriver()
	{
		static const QString key("VideoDriver");
		return key;
	}

private:

	XineConfig() : AppConfigGroup("Xine", defaults()) {}

	XineConfig(const XineConfig &config) : AppConfigGroup(config) {}

	static QMap<QString, QString> defaults()
	{
		QMap<QString, QString> defaults;

		defaults[keyAudioDriver()] = "";
		defaults[keyVideoDriver()] = "";

		return defaults;
	}
};
}

#endif
