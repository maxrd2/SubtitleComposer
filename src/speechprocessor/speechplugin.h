/*
    SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SPEECHPLUGIN_H
#define SPEECHPLUGIN_H

#include <QObject>

#define SpeechPlugin_iid "org.kde.SubtitleComposer.SpeechPlugin"

class KCoreConfigSkeleton;

namespace SubtitleComposer {
class SpeechPlugin : public QObject
{
	Q_OBJECT

	friend class SpeechProcessor;
	friend class ConfigDialog;
	template <class C, class T> friend class PluginHelper;

protected:
	explicit SpeechPlugin();

	virtual QWidget * newConfigWidget(QWidget *parent) = 0;
	virtual KCoreConfigSkeleton * config() const = 0;

private:
	virtual const QString & name() = 0;

	virtual bool init() = 0;
	virtual void cleanup() = 0;

	virtual void processSamples(const qint16 *sampleData, qint32 sampleCount) = 0;
	virtual void processComplete() = 0;

signals:
	void error(int code, const QString &message);
	void textRecognized(const QString &text, const double milliShow, const double milliHide);
};
}

Q_DECLARE_INTERFACE(SubtitleComposer::SpeechPlugin, SpeechPlugin_iid)

#endif // SPEECHPLUGIN_H
