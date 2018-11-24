#ifndef SPEECHPLUGIN_H
#define SPEECHPLUGIN_H
/*
 * Copyright (C) 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <QObject>

#define SpeechPlugin_iid "org.kde.SubtitleComposer.SpeechPlugin"

class SCConfig;

namespace SubtitleComposer {
class SpeechPlugin : public QObject
{
	Q_OBJECT

	friend class SpeechProcessor;

protected:
	explicit SpeechPlugin();

private:
	virtual const QString & name() = 0;

	virtual bool init() = 0;
	virtual void cleanup() = 0;

	virtual void processSamples(const qint16 *sampleData, qint32 sampleCount) = 0;
	virtual void processComplete() = 0;

	virtual void setSCConfig(SCConfig *scConfig) = 0;

signals:
	void error(int code, const QString &message);
	void textRecognized(const QString &text, const double milliShow, const double milliHide);
};
}

Q_DECLARE_INTERFACE(SubtitleComposer::SpeechPlugin, SpeechPlugin_iid)

#endif // SPEECHPLUGIN_H
