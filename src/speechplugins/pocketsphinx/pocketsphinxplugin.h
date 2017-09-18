#ifndef POCKETSPHINXPLUGIN_H
#define POCKETSPHINXPLUGIN_H
/**
 * Copyright (C) 2010-2016 Mladen Milinkovic <max@smoothware.net>
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

private:
	virtual const QString & name();

	virtual bool init();
	virtual void cleanup();

	virtual void processSamples(const qint16 *sampleData, qint32 sampleCount);
	virtual void processComplete();

	virtual void setSCConfig(SCConfig *scConfig);

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
