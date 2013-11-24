#ifndef GENERALCONFIG_H
#define GENERALCONFIG_H

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

#include <QtCore/QTextCodec>

#include <KGlobal>
#include <KLocale>
#include <KCharsets>

namespace SubtitleComposer {
class GeneralConfig : public AppConfigGroup
{
	friend class Application;
	friend class GeneralConfigWidget;

public:

	virtual AppConfigGroup * clone() const { return new GeneralConfig(*this); }
	QString defaultSubtitlesEncoding() const { return option(keyDefaultSubtitlesEncoding()); }
	QTextCodec * defaultSubtitlesCodec() const
	{
		bool encodingFound;
		QTextCodec *codec = KGlobal::charsets()->codecForName(defaultSubtitlesEncoding(), encodingFound);
		if(!encodingFound)
			codec = KGlobal::locale()->codecForEncoding();
		return codec;
	}

	void setDefaultSubtitlesEncoding(const QString &encoding) { setOption(keyDefaultSubtitlesEncoding(), encoding); }

	int seekOffsetOnDoubleClick() const { return optionAsInt(keySeekOffsetOnDoubleClick()); } // in milliseconds

	void setSeekOffsetOnDoubleClick(int mseconds) { setOption(keySeekOffsetOnDoubleClick(), mseconds); }
	bool automaticVideoLoad() const { return optionAsBool(keyAutomaticVideoLoad()); }
	void setAutomaticVideoLoad(bool automaticLoad) { setOption(keyAutomaticVideoLoad(), automaticLoad); }

	int linesQuickShiftAmount() const { return optionAsInt(keyLinesQuickShiftAmount()); }  // in milliseconds
	void setLinesQuickShiftAmount(int mseconds) { setOption(keyLinesQuickShiftAmount(), mseconds); }

	int grabbedPositionCompensation() const { return optionAsInt(keyGrabbedPositionCompensation()); } // in milliseconds
	void setGrabbedPositionCompensation(int mseconds) { setOption(keyGrabbedPositionCompensation(), mseconds); }

	static const QString & keyDefaultSubtitlesEncoding() { static const QString key("DefaultSubtitlesEncoding"); return key; }

	static const QString & keySeekOffsetOnDoubleClick() { static const QString key("SeekOffsetOnDoubleClick"); return key; }

	static const QString & keyAutomaticVideoLoad() { static const QString key("AutomaticVideoLoad"); return key; }

	static const QString & keyLinesQuickShiftAmount() { static const QString key("LinesQuickShiftAmount"); return key; }

	static const QString & keyGrabbedPositionCompensation() { static const QString key("GrabbedPositionCompensation"); return key; }

private:

	GeneralConfig() : AppConfigGroup("General", defaults()) {}

	GeneralConfig(const GeneralConfig &config) : AppConfigGroup(config) {}

	static QMap<QString, QString> defaults()
	{
		QMap<QString, QString> defaults;

		defaults[keyDefaultSubtitlesEncoding()] = KGlobal::locale()->codecForEncoding()->name();
		defaults[keySeekOffsetOnDoubleClick()] = "1500";        // in milliseconds
		defaults[keyAutomaticVideoLoad()] = "true";
		defaults[keyLinesQuickShiftAmount()] = "100";   // in milliseconds
		defaults[keyGrabbedPositionCompensation()] = "250";     // in milliseconds

		return defaults;
	}
};
}

#endif
