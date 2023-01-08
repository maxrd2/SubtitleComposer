/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "config.h"

#include "formatmanager.h"
#include "inputformat.h"
#include "outputformat.h"
#include "gui/treeview/lineswidget.h"
#include "application.h"
#include "helpers/fileloadhelper.h"
#include "helpers/filesavehelper.h"
#include "dialogs/encodingdetectdialog.h"
#include "scconfig.h"

#include "microdvd/microdvdinputformat.h"
#include "microdvd/microdvdoutputformat.h"
#include "mplayer/mplayerinputformat.h"
#include "mplayer/mplayeroutputformat.h"
#include "mplayer2/mplayer2inputformat.h"
#include "mplayer2/mplayer2outputformat.h"
#include "subrip/subripinputformat.h"
#include "subrip/subripoutputformat.h"
#include "substationalpha/substationalphainputformat.h"
#include "substationalpha/substationalphaoutputformat.h"
#include "subviewer1/subviewer1inputformat.h"
#include "subviewer1/subviewer1outputformat.h"
#include "subviewer2/subviewer2inputformat.h"
#include "subviewer2/subviewer2outputformat.h"
#include "tmplayer/tmplayerinputformat.h"
#include "tmplayer/tmplayeroutputformat.h"
#include "vobsub/vobsubinputformat.h"
#include "webvtt/webvttinputformat.h"
#include "webvtt/webvttoutputformat.h"
#include "youtubecaptions/youtubecaptionsinputformat.h"
#include "youtubecaptions/youtubecaptionsoutputformat.h"

#include <QFile>
#include <QFileDevice>
#include <QFileInfo>
#include <QTextCodec>

#include <QUrl>

#ifdef HAVE_ICU
#	include <unicode/ucsdet.h>
#endif

using namespace SubtitleComposer;

FormatManager &
FormatManager::instance()
{
	static FormatManager instance;
	return instance;
}

#define INPUT_FORMAT(fmt) { InputFormat *f = new fmt##InputFormat(); m_inputFormats[f->name()] = f; }
#define OUTPUT_FORMAT(fmt) { OutputFormat *f = new fmt##OutputFormat(); m_outputFormats[f->name()] = f; }
#define IN_OUT_FORMAT(fmt) INPUT_FORMAT(fmt) OUTPUT_FORMAT(fmt)

FormatManager::FormatManager()
{
	IN_OUT_FORMAT(WebVTT)
	IN_OUT_FORMAT(SubRip)
	IN_OUT_FORMAT(MicroDVD)
	IN_OUT_FORMAT(MPlayer)
	IN_OUT_FORMAT(MPlayer2)
	IN_OUT_FORMAT(SubStationAlpha)
	IN_OUT_FORMAT(AdvancedSubStationAlpha)
	IN_OUT_FORMAT(SubViewer1)
	IN_OUT_FORMAT(SubViewer2)
	IN_OUT_FORMAT(TMPlayer)
	IN_OUT_FORMAT(TMPlayerPlus)
	IN_OUT_FORMAT(YouTubeCaptions)
	INPUT_FORMAT(VobSub)
}

FormatManager::~FormatManager()
{
	for(QMap<QString, InputFormat *>::ConstIterator it = m_inputFormats.constBegin(), end = m_inputFormats.constEnd(); it != end; ++it)
		delete it.value();

	for(QMap<QString, OutputFormat *>::ConstIterator it = m_outputFormats.constBegin(), end = m_outputFormats.constEnd(); it != end; ++it)
		delete it.value();
}

bool
FormatManager::hasInput(const QString &name) const
{
	return m_inputFormats.contains(name);
}

const InputFormat *
FormatManager::input(const QString &name) const
{
	QMap<QString, InputFormat *>::const_iterator it = m_inputFormats.find(name);
	return it != m_inputFormats.end() ? it.value() : nullptr;
}

QStringList
FormatManager::inputNames() const
{
	return m_inputFormats.keys();
}

inline static QTextCodec *
detectEncoding(const QByteArray &byteData)
{
	EncodingDetectDialog dlg(byteData);

#ifdef HAVE_ICU
	UErrorCode status = U_ZERO_ERROR;
	UCharsetDetector *csd = ucsdet_open(&status);
	ucsdet_setText(csd, byteData.data(), byteData.length(), &status);
	int32_t matchesFound = 0;
	const UCharsetMatch **ucms = ucsdet_detectAll(csd, &matchesFound, &status);
	for(int index = 0; index < matchesFound; ++index) {
		int confidence = ucsdet_getConfidence(ucms[index], &status);
		QTextCodec *codec = QTextCodec::codecForName(ucsdet_getName(ucms[index], &status));
		if(codec) {
			if(confidence == 100)
				return codec;
			dlg.addEncoding(codec->name(), confidence);
		}
	}
	ucsdet_close(csd);
#else
	KEncodingProber prober(KEncodingProber::Universal);
	prober.feed(byteData);
	QTextCodec *codec = QTextCodec::codecForName(prober.encoding());
	if(codec) {
		if(prober.confidence() >= 1.)
			return codec;
		dlg.addEncoding(codec->name(), prober.confidence() * 100.);
	}
#endif

	if(dlg.exec() == QDialog::Accepted)
		return QTextCodec::codecForName(dlg.selectedEncoding().toUtf8());

	return nullptr;
}


FormatManager::Status
FormatManager::readBinary(Subtitle &subtitle, const QUrl &url, bool primary,
						  QTextCodec **codec, QString *formatName) const
{
	foreach(InputFormat *format, m_inputFormats) {
		QExplicitlySharedDataPointer<Subtitle> newSubtitle(new Subtitle());
		Status res = format->readBinary(*newSubtitle, url);
		if(res == ERROR)
			continue;
		if(res == SUCCESS) {
			if(formatName)
				*formatName = format->name();
			*codec = QTextCodec::codecForName(SCConfig::defaultSubtitlesEncoding().toUtf8());
			if(primary)
				subtitle.setPrimaryData(*newSubtitle, true);
			else
				subtitle.setSecondaryData(*newSubtitle, true);
		}
		return res;
	}
	return ERROR;
}

FormatManager::Status
FormatManager::readText(Subtitle &subtitle, const QUrl &url, bool primary,
						QTextCodec **codec, QString *formatName) const
{
	FileLoadHelper fileLoadHelper(url);
	if(!fileLoadHelper.open())
		return ERROR;
	// WARNING: only 1MB of text subtitle is being read here
	QByteArray byteData = fileLoadHelper.file()->read(1024 * 1024);
	fileLoadHelper.close();

	QString stringData;
	if(!codec) {
		// don't care about text nor text encoding
		stringData = QString::fromLatin1(byteData);
	} else {
		if(!*codec) {
			QTextCodec *c = detectEncoding(byteData);
			if(!c)
				return CANCEL;
			*codec = c;
		}
		if(*codec)
			stringData = (*codec)->toUnicode(byteData);
	}

	stringData.replace(QLatin1String("\r\n"), QLatin1String("\n"));
	stringData.replace('\r', '\n');

	const QString extension = QFileInfo(url.path()).suffix();

	// attempt to parse subtitles based on extension information first
	for(QMap<QString, InputFormat *>::ConstIterator it = m_inputFormats.begin(), end = m_inputFormats.end(); it != end; ++it) {
		if(it.value()->knowsExtension(extension)) {
			if(it.value()->readSubtitle(subtitle, primary, stringData)) {
				if(formatName)
					*formatName = it.value()->name();
				return SUCCESS;
			}
		}
	}

	// attempt to parse subtitles based on content
	for(QMap<QString, InputFormat *>::ConstIterator it = m_inputFormats.begin(), end = m_inputFormats.end(); it != end; ++it) {
		if(!it.value()->knowsExtension(extension)) {
			if(it.value()->readSubtitle(subtitle, primary, stringData)) {
				if(formatName)
					*formatName = it.value()->name();
				return SUCCESS;
			}
		}
	}

	return ERROR;
}

FormatManager::Status
FormatManager::readSubtitle(Subtitle &subtitle, bool primary, const QUrl &url,
							QTextCodec **codec, QString *formatName) const
{
	Status res = readBinary(subtitle, url, primary, codec, formatName);
	if(res != ERROR) // when SUCCESS or CANCEL no need to try text formats
		return res;

	return readText(subtitle, url, primary, codec, formatName);
}

bool
FormatManager::hasOutput(const QString &name) const
{
	return m_outputFormats.contains(name);
}

const OutputFormat *
FormatManager::output(const QString &name) const
{
	return m_outputFormats.contains(name) ? m_outputFormats[name] : nullptr;
}

const OutputFormat *
FormatManager::defaultOutput() const
{
	return output(QStringLiteral("SubRip"));
}

QStringList
FormatManager::outputNames() const
{
	return m_outputFormats.keys();
}

bool
FormatManager::writeSubtitle(const Subtitle &subtitle, bool primary, const QUrl &url,
							 QTextCodec *codec, const QString &formatName, bool overwrite) const
{
	const OutputFormat *format = output(formatName);
	if(format == nullptr) {
		QString extension = QFileInfo(url.path()).suffix();
		// attempt find format based on extension information
		for(QMap<QString, OutputFormat *>::ConstIterator it = m_outputFormats.begin(), end = m_outputFormats.end(); it != end; ++it)
			if(it.value()->knowsExtension(extension)) {
				format = *it;
				break;
			}
	}

	if(format == nullptr)
		return false;

	FileSaveHelper fileSaveHelper(url, overwrite);
	if(!fileSaveHelper.open())
		return false;

	QFileDevice *file = fileSaveHelper.file();
	QString data = format->writeSubtitle(subtitle, primary);
	if(codec->name().startsWith("UTF-") || codec->name().contains("UCS-"))
		data.prepend(QChar::ByteOrderMark);
	switch(SCConfig::textLineBreak()) {
	case 1: // CRLF
		file->write(codec->fromUnicode(data.replace(QChar::LineFeed, QLatin1String("\r\n"))));
		break;
	case 2: // CR
		file->write(codec->fromUnicode(data.replace(QChar::LineFeed, QChar::CarriageReturn)));
		break;
	default: // LF
		file->write(codec->fromUnicode(data));
		break;
	}

	return fileSaveHelper.close();
}
