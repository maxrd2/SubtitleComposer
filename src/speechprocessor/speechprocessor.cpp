/*
 * Copyright (C) 2010-2017 Mladen Milinkovic <max@smoothware.net>
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

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "speechprocessor.h"
#include "speechplugin.h"
#include "application.h"
#include "lineswidget.h"

#include "scconfig.h"

#include <QLabel>
#include <QProgressBar>
#include <QBoxLayout>
#include <QThread>
#include <QToolButton>

#include <QPluginLoader>
#include <QDir>
#include <QFile>

#include <QDebug>

#include <KLocalizedString>

using namespace SubtitleComposer;

SpeechProcessor::SpeechProcessor(QWidget *parent)
	: QObject(parent),
	  m_mediaFile(QString()),
	  m_streamIndex(-1),
	  m_stream(new StreamProcessor(this)),
	  m_subtitle(nullptr),
	  m_progressWidget(new QWidget(parent)),
	  m_plugin(nullptr)
{
	// Progress Bar
	m_progressWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	m_progressWidget->hide();

	QLabel *label = new QLabel(i18n("Recognizing speech"), m_progressWidget);

	m_progressBar = new QProgressBar(m_progressWidget);
	m_progressBar->setMinimumWidth(300);
	m_progressBar->setTextVisible(true);

	QToolButton *btnAbort = new QToolButton(m_progressWidget);
	btnAbort->setIcon(QIcon::fromTheme(QStringLiteral("process-stop")));
	btnAbort->setToolTip(i18n("Abort speech recognition"));

	QBoxLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight, m_progressWidget);
	layout->setContentsMargins(1, 0, 1, 0);
	layout->setSpacing(1);
	layout->addWidget(label);
	layout->addWidget(m_progressBar);
	layout->addWidget(btnAbort);

	connect(btnAbort, &QToolButton::clicked, m_stream, &StreamProcessor::close);
	connect(m_stream, &StreamProcessor::streamProgress, this, &SpeechProcessor::onStreamProgress);
	connect(m_stream, &StreamProcessor::streamError, this, &SpeechProcessor::onStreamError);
	connect(m_stream, &StreamProcessor::streamFinished, this, &SpeechProcessor::onStreamFinished);
	// Using Qt::DirectConnection here makes SpeechProcessor::onStreamData() to execute in SpeechProcessor's thread
	connect(m_stream, &StreamProcessor::audioDataAvailable, this, &SpeechProcessor::onStreamData, Qt::DirectConnection);

	const QString buildPluginPath(qApp->applicationDirPath() + QStringLiteral("/speechplugins"));
	if(QDir(buildPluginPath).exists()) {
		// if application is launched from build directory it will load plugins from build directory
		pluginLoad(buildPluginPath + QStringLiteral("/pocketsphinx/pocketsphinxasr.so"));
	} else {
		QDir pluginsDir(QStringLiteral(SCPLUGIN_PATH));
		foreach(const QString pluginFile, pluginsDir.entryList(QDir::Files, QDir::Name)) {
			if(QLibrary::isLibrary(pluginFile))
				pluginLoad(pluginsDir.filePath(pluginFile));
		}
	}
}

SpeechProcessor::~SpeechProcessor()
{
	m_progressWidget = NULL;
	clearAudioStream();
}

SpeechPlugin *
SpeechProcessor::pluginLoad(const QString &pluginPath)
{
	const QString realPath = QDir(pluginPath).canonicalPath();
	if(realPath.isEmpty())
		return NULL;

	QPluginLoader loader(realPath);
	QObject *plugin = loader.instance();
	if(!plugin)
		return NULL;

	SpeechPlugin *asrPlugin = qobject_cast<SpeechPlugin *>(plugin);
	if(!asrPlugin)
		return NULL;

	qInfo() << "Loaded SpeechProcessor plugin" << asrPlugin->name() << "from" << realPath;

	asrPlugin->setSCConfig(SCConfig::self());

	pluginAdd(asrPlugin);

	return asrPlugin;
}

void
SpeechProcessor::pluginAdd(SpeechPlugin *plugin)
{
	plugin->setParent(this); // SpeechProcessor will delete *plugin

	if(m_plugins.contains(plugin->name())) {
		qCritical() << "Attempted to insert duplicate SpeechProcessor plugin" << plugin->name();
		return;
	}

	m_plugins[plugin->name()] = plugin;
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

	if(!m_plugins.isEmpty()) {
		m_plugin = m_plugins.first();
	} else {
		onStreamError(1, i18n("No speech recognition plugins available"), QString());
		return;
	}

	if(!m_plugin->init()) {
		onStreamError(1, i18n("Initialization of speech recognition plugin failed"), QString());
		return;
	}

	connect(m_plugin, &SpeechPlugin::textRecognized, this, &SpeechProcessor::onTextRecognized);
	connect(m_plugin, &SpeechPlugin::error, [this](int code, const QString &message) { onStreamError(code, message, QString()); });

	m_mediaFile = mediaFile;
	m_streamIndex = audioStream;

	m_audioDuration = 0;

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

	if(m_plugin) {
		m_plugin->disconnect();
		m_plugin->cleanup();
		m_plugin = nullptr;
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
	if(m_plugin)
		m_plugin->processComplete();
	clearAudioStream();
}

void
SpeechProcessor::onStreamData(const void *buffer, const qint32 size, const WaveFormat */*waveFormat*/, const qint64 /*msecStart*/, const qint64 /*msecDuration*/)
{
	// make sure SpeechProcessor::onStreamProgress() signal was processed since we're in different thread
	while(!m_audioDuration)
		QThread::yieldCurrentThread();

	Q_ASSERT(size % sizeof(qint16) == 0);

	if(m_plugin)
		m_plugin->processSamples(reinterpret_cast<const qint16 *>(buffer), size / sizeof(qint16));
}

void
SpeechProcessor::onStreamError(int code, const QString &message, const QString &debug)
{
	emit onError(i18n("Speech Recognition failed: %2\nCode %1: %3")
				 .arg(code)
				 .arg(message)
				 .arg(debug));

	clearAudioStream();
}

void
SpeechProcessor::onTextRecognized(const QString &text, const double milliShow, const double milliHide)
{
	if(!m_subtitle)
		return;

	LinesWidgetScrollToModelDetacher detacher(*app()->linesWidget());
	m_subtitle->insertLine(new SubtitleLine(SString(text), milliShow, milliHide));
}

