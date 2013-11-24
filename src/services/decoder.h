#ifndef DECODER_H
#define DECODER_H

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

#include "service.h"
#include "waveformat.h"

#include <QtCore/QString>

class QTimer;

namespace SubtitleComposer {
class DecoderBackend;

class Decoder : public Service
{
	Q_OBJECT

public:
	typedef enum {
		Closed = Service::Initialized,          // same as Initialized
		Opening,
		// Opened, same as >Opening
		Decoding,
		Ready                                   // same as Stopped or Finished
	} State;

	virtual QString dummyBackendName() const { return "Dummy"; }

	inline DecoderBackend * backend(const QString &name) const { return (DecoderBackend *)Service::backend(name); }

	inline DecoderBackend * activeBackend() const { return (DecoderBackend *)Service::activeBackend(); }

	static Decoder * instance();

	inline const QString & filePath() const;

	inline bool isDecoding() const;
	inline double position() const;
	inline double length() const;
	inline bool isStopped() const;

	inline QString audioStreamName(int index) const;
	inline WaveFormat audioStreamFormat(int index) const;

	inline bool hasAudioStreams() const;
	const QStringList & audioStreamNames() const;
	const QList<WaveFormat> & audioStreamFormats() const;

public slots:
// return values of this functions don't imply that the operation was completed OK
// but that it was allowed (a false return value means that nothing was attempted).
	bool openFile(const QString &filePath);
	bool closeFile();

	bool decode(int audioStream, const QString &outputPath, const WaveFormat &outputFormat);
	bool stop();

signals:
	void fileOpenError(const QString &filePath);
	void fileOpened(const QString &filePath);
	void fileClosed();

	void decodingError(const QString &errorMessage = QString());
	void decoding();
	void positionChanged(double seconds);
	void lengthChanged(double seconds);
	void stopped();

	void audioStreamsChanged(const QStringList &names, const QList<WaveFormat> &formats);

protected:
	/**
	 * @brief initializeBackend - attempts to initialize the backend, making it the active backend.
	 * @param backend
	 * @param widgetParent
	 * @return true if backend is the active backend after the call; false if there was already another backend initialized
	 */
	virtual bool initializeBackend(ServiceBackend *backend, QWidget *widgetParent);

	/**
	 * @brief finalizeBackend - finalizes the active backend, leaving no active backend.
	 *  returns??? the previously initialized backend (or 0 if there was none).
	 * @param backend
	 */
	virtual void finalizeBackend(ServiceBackend *backend);

private:
	Decoder();
	virtual ~Decoder();

	void resetState();

// functions used by the backends to inform changes in state:

	void setPosition(double position);              // value in seconds
	void setLength(double length);          // value in seconds

	void setState(Decoder::State state);
	void setErrorState(const QString &errorMessage = QString());

	void appendAudioStream(const QString &name, const WaveFormat &format);
	void insertAudioStream(int index, const QString &name, const WaveFormat &format);

private slots:
	/**
	 * @brief onOpenFileTimeout - called if the Decoder fails to set the state to Playing after opening the file
	 */
	void onOpenFileTimeout();

private:
	QString m_filePath;

	double m_position;
	double m_length;

	QStringList m_audioStreamNames;
	QList<WaveFormat> m_audioStreamFormats;

	QTimer *m_openFileTimer;

	friend class DecoderBackend;
};

const QString &
Decoder::filePath() const
{
	return m_filePath;
}

bool
Decoder::isDecoding() const
{
	return m_state == Decoder::Decoding;
}

double
Decoder::position() const
{
	return m_state <= Decoder::Opening ? -1.0 : (m_state == Decoder::Ready ? 0.0 : m_position);
}

double
Decoder::length() const
{
	return m_state <= Decoder::Opening ? -1.0 : m_length;
}

bool
Decoder::isStopped() const
{
	return m_state == Decoder::Ready;
}

QString
Decoder::audioStreamName(int index) const
{
	return (m_state <= Decoder::Opening || index < 0 || index >= m_audioStreamNames.size()) ? QString() : m_audioStreamNames.at(index);
}

WaveFormat
Decoder::audioStreamFormat(int index) const
{
	return (m_state <= Decoder::Opening || index < 0 || index >= m_audioStreamFormats.size()) ? WaveFormat() : m_audioStreamFormats.at(index);
}

bool
Decoder::hasAudioStreams() const
{
	return m_state <= Decoder::Opening ? false : m_audioStreamNames.count();
}
}
#endif
