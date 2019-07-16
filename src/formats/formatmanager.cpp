/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
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

#include "formatmanager.h"
#include "inputformat.h"
#include "outputformat.h"
#include "lineswidget.h"
#include "application.h"
#include "helpers/fileloadhelper.h"
#include "helpers/filesavehelper.h"

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

#ifdef HAVE_CONFIG_H
# include <config.h>
# ifdef HAVE_ICU
#  include <unicode/ucsdet.h>
# endif
#endif

using namespace SubtitleComposer;

FormatManager &
FormatManager::instance()
{
	static FormatManager instance;
	return instance;
}

FormatManager::FormatManager()
{
	/*foreach( const QStringList &encodingsForScript, KCharsets::charsets()->encodingsByScript() )
	   {
	   KEncodingDetector::AutoDetectScript scri = KEncodingDetector::scriptForName( encodingsForScript.at( 0 ) );
	   if ( KEncodingDetector::hasAutoDetectionForScript( scri ) )
	   qDebug() << encodingsForScript.at( 0 ) << "[autodetect available]";
	   else
	   qDebug() << encodingsForScript.at( 0 );
	   for ( int i=1; i < encodingsForScript.size(); ++i )
	   qDebug() << "-" << encodingsForScript.at( i );
	   } */

	InputFormat *inputFormats[] = {
		new SubRipInputFormat(),
		new MicroDVDInputFormat(),
		new MPlayerInputFormat(),
		new MPlayer2InputFormat(),
		new SubStationAlphaInputFormat(),
		new AdvancedSubStationAlphaInputFormat(),
		new SubViewer1InputFormat(),
		new SubViewer2InputFormat(),
		new TMPlayerInputFormat(),
		new TMPlayerPlusInputFormat(),
		new YouTubeCaptionsInputFormat(),
		new VobSubInputFormat(),
	};

	for(int index = 0, count = sizeof(inputFormats) / sizeof(*(inputFormats)); index < count; ++index) {
		m_inputFormats[inputFormats[index]->name()] = inputFormats[index];
	}

	OutputFormat *outputFormats[] = {
		new SubRipOutputFormat(),
		new MicroDVDOutputFormat(),
		new MPlayerOutputFormat(),
		new MPlayer2OutputFormat(),
		new SubStationAlphaOutputFormat(),
		new AdvancedSubStationAlphaOutputFormat(),
		new SubViewer1OutputFormat(),
		new SubViewer2OutputFormat(),
		new TMPlayerOutputFormat(),
		new TMPlayerPlusOutputFormat(),
		new YouTubeCaptionsOutputFormat(),
	};

	for(int index = 0, count = sizeof(outputFormats) / sizeof(*(outputFormats)); index < count; ++index) {
		m_outputFormats[outputFormats[index]->name()] = outputFormats[index];
	}
}

FormatManager::~FormatManager()
{
	for(QMap<QString, InputFormat *>::ConstIterator it = m_inputFormats.begin(), end = m_inputFormats.end(); it != end; ++it)
		delete it.value();

	for(QMap<QString, OutputFormat *>::ConstIterator it = m_outputFormats.begin(), end = m_outputFormats.end(); it != end; ++it)
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

bool
FormatManager::readSubtitle(Subtitle &subtitle, bool primary, const QUrl &url,
							QTextCodec **codec, QString *formatName) const
{
	// attempt to load binary subtitle
	Subtitle newSubtitle;
	foreach(InputFormat *format, m_inputFormats) {
		if(format->readBinary(newSubtitle, url)) {
			if(formatName)
				*formatName = format->name();
			if(primary)
				subtitle.setPrimaryData(newSubtitle, true);
			else
				subtitle.setSecondaryData(newSubtitle, true);
			return true;
		}
	}

	// attempt to load text subtitle
	FileLoadHelper fileLoadHelper(url);
	if(!fileLoadHelper.open())
		return false;
	QByteArray byteData = fileLoadHelper.file()->read(1024 * 1024);
	fileLoadHelper.close();

	QString stringData;

#ifdef HAVE_ICU
	if(!*codec) {
		UErrorCode status = U_ZERO_ERROR;
		UCharsetDetector *csd = ucsdet_open(&status);
		ucsdet_setText(csd, byteData.data(), byteData.length(), &status);
		int32_t matchesFound = 0;
		const UCharsetMatch **ucms = ucsdet_detectAll(csd, &matchesFound, &status);
		for(int index = 0; index < matchesFound; ++index) {
			int32_t confidence = ucsdet_getConfidence(ucms[index], &status);
			const char *name = ucsdet_getName(ucms[index], &status);
			qDebug() << "encoding" << name << "confidence" << confidence;
			bool encodingFound;
			*codec = KCharsets::charsets()->codecForName(name, encodingFound);
			if(encodingFound)
				break;
			else
				*codec = 0;
		}
		ucsdet_close(csd);
	}
#endif
	if(!*codec) {
		KEncodingProber prober(KEncodingProber::Universal);
		prober.feed(byteData);
		bool encodingFound;
		*codec = KCharsets::charsets()->codecForName(prober.encoding(), encodingFound);
	}
	if(*codec) {
		QTextStream textStream(byteData);
		textStream.setCodec(*codec);
		stringData = textStream.readAll();
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
				return true;
			}
		}
	}

	// that didn't worked, attempt to parse subtitles based on content
	for(QMap<QString, InputFormat *>::ConstIterator it = m_inputFormats.begin(), end = m_inputFormats.end(); it != end; ++it) {
		if(!it.value()->knowsExtension(extension)) {
			if(it.value()->readSubtitle(subtitle, primary, stringData)) {
				if(formatName)
					*formatName = it.value()->name();
				return true;
			}
		}
	}

	return false;
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
	if(format == 0) {
		QString extension = QFileInfo(url.path()).suffix();
		// attempt find format based on extension information
		for(QMap<QString, OutputFormat *>::ConstIterator it = m_outputFormats.begin(), end = m_outputFormats.end(); it != end; ++it)
			if(it.value()->knowsExtension(extension)) {
				format = *it;
				break;
			}
	}

	if(format == 0)
		return false;

	FileSaveHelper fileSaveHelper(url, overwrite);
	if(!fileSaveHelper.open())
		return false;

	const QString data = format->writeSubtitle(subtitle, primary);
	QTextStream stream(fileSaveHelper.file());
	stream.setCodec(codec);
	stream.setGenerateByteOrderMark(true);
	stream << data;

	return fileSaveHelper.close();
}
