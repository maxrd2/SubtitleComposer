/*
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef POCKETSPHINXPLUGIN_H
#define POCKETSPHINXPLUGIN_H

#include "speechprocessor/speechplugin.h"

typedef struct ps_decoder_s ps_decoder_t;
typedef struct cmd_ln_s cmd_ln_t;
typedef struct SpeexPreprocessState_ SpeexPreprocessState;

namespace SubtitleComposer {
class PocketSphinxPlugin : public SpeechPlugin
{
	Q_OBJECT

	Q_PLUGIN_METADATA(IID SpeechPlugin_iid)
	Q_INTERFACES(SubtitleComposer::SpeechPlugin)

public:
	PocketSphinxPlugin();

	QWidget * newConfigWidget(QWidget *parent) override;
	KCoreConfigSkeleton * config() const override;

private:
	const QString & name() override;

	bool init() override;
	void cleanup() override;

	void processSamples(const qint16 *sampleData, qint32 sampleCount) override;
	void processComplete() override;

	void processUtterance();

private:
	cmd_ln_t *m_psConfig;
	ps_decoder_t *m_psDecoder;
	qint32 m_psFrameRate;

	QString m_lineText;
	int m_lineIn;
	int m_lineOut;

	bool m_utteranceStarted;
	bool m_speechStarted;
};
}

#endif // POCKETSPHINXPLUGIN_H
