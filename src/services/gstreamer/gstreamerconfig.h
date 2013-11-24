#ifndef GSTREAMERCONFIG_H
#define GSTREAMERCONFIG_H

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
class GStreamerConfig : public AppConfigGroup
{
	friend class GStreamerPlayerBackend;
	friend class GStreamerDecoderBackend;
	friend class GStreamerConfigWidget;

public:

	virtual AppConfigGroup * clone() const
	{
		return new GStreamerConfig(*this);
	}

	bool hasAudioSink() const
	{
		return !option(keyAudioSink()).isEmpty();
	}

	QString audioSink() const
	{
		return option(keyAudioSink());
	}

	void setAudioSink(const QString &audioSink)
	{
		setOption(keyAudioSink(), audioSink);
	}

	bool hasVideoSink() const
	{
		return !option(keyVideoSink()).isEmpty();
	}

	QString videoSink() const
	{
		return option(keyVideoSink());
	}

	void setVideoSink(const QString &videoSink)
	{
		setOption(keyVideoSink(), videoSink);
	}

	bool experimentalFeatures() const
	{
		return optionAsBool(keyExperimentalFeatures());
	}

	void setExperimentalFeatures(bool value)
	{
		setOption(keyExperimentalFeatures(), value);
	}

	static const QString & keyAudioSink()
	{
		static const QString key("AudioSink");
		return key;
	}

	static const QString & keyVideoSink()
	{
		static const QString key("VideoSink");
		return key;
	}

	static const QString & keyExperimentalFeatures()
	{
		static const QString key("ExperimentalFeatures");
		return key;
	}

private:

	GStreamerConfig() : AppConfigGroup("GStreamer", defaults()) {}

	GStreamerConfig(const GStreamerConfig &config) : AppConfigGroup(config) {}

	static QMap<QString, QString> defaults()
	{
		QMap<QString, QString> defaults;

		defaults[keyAudioSink()] = "";
		defaults[keyVideoSink()] = "";
		defaults[keyExperimentalFeatures()] = "true";

		return defaults;
	}
};
}

#endif
