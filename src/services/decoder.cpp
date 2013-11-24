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

#include "decoder.h"
#include "decoderbackend.h"

#ifdef HAVE_GSTREAMER
#include "gstreamer/gstreamerdecoderbackend.h"
#endif
#ifdef HAVE_XINE
#include "xine/xinedecoderbackend.h"
#endif

#include <QtCore/QTimer>
#include <QtCore/QFileInfo>

#include <KDebug>

#define DEFAULT_MIN_POSITION_DELTA 0.02

namespace SubtitleComposer {
class DummyDecoderBackend : public DecoderBackend
{
public:
	DummyDecoderBackend(Decoder *decoder) : DecoderBackend(decoder, "Dummy", new AppConfigGroup("Dummy", QMap<QString, QString>())) {}

	virtual ~DummyDecoderBackend() {}

	virtual AppConfigGroupWidget * newAppConfigGroupWidget(QWidget * /*parent */) { return 0; }

protected:
	virtual QWidget * initialize(QWidget * /*widgetParent */) { return (QWidget *)1; }

	virtual void finalize() {}

	virtual bool openFile(const QString & /*filePath */) { return false; }

	virtual void closeFile() {}

	virtual bool decode(int /*audioStream */, const QString & /*outputPath */, const WaveFormat & /*outputFormat */) { return false; }

	virtual bool stop() { return false; }
};
}

using namespace SubtitleComposer;

Decoder::Decoder() :
	m_filePath(),
	m_position(-1.0),
	m_length(-1.0),
	m_audioStreamNames(),
	m_audioStreamFormats(),
	m_openFileTimer(new QTimer(this))
{
	addBackend(new DummyDecoderBackend(this));

#ifdef HAVE_GSTREAMER
	addBackend(new GStreamerDecoderBackend(this));
#endif

#ifdef HAVE_XINE
	addBackend(new XineDecoderBackend(this));
#endif

	// the timeout might seem too much, but it only matters when the file couldn't be
	// opened, and it's better to have the user wait a bit in that case than showing
	// an error we there's nothing wrong with the file (a longer time might be needed
	// for example if the computer is under heavy load or is just slow)

	m_openFileTimer->setSingleShot(true);

	connect(m_openFileTimer, SIGNAL(timeout()), this, SLOT(onOpenFileTimeout()));
}

Decoder::~Decoder()
{}

Decoder *
Decoder::instance()
{
	static Decoder Decoder;

	return &Decoder;
}

bool
Decoder::initializeBackend(ServiceBackend *backend, QWidget * /*widgetParent */)
{
	return backend->initialize(0) == 0;
}

void
Decoder::finalizeBackend(ServiceBackend *backend)
{
	closeFile();

	backend->finalize();
}

const QStringList &
Decoder::audioStreamNames() const
{
	static const QStringList emptyList;

	return m_state <= Decoder::Opening ? emptyList : m_audioStreamNames;
}

const QList<WaveFormat> &
Decoder::audioStreamFormats() const
{
	static const QList<WaveFormat> emptyList;

	return m_state <= Decoder::Opening ? emptyList : m_audioStreamFormats;
}

void
Decoder::resetState()
{
	if(m_openFileTimer->isActive())
		m_openFileTimer->stop();

	m_filePath.clear();

	m_position = -1.0;
	m_length = -1.0;

	m_audioStreamNames.clear();
	m_audioStreamFormats.clear();

	m_state = Decoder::Closed;
}

void
Decoder::setPosition(double position)
{
	if(m_state <= Decoder::Closed || m_state == Decoder::Ready)
		return;

	if(position > m_length && m_length > 0)
		setLength(position);

	if(m_position != position) {
		if(m_position <= 0 || DEFAULT_MIN_POSITION_DELTA <= 0.0) {
			m_position = position;
			emit positionChanged(position);
		} else {
			double positionDelta = m_position - position;
			if(positionDelta >= DEFAULT_MIN_POSITION_DELTA || -positionDelta >= DEFAULT_MIN_POSITION_DELTA) {
				m_position = position;
				emit positionChanged(position);
			}
		}
	}
}

void
Decoder::setLength(double length)
{
	if(m_state <= Decoder::Closed)
		return;

	if(length >= 0 && m_length != length) {
		m_length = length;
		emit lengthChanged(length);
	}
}

void
Decoder::setState(Decoder::State newState)
{
	if(m_state == Decoder::Opening || m_openFileTimer->isActive()) {
		if(newState == Decoder::Ready) {
			m_openFileTimer->stop();

			m_state = Decoder::Ready;

			emit fileOpened(m_filePath);

			// we emit this signals in case their values were already set
			emit lengthChanged(m_length);
			emit audioStreamsChanged(m_audioStreamNames, m_audioStreamFormats);
		}
	} else if(m_state > Decoder::Opening) {
		if(m_state != newState) {
			m_state = newState;
			switch(m_state) {
			case Decoder::Decoding:
				emit decoding();
				break;
			case Decoder::Ready:
				emit stopped();
				break;
			default:
				break;
			}
		}
	}
}

void
Decoder::setErrorState(const QString &errorMessage)
{
	if(!isInitialized())
		return;

	if(m_state <= Decoder::Opening) {
		m_openFileTimer->stop();
		QString filePath(m_filePath);
		resetState();
		emit fileOpenError(filePath);
	} else {
		activeBackend()->stop();
		m_state = Decoder::Ready;
		emit decodingError(errorMessage);
		emit stopped();
	}
}

void
Decoder::appendAudioStream(const QString &name, const WaveFormat &format)
{
	insertAudioStream(m_audioStreamNames.size(), name, format);
}

void
Decoder::insertAudioStream(int index, const QString &name, const WaveFormat &format)
{
	if(m_state <= Decoder::Closed || index < 0 || index > m_audioStreamNames.count())
		return;

	m_audioStreamNames.insert(index, name);
	m_audioStreamFormats.insert(index, format);

	emit audioStreamsChanged(m_audioStreamNames, m_audioStreamFormats);
}

bool
Decoder::openFile(const QString &filePath)
{
	if(m_state != Decoder::Closed)
		return false;

	QFileInfo fileInfo(filePath);
	if(!fileInfo.exists() || !fileInfo.isFile() || !fileInfo.isReadable()) {
		emit fileOpenError(filePath);   // operation will never succed
		return true;
	}

	m_filePath = filePath;
	m_state = Decoder::Opening;

	m_openFileTimer->start(6000);

	if(!activeBackend()->openFile(fileInfo.absoluteFilePath())) {
		resetState();
		emit fileOpenError(filePath);
		return true;
	}

	return true;
}

void
Decoder::onOpenFileTimeout()
{
	QString filePath(m_filePath);

	activeBackend()->stop();
	activeBackend()->closeFile();

	resetState();

	emit fileOpenError(filePath);
}

bool
Decoder::closeFile()
{
	if(m_state <= Decoder::Closed)
		return false;

	bool stop = m_state != Decoder::Ready;
	if(stop)
		activeBackend()->stop(); // we can safely ignore the stop() return value here as we're about to close the file

	activeBackend()->closeFile();

	resetState();

	if(stop)
		emit stopped();

	emit fileClosed();

	return true;
}

bool
Decoder::decode(int audioStream, const QString &outputPath, const WaveFormat &outputFormat)
{
	if(m_state <= Decoder::Opening || m_state == Decoder::Decoding || audioStream < 0 || audioStream >= m_audioStreamNames.size())
		return false;

	if(!activeBackend()->decode(audioStream, outputPath, outputFormat)) {
		resetState();
		emit decodingError();
	}

	return true;
}

bool
Decoder::stop()
{
	if(m_state <= Decoder::Opening || m_state == Decoder::Ready)
		return false;

	if(!activeBackend()->stop()) {
		resetState();
		emit decodingError();
		return true;
	}

	return true;
}

#include "decoder.moc"
