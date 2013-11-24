#ifndef XINEDECODERBACKEND_H
#define XINEDECODERBACKEND_H

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "xineconfig.h"
#include "../decoderbackend.h"

#include <QtCore/QString>

#define XINE_ENABLE_EXPERIMENTAL_FEATURES
#include <xine.h>

class QEvent;

namespace SubtitleComposer {
class DecodingThread;

class XineDecoderBackend : public DecoderBackend
{
	Q_OBJECT

public:
	XineDecoderBackend(Decoder *decoder);
	virtual ~XineDecoderBackend();

	const XineConfig * config() { return static_cast<const XineConfig *const>(DecoderBackend::config()); }

	virtual AppConfigGroupWidget * newAppConfigGroupWidget(QWidget *parent);

protected:
	virtual QWidget * initialize(QWidget *videoWidgetParent);
	virtual void finalize();

	virtual bool openFile(const QString &filePath);
	virtual void closeFile();

	virtual bool decode(int audioStream, const QString &outputPath, const WaveFormat &outputFormat);
	virtual bool stop();

	virtual void customEvent(QEvent *event);

private:
	bool initializeXine();
	void finalizeXine();

	unsigned long readUncompressedData(void *buffer, unsigned long bufferSize);
	bool readNextFrame(bool first);

private:
	xine_t *m_xineEngine;
	xine_audio_port_t *m_audioPort;
	xine_stream_t *m_xineStream;
	xine_event_queue_t *m_eventQueue;
	xine_audio_frame_t *m_frame;

	bool m_isValidFrame;
	unsigned long m_framePos;
	unsigned long m_frameSize;

	DecodingThread *m_decodingThread;
	friend class DecodingThread;
};
}

#endif
