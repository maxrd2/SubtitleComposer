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

#include "config.h"
#include "speechprocessor.h"

#include <pocketsphinx.h>
#include <speex/speex_preprocess.h>

#include <QLabel>
#include <QProgressBar>
#include <QBoxLayout>
#include <QThread>

#include <KLocalizedString>

//#define SAMPLE_FRAME_SIZE 240
#define SAMPLE_FRAME_SIZE 80

using namespace SubtitleComposer;

SpeechProcessor::SpeechProcessor(QWidget *parent)
	: QObject(parent),
	  m_mediaFile(QString()),
	  m_streamIndex(-1),
	  m_stream(new StreamProcessor(this)),
	  m_subtitle(NULL),
	  m_progressWidget(new QWidget(parent)),
	  m_speexPP(NULL),
	  m_psConfig(NULL),
	  m_psDecoder(NULL),
	  m_sampleFrame(new qint16[SAMPLE_FRAME_SIZE]),
	  m_sampleFrameLen(0)
{
	// Progress Bar
	m_progressWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	m_progressWidget->hide();

	QLabel *label = new QLabel(i18n("Recognizing speech"), m_progressWidget);

	m_progressBar = new QProgressBar(m_progressWidget);
	m_progressBar->setMinimumWidth(300);
	m_progressBar->setTextVisible(true);

	QBoxLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight, m_progressWidget);
	layout->setContentsMargins(1, 0, 1, 0);
	layout->setSpacing(1);
	layout->addWidget(label);
	layout->addWidget(m_progressBar);

	connect(m_stream, &StreamProcessor::streamProgress, this, &SpeechProcessor::onStreamProgress);
	connect(m_stream, &StreamProcessor::streamError, this, &SpeechProcessor::onStreamError);
	connect(m_stream, &StreamProcessor::streamFinished, this, &SpeechProcessor::onStreamFinished);
	// Using Qt::DirectConnection here makes SpeechProcessor::onStreamData() to execute in GStreamer's thread
	connect(m_stream, &StreamProcessor::audioDataAvailable, this, &SpeechProcessor::onStreamData, Qt::DirectConnection);
}

SpeechProcessor::~SpeechProcessor()
{
	m_progressWidget = NULL;
	clearAudioStream();

	delete[] m_sampleFrame;
}

void
SpeechProcessor::setSubtitle(Subtitle *subtitle)
{
	m_subtitle = subtitle;
	clearAudioStream();
}

QWidget *
SpeechProcessor::progressWidget()
{
	return m_progressWidget;
}

void
SpeechProcessor::setAudioStream(const QString &mediaFile, int audioStream)
{
	if(m_mediaFile == mediaFile && audioStream == m_streamIndex)
		return;

	clearAudioStream();

	m_speexPP = speex_preprocess_state_init(SAMPLE_FRAME_SIZE, 16000);

	int value = 1;
	speex_preprocess_ctl(m_speexPP, SPEEX_PREPROCESS_SET_VAD, &value);
//	speex_preprocess_ctl(m_speexPP, SPEEX_PREPROCESS_GET_PROB_START, &value);
//	qDebug() << "Prob start" << value;
//	speex_preprocess_ctl(m_speexPP, SPEEX_PREPROCESS_GET_PROB_CONTINUE, &value);
//	qDebug() << "Prob continue" << value;
//	value = 50;
//	speex_preprocess_ctl(m_speexPP, SPEEX_PREPROCESS_SET_PROB_START, &value);
//	value = 15;
//	speex_preprocess_ctl(m_speexPP, SPEEX_PREPROCESS_SET_PROB_CONTINUE, &value);

//	speex_preprocess_ctl(m_speexPP, SPEEX_PREPROCESS_SET_AGC, &value);

	m_sampleFrameLen = 0;

	m_psConfig = cmd_ln_init(NULL, ps_args(), TRUE,
				 "-hmm", POCKETSPHINX_MODELDIR "/en-us/en-us",
				 "-lm", POCKETSPHINX_MODELDIR "/en-us/en-us.lm.bin",
				 "-dict", POCKETSPHINX_MODELDIR "/en-us/cmudict-en-us.dict",
				 NULL);
	if(m_psConfig == NULL) {
		qWarning() << "Failed to create PocketSphinx config object";
		return;
	}

	m_psDecoder = ps_init(m_psConfig);
	if(m_psDecoder == NULL) {
		qWarning() << "Failed to create PocketSphinx recognizer";
		return;
	}

	m_mediaFile = mediaFile;
	m_streamIndex = audioStream;

	m_audioDuration = 0;
	m_lastFrameWasSilence = true;

	static WaveFormat waveFormat(16000, 1, 16, true);
	if(m_stream->open(mediaFile) && m_stream->initAudio(audioStream, waveFormat))
		m_stream->start();
}

void
SpeechProcessor::clearAudioStream()
{
	if(m_progressWidget)
		m_progressWidget->hide();

	m_stream->close();

	m_mediaFile.clear();
	m_streamIndex = -1;

	if(m_psDecoder != NULL) {
		ps_free(m_psDecoder);
		m_psDecoder = NULL;
	}
	if(m_psConfig != NULL) {
		cmd_ln_free_r(m_psConfig);
		m_psConfig = NULL;
	}

	if(m_speexPP != NULL) {
		speex_preprocess_state_destroy(m_speexPP);
		m_speexPP = NULL;
	}
}

void
SpeechProcessor::onStreamProgress(quint64 msecPos, quint64 msecLength)
{
	if(!m_audioDuration) {
		m_audioDuration = msecLength / 1000;
		m_progressBar->setRange(0, m_audioDuration);
		m_progressWidget->show();
	}
	m_progressBar->setValue(msecPos / 1000);
}

void
SpeechProcessor::onStreamFinished()
{
	clearAudioStream();
}

void
SpeechProcessor::processSamples(qint16 *sampleData, qint16 sampleCount, const quint64 msecStart)
{
	Q_ASSERT(m_sampleFrameLen == 0);

	qint32 score;
	char const *hyp;

	while(sampleCount >= SAMPLE_FRAME_SIZE) {
		if(speex_preprocess_run(m_speexPP, sampleData)) {
			if(m_lastFrameWasSilence) {
				ps_start_utt(m_psDecoder);
				m_timeStart = msecStart;
				m_timeDuration = 0.;
			}
			m_timeDuration += double(SAMPLE_FRAME_SIZE) / 16.;
			ps_process_raw(m_psDecoder, sampleData, SAMPLE_FRAME_SIZE, FALSE, FALSE);
			m_lastFrameWasSilence = false;
		} else {
			if(!m_lastFrameWasSilence) {
				ps_end_utt(m_psDecoder);
				// TODO: fetch start time
				hyp = /*ps_get_hyp*/ps_get_hyp_final(m_psDecoder, &score);
				if(hyp && *hyp && m_subtitle)
					m_subtitle->insertLine(new SubtitleLine(SString(QString::fromLatin1(hyp)), m_timeStart, m_timeStart + m_timeDuration));
			}
			m_lastFrameWasSilence = true;
		}

		sampleData += SAMPLE_FRAME_SIZE;
		sampleCount -= SAMPLE_FRAME_SIZE;
	}

	if(sampleCount > 0) {
		memcpy(m_sampleFrame, sampleData, sampleCount * 2);
		m_sampleFrameLen = sampleCount;
	}
}

void
SpeechProcessor::onStreamData(const void *buffer, const qint32 size, const WaveFormat */*waveFormat*/, const quint64 msecStart, const quint64 /*msecDuration*/)
{
	// make sure SpeechProcessor::onStreamProgress() signal was processed since we're in different thread
	while(!m_audioDuration)
		QThread::yieldCurrentThread();

	Q_ASSERT(size % 2 == 0);

	qint16 *sampleData = reinterpret_cast<qint16 *>(const_cast<void *>(buffer));
	int sampleCount = size / 2;

	if(m_sampleFrameLen > 0) {
		int missingSamples = SAMPLE_FRAME_SIZE - m_sampleFrameLen;
		if(missingSamples > sampleCount)
			missingSamples = sampleCount;
		memcpy(&m_sampleFrame[m_sampleFrameLen], buffer, missingSamples * 2);
		sampleData += missingSamples;
		sampleCount -= missingSamples;
		m_sampleFrameLen += missingSamples;
		if(m_sampleFrameLen != SAMPLE_FRAME_SIZE) {
			Q_ASSERT(sampleCount == 0);
			return;
		}
		m_sampleFrameLen = 0;
		processSamples(m_sampleFrame, SAMPLE_FRAME_SIZE, msecStart);
	}

	if(sampleCount > 0)
		processSamples(sampleData, sampleCount, msecStart);
}

void
SpeechProcessor::onStreamError(int code, const QString &message, const QString &debug)
{
	emit onError(i18n("Subtitle demux failed %1: %2\n%3")
				 .arg(code)
				 .arg(message)
				 .arg(debug));

	clearAudioStream();
}
