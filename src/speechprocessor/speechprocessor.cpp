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

using namespace SubtitleComposer;

SpeechProcessor::SpeechProcessor(QWidget *parent)
	: QObject(parent),
	  m_mediaFile(QString()),
	  m_streamIndex(-1),
	  m_stream(new StreamProcessor(this)),
	  m_subtitle(NULL),
	  m_progressWidget(new QWidget(parent))
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
	connect(m_stream, &StreamProcessor::textDataAvailable, this, &SpeechProcessor::onStreamData);
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

	m_audioDuration = 0;

	m_mediaFile = mediaFile;
	m_streamIndex = audioStream;

	if(m_stream->open(mediaFile) && m_stream->initSpeech(audioStream))
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
SpeechProcessor::onStreamData(const QString &text, const quint64 msecStart, const quint64 msecDuration)
{
	m_subtitle->insertLine(new SubtitleLine(SString(text), msecStart, msecStart + msecDuration));
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
