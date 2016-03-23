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

#include <QLabel>
#include <QProgressBar>
#include <QBoxLayout>
#include <QThread>

#include <KLocalizedString>

using namespace SubtitleComposer;

SpeechProcessor::SpeechProcessor(QWidget *parent)
	: QObject(parent),
	  m_mediaFile(QString()),
	  m_streamIndex(-1),
	  m_stream(new StreamProcessor(this)),
	  m_subtitle(NULL),
	  m_progressWidget(new QWidget(parent)),
	  m_psConfig(NULL),
	  m_psDecoder(NULL)/*,
	  m_sampleFrame(new qint16[SAMPLE_FRAME_SIZE]),
	  m_sampleFrameLen(0)*/
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

	m_psConfig = cmd_ln_init(NULL, ps_args(), TRUE,
				 "-hmm", POCKETSPHINX_MODELDIR "/en-us/en-us",
				 "-lm", POCKETSPHINX_MODELDIR "/en-us/en-us.lm.bin",
				 "-dict", POCKETSPHINX_MODELDIR "/en-us/cmudict-en-us.dict",
//				 "-frate", "100",
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

	m_psFrameRate = cmd_ln_int32_r(m_psConfig, "-frate");

	m_mediaFile = mediaFile;
	m_streamIndex = audioStream;

	m_audioDuration = 0;

	m_utteranceStarted = false;
	m_speechStarted = false;

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
	if(m_psDecoder) {
		if(m_utteranceStarted)
			ps_end_utt(m_psDecoder);
		processUtterance();
	}
	clearAudioStream();
}

void
SpeechProcessor::processUtterance()
{
	if(!m_subtitle)
		return;

	qint32 score;
	char const *hyp = ps_get_hyp(m_psDecoder, &score);
	if(!hyp || !*hyp)
		return;

	qDebug() << "RECOGNIZED:" << hyp;

	QString lineText;
	int lineIn, lineOut;
	ps_seg_t *iter = ps_seg_iter(m_psDecoder);
	while(iter != NULL && m_subtitle != NULL) {
		const char *word = ps_seg_word(iter);
		int wordIn, wordOut;
		ps_seg_frames(iter, &wordIn, &wordOut);
		if(*word == '<' || *word == '['
//		|| strcmp(word, "<s>") == 0
//		|| strcmp(word, "</s>") == 0
//		|| strcmp(word, "<sil>") == 0
//		|| strcmp(word, "[SPEECH]") == 0
		) {
			if(!lineText.isEmpty()) {
				m_subtitle->insertLine(new SubtitleLine(SString(lineText),
														double(lineIn) * 1000. / double(m_psFrameRate),
														double(lineOut) * 1000. / double(m_psFrameRate)));
				lineText.clear();
			}
		} else {
			if(lineText.isEmpty()) {
				lineText = QString::fromLatin1(word);
				lineIn = wordIn;
			} else {
				lineText += ' ';
				lineText += QString::fromLatin1(word);
			}
			lineOut = wordOut;
		}

		qDebug() << word << ": " << wordIn << "-" << wordOut;
		iter = ps_seg_next(iter);
	}
	if(!lineText.isEmpty()) {
		m_subtitle->insertLine(new SubtitleLine(SString(lineText),
												double(lineIn) * 1000. / double(m_psFrameRate),
												double(lineOut) * 1000. / double(m_psFrameRate)));
	}
}

void
SpeechProcessor::onStreamData(const void *buffer, const qint32 size, const WaveFormat */*waveFormat*/, const quint64 /*msecStart*/, const quint64 /*msecDuration*/)
{
	// make sure SpeechProcessor::onStreamProgress() signal was processed since we're in different thread
	while(!m_audioDuration)
		QThread::yieldCurrentThread();

	Q_ASSERT(size % 2 == 0);

	qint16 *sampleData = reinterpret_cast<qint16 *>(const_cast<void *>(buffer));
	int sampleCount = size / 2;

	if(!m_utteranceStarted) {
		ps_start_utt(m_psDecoder);
		m_utteranceStarted = true;
		m_speechStarted = false;
	}

	ps_process_raw(m_psDecoder, sampleData, sampleCount, FALSE, FALSE);

	if(ps_get_in_speech(m_psDecoder)) {
		m_speechStarted = true;
	} else {
		if(m_utteranceStarted && m_speechStarted) {
			ps_end_utt(m_psDecoder);
			processUtterance();
			m_speechStarted = false;
			m_utteranceStarted = false;
		}
	}
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
