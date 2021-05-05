/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2019 Mladen Milinkovic <max@smoothware.net>
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
#include "youtubecaptions/youtubecaptionsinputformat.h"
#include "youtubecaptions/youtubecaptionsoutputformat.h"
#include "vobsub/vobsubinputformat.h"

#include <QFile>
#include <QFileInfo>
#include <QTextCodec>
#include <QTextStream>

#include <KCharsets>
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
		bool encodingFound;
		QTextCodec *codec = KCharsets::charsets()->codecForName(ucsdet_getName(ucms[index], &status), encodingFound);
		if(encodingFound) {
			if(confidence == 100)
				return codec;
			dlg.addEncoding(codec->name(), confidence);
		}
	}
	ucsdet_close(csd);
#else
	KEncodingProber prober(KEncodingProber::Universal);
	prober.feed(byteData);
	bool encodingFound;
	QTextCodec *codec = KCharsets::charsets()->codecForName(prober.encoding(), encodingFound);
	if(encodingFound) {
		if(prober.confidence() >= 1.)
			return codec;
		dlg.addEncoding(codec->name(), prober.confidence() * 100.);
	}
#endif

	if(dlg.exec() == QDialog::Accepted) {
		bool encodingFound;
		return KCharsets::charsets()->codecForName(dlg.selectedEncoding(), encodingFound);
	}

	return nullptr;
}


FormatManager::Status
FormatManager::readBinary(Subtitle &subtitle, const QUrl &url, bool primary,
						  QTextCodec **codec, QString *formatName) const
{
	foreach(InputFormat *format, m_inputFormats) {
		Subtitle newSubtitle;
		Status res = format->readBinary(newSubtitle, url);
		if(res == ERROR)
			continue;
		if(res == SUCCESS) {
			if(formatName)
				*formatName = format->name();
			*codec = KCharsets::charsets()->codecForName(SCConfig::defaultSubtitlesEncoding());
			if(primary)
				subtitle.setPrimaryData(newSubtitle, true);
			else
				subtitle.setSecondaryData(newSubtitle, true);
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
		if(*codec) {
			QTextStream textStream(byteData);
			textStream.setCodec(*codec);
			textStream.setAutoDetectUnicode(false);
			stringData = textStream.readAll();
		}
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

	QTextStream stream(fileSaveHelper.file());
	stream.setCodec(codec);
	stream.setGenerateByteOrderMark(true);

	QString data = format->writeSubtitle(subtitle, primary);
	switch(SCConfig::textLineBreak()) {
	case 1: // CRLF
		stream << data.replace(QChar::LineFeed, QLatin1String("\r\n"));
		break;
	case 2: // CR
		stream << data.replace(QChar::LineFeed, QChar::CarriageReturn);
		break;
	default: // LF
		stream << data;
		break;
	}

	return fileSaveHelper.close();
}
