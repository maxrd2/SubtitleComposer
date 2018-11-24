#ifndef SPEECHPROCESSOR_H
#define SPEECHPROCESSOR_H
/*
 * Copyright (C) 2010-2018 Mladen Milinkovic <max@smoothware.net>
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

#include "core/time.h"
#include "core/subtitle.h"
#include "videoplayer/waveformat.h"
#include "streamprocessor/streamprocessor.h"

#include <QList>

QT_FORWARD_DECLARE_CLASS(QWidget)
QT_FORWARD_DECLARE_CLASS(QProgressBar)

namespace SubtitleComposer {
class SpeechPlugin;
class SpeechProcessor : public QObject
{
	Q_OBJECT

public:
	explicit SpeechProcessor(QWidget *parent = NULL);
	virtual ~SpeechProcessor();

	QWidget * progressWidget();

public slots:
	void setSubtitle(Subtitle *subtitle = NULL);
	void setAudioStream(const QString &mediaFile, int audioStream);
	void clearAudioStream();

signals:
	void onError(const QString &message);

private slots:
	void onStreamProgress(quint64 msecPos, quint64 msecLength);
	void onStreamError(int code, const QString &message, const QString &debug);
	void onStreamFinished();
	void onStreamData(const void *buffer, const qint32 size, const WaveFormat *waveFormat, const qint64 msecStart, const qint64 msecDuration);
	void onTextRecognized(const QString &text, const double milliShow, const double milliHide);

private:
	SpeechPlugin * pluginLoad(const QString &pluginPath);
	void pluginAdd(SpeechPlugin *plugin);

private:
	QString m_mediaFile;
	int m_streamIndex;

	StreamProcessor *m_stream;
	Subtitle *m_subtitle;

	quint32 m_audioDuration;

	QWidget *m_progressWidget;
	QProgressBar *m_progressBar;

	SpeechPlugin *m_plugin;
	QMap<QString, SpeechPlugin *> m_plugins;
};
}

#endif // SPEECHPROCESSOR_H
