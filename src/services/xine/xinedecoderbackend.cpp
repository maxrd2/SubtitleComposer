/***************************************************************************
 *   Copyright (C) 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)  *
 *   based on Kaffeine by Jürgen Kofler                                    *
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

#include "xinedecoderbackend.h"
#include "xineconfig.h"
#include "xineconfigwidget.h"
#include "../wavewriter.h"

#include <QtCore/QEvent>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QThread>
#include <QtCore/QCoreApplication>

#include <KDebug>
#include <KLocale>
#include <KUrl>

using namespace SubtitleComposer;

#define EVENT_DECODING_STARTED (QEvent::User + 1)
#define EVENT_POSITION_UPDATED (QEvent::User + 2)
#define EVENT_ERROR_INCOMPATIBLE_DATA (QEvent::User + 3)
#define EVENT_ERROR_FILE_CLOSED (QEvent::User + 4)
#define EVENT_DECODING_FINISHED (QEvent::User + 5)

namespace SubtitleComposer {
class DecodingThread : public QThread
{
public:
	DecodingThread(XineDecoderBackend *backend, int audioStream, const QString &outputPath, const WaveFormat &outputFormat) :
		QThread(backend),
		m_backend(backend),
		m_audioStream(audioStream),
		m_waveWriter(),
		m_cancelled(false),
		m_finished(false)
	{
		m_cancelled = !m_waveWriter.open(outputPath, outputFormat);
	}

	bool isCancelled() const { return m_cancelled; }
	void setCancelled() { m_cancelled = true; }
	bool isFinished() const { return m_finished; }
	bool isWorking() const { return isRunning() && !m_finished && !m_cancelled; }
	double position() const { return m_waveWriter.writtenSeconds(); }

	void run()
	{
		const unsigned long bufferSize = 0x100000;              // 1MB
		char buffer[bufferSize];                //
		const WaveFormat inputFormat = m_backend->decoder()->audioStreamFormat(m_audioStream);

		if(!m_cancelled)
			QCoreApplication::postEvent(m_backend, new QEvent((QEvent::Type)EVENT_DECODING_STARTED));

		unsigned long pcmChunkSize;
		while(!m_cancelled && (pcmChunkSize = m_backend->readUncompressedData(buffer, bufferSize)) > 0) {
			if(m_waveWriter.writeSamplesData(buffer, pcmChunkSize, inputFormat))
				QCoreApplication::postEvent(m_backend, new QEvent((QEvent::Type)EVENT_POSITION_UPDATED));
			else {
				m_cancelled = true;
				if(m_waveWriter.isOpened())
					QCoreApplication::postEvent(m_backend, new QEvent((QEvent::Type)EVENT_ERROR_INCOMPATIBLE_DATA));
				else
					QCoreApplication::postEvent(m_backend, new QEvent((QEvent::Type)EVENT_ERROR_FILE_CLOSED));
			}
		}

		m_waveWriter.close();

		m_finished = true;

		QCoreApplication::postEvent(m_backend, new QEvent((QEvent::Type)EVENT_DECODING_FINISHED));
	}

private:
	XineDecoderBackend *m_backend;
	int m_audioStream;
	WaveWriter m_waveWriter;
	bool m_cancelled;
	bool m_finished;
};
}

XineDecoderBackend::XineDecoderBackend(Decoder *decoder) :
	DecoderBackend(decoder, "Xine", new XineConfig()),
	m_xineEngine(0),
	m_audioPort(0),
	m_xineStream(0),
	m_eventQueue(0),
	m_frame(0),
	m_isValidFrame(false),
	m_framePos(0),
	m_frameSize(0),
	m_decodingThread(0)
{}

XineDecoderBackend::~XineDecoderBackend()
{
	if(m_decodingThread && m_decodingThread->isWorking())
		m_decodingThread->setCancelled();

	if(isInitialized())
		finalizeXine();
}

QWidget *
XineDecoderBackend::initialize(QWidget * /*videoWidgetParent */)
{
	if(!initializeXine()) {
		finalizeXine();
		kError() << "xine initialization failed!";
		return (QWidget *)1;
	}

	return (QWidget *)0;
}

void
XineDecoderBackend::finalize()
{
	return finalizeXine();
}

SubtitleComposer::AppConfigGroupWidget *
XineDecoderBackend::newAppConfigGroupWidget(QWidget *parent)
{
	return new XineConfigWidget(parent);
}

bool
XineDecoderBackend::readNextFrame(bool first)
{
	if(m_isValidFrame)
		xine_free_audio_frame(m_audioPort, m_frame);

	m_framePos = 0;
	m_frameSize = 0;

	if(first && !xine_play(m_xineStream, 0, 0)) {
		m_isValidFrame = false;
		return false;
	}

	m_isValidFrame = xine_get_next_audio_frame(m_audioPort, m_frame) > 0;
	if(m_isValidFrame)
		m_frameSize = m_frame->num_samples * WaveFormat::blockAlign(m_frame->num_channels, m_frame->bits_per_sample);

	return m_isValidFrame;
}

bool
XineDecoderBackend::openFile(const QString &filePath)
{
	// Lame attempt at preventing opening of files with formats known to
	// "be problematic" (their Xine drivers in are buggy).
	// - Matroska files make xine_open() call hang
	// - Ogg Vorbis files make  xine_get_next_audio_frame() call hang for the last frame
	QString fileExtension = QFileInfo(filePath).suffix().toLower();
	if(fileExtension == "mkv" || fileExtension == "ogg")
		return false;

	KUrl fileUrl;
	fileUrl.setProtocol("file");
	fileUrl.setPath(filePath);

	if(!xine_open(m_xineStream, fileUrl.url().toLocal8Bit()))
		return false;

	QStringList audioStreamNames;
	QList<WaveFormat> audioStreamFormats;

	int channels = xine_get_stream_info(m_xineStream, XINE_STREAM_INFO_MAX_AUDIO_CHANNEL);

	if(!channels) {
		xine_close(m_xineStream);
		return false;
	}

	for(int index = 0; index < channels; ++index) {
		QString audioStreamName = i18n("Audio Stream #%1", index + 1);
		char lang[XINE_LANG_MAX];
		if(xine_get_audio_lang(m_xineStream, index, lang))
			audioStreamName += QString(" - ") + lang;

		xine_set_param(m_xineStream, XINE_PARAM_AUDIO_CHANNEL_LOGICAL, index);
		if(readNextFrame(true)) {
			audioStreamNames << audioStreamName;
			audioStreamFormats << WaveFormat(m_frame->sample_rate, m_frame->num_channels, m_frame->bits_per_sample);
		}
	}

	if(audioStreamNames.isEmpty()) {
		xine_close(m_xineStream);
		return false;
	}

	setDecoderState(Decoder::Ready);

	for(int index = 0, size = audioStreamNames.size(); index < size; ++index)
		appendDecoderAudioStream(audioStreamNames.at(index), audioStreamFormats.at(index));

	int time, length;
	if(xine_get_pos_length(m_xineStream, 0, &time, &length))
		setDecoderLength(length / 1000.0);

	return true;
}

void
XineDecoderBackend::closeFile()
{
	if(m_xineStream)
		xine_close(m_xineStream);
}

bool
XineDecoderBackend::decode(int audioStream, const QString &outputPath, const WaveFormat &outputFormat)
{
	if(m_decodingThread && m_decodingThread->isWorking())
		return false;

	xine_set_param(m_xineStream, XINE_PARAM_AUDIO_CHANNEL_LOGICAL, audioStream);
	if(!readNextFrame(true))
		return false;

	m_decodingThread = new DecodingThread(this, audioStream, outputPath, outputFormat);

	if(m_decodingThread->isCancelled())
		m_decodingThread = 0;
	else
		m_decodingThread->start();

	return m_decodingThread;
}

bool
XineDecoderBackend::stop()
{
	if(!m_decodingThread || !m_decodingThread->isWorking())
		return false;

	m_decodingThread->setCancelled();

	return true;
}

unsigned long
XineDecoderBackend::readUncompressedData(void *buffer, unsigned long bufferSize)
{
	unsigned long dataSize = 0, frameRemainingSize = 0;

	while(m_isValidFrame) {
		if(bufferSize && m_framePos < m_frameSize) {
			frameRemainingSize = m_frameSize - m_framePos;
			if(frameRemainingSize > bufferSize)
				frameRemainingSize = bufferSize;
			memcpy(((char *)buffer) + dataSize, m_frame->data + m_framePos, frameRemainingSize);
			m_framePos += frameRemainingSize;
			bufferSize -= frameRemainingSize;
			dataSize += frameRemainingSize;

			if(!bufferSize)
				break;
		}

		if(m_framePos >= m_frameSize)
			readNextFrame(false);
	}

	return dataSize;
}

void
XineDecoderBackend::customEvent(QEvent *event)
{
	switch((int)event->type()) {
	case EVENT_DECODING_STARTED:
		setDecoderState(Decoder::Decoding);
		break;

	case EVENT_POSITION_UPDATED:
		if(m_decodingThread)
			setDecoderPosition(m_decodingThread->position());
		break;

	case EVENT_ERROR_INCOMPATIBLE_DATA:
		setDecoderErrorState(i18n("There was an error converting audio data to output format."));
		break;

	case EVENT_ERROR_FILE_CLOSED:
	case EVENT_DECODING_FINISHED:
		setDecoderState(Decoder::Ready);
		break;

	default:
		DecoderBackend::customEvent(event);
		break;
	}
}

bool
XineDecoderBackend::initializeXine()
{
	if(!(m_xineEngine = xine_new())) {
		kDebug() << "Could not init Xine engine!";
		return false;
	}

	QString configFilePath(QDir::homePath() + "/.xine/config");
	if(QFile::exists(configFilePath))
		xine_config_load(m_xineEngine, QFile::encodeName(configFilePath));

	xine_init(m_xineEngine);

	m_audioPort = xine_new_framegrab_audio_port(m_xineEngine);
	if(!m_audioPort) {
		kDebug() << "Could not get Xine framegrab audio port!";
		return false;
	}

	m_xineStream = xine_stream_new(m_xineEngine, m_audioPort, NULL);
	if(!m_xineStream) {
		kDebug() << "Could not create Xine stream!";
		return false;
	}

	xine_set_param(m_xineStream, XINE_PARAM_IGNORE_VIDEO, 1);

	m_eventQueue = xine_event_new_queue(m_xineStream);
	// xine_event_create_listener_thread(m_eventQueue, &XineDecoderBackend::xineEventListener, (void*)this);

	m_frame = new xine_audio_frame_t;

	return true;
}

void
XineDecoderBackend::finalizeXine()
{
	if(m_audioPort) {
		xine_close_audio_driver(m_xineEngine, m_audioPort);
		m_audioPort = 0;
	}

	if(m_xineEngine) {
		xine_exit(m_xineEngine);
		m_xineEngine = 0;
	}

	if(m_eventQueue) {
		xine_event_dispose_queue(m_eventQueue);
		m_eventQueue = 0;
	}

	if(m_xineStream) {
		// xine_dispose(m_xineStream);
		m_xineStream = 0;
	}

	if(m_frame) {
		delete m_frame;
		m_frame = 0;
	}
}

#include "xinedecoderbackend.moc"
