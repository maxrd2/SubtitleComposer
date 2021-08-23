/*
 * SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SPEECHPROCESSOR_H
#define SPEECHPROCESSOR_H

#include "core/time.h"
#include "core/subtitle.h"
#include "videoplayer/waveformat.h"
#include "streamprocessor/streamprocessor.h"

#include <QExplicitlySharedDataPointer>
#include <QList>

QT_FORWARD_DECLARE_CLASS(QWidget)
QT_FORWARD_DECLARE_CLASS(QProgressBar)

namespace SubtitleComposer {
class SpeechPlugin;
class SpeechProcessor : public QObject
{
	Q_OBJECT

	template <class C, class T> friend class PluginHelper;

public:
	explicit SpeechProcessor(QWidget *parent = NULL);
	virtual ~SpeechProcessor();

	QWidget * progressWidget();

	inline const QMap<QString, SpeechPlugin *> & plugins() const { return m_plugins; }

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
	QString m_mediaFile;
	int m_streamIndex;

	StreamProcessor *m_stream;
	QExplicitlySharedDataPointer<Subtitle> m_subtitle;

	quint32 m_audioDuration;

	QWidget *m_progressWidget;
	QProgressBar *m_progressBar;

	SpeechPlugin *m_plugin;
	QMap<QString, SpeechPlugin *> m_plugins;
};
}

#endif // SPEECHPROCESSOR_H
