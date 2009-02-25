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

#include "formatmanager.h"

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

using namespace SubtitleComposer;


FormatManager& FormatManager::instance()
{
	static FormatManager instance;
	return instance;
}

FormatManager::FormatManager()
{
	InputFormat* inputFormat = new SubRipInputFormat();
	m_inputFormats[inputFormat->name()] = inputFormat;
	inputFormat = new MicroDVDInputFormat();
	m_inputFormats[inputFormat->name()] = inputFormat;
	inputFormat = new MPlayerInputFormat();
	m_inputFormats[inputFormat->name()] = inputFormat;
	inputFormat = new MPlayer2InputFormat();
	m_inputFormats[inputFormat->name()] = inputFormat;
	inputFormat = new SubStationAlphaInputFormat();
	m_inputFormats[inputFormat->name()] = inputFormat;
	inputFormat = new AdvancedSubStationAlphaInputFormat();
	m_inputFormats[inputFormat->name()] = inputFormat;
	inputFormat = new SubViewer1InputFormat();
	m_inputFormats[inputFormat->name()] = inputFormat;
	inputFormat = new SubViewer2InputFormat();
	m_inputFormats[inputFormat->name()] = inputFormat;
	inputFormat = new TMPlayerInputFormat();
	m_inputFormats[inputFormat->name()] = inputFormat;
	inputFormat = new TMPlayerPlusInputFormat();
	m_inputFormats[inputFormat->name()] = inputFormat;

	OutputFormat* outputFormat = new SubRipOutputFormat();
	m_outputFormats[outputFormat->name()] = outputFormat;
	outputFormat = new MicroDVDOutputFormat();
	m_outputFormats[outputFormat->name()] = outputFormat;
	outputFormat = new MPlayerOutputFormat();
	m_outputFormats[outputFormat->name()] = outputFormat;
	outputFormat = new MPlayer2OutputFormat();
	m_outputFormats[outputFormat->name()] = outputFormat;
	outputFormat = new SubStationAlphaOutputFormat();
	m_outputFormats[outputFormat->name()] = outputFormat;
	outputFormat = new AdvancedSubStationAlphaOutputFormat();
	m_outputFormats[outputFormat->name()] = outputFormat;
	outputFormat = new SubViewer1OutputFormat();
	m_outputFormats[outputFormat->name()] = outputFormat;
	outputFormat = new SubViewer2OutputFormat();
	m_outputFormats[outputFormat->name()] = outputFormat;
	outputFormat = new TMPlayerOutputFormat();
	m_outputFormats[outputFormat->name()] = outputFormat;
	outputFormat = new TMPlayerPlusOutputFormat();
	m_outputFormats[outputFormat->name()] = outputFormat;
}

FormatManager::~FormatManager()
{
	for ( QMap<QString,InputFormat*>::ConstIterator it = m_inputFormats.begin(), end = m_inputFormats.end(); it != end; ++it )
		delete it.value();

	for ( QMap<QString,OutputFormat*>::ConstIterator it = m_outputFormats.begin(), end = m_outputFormats.end(); it != end; ++it )
		delete it.value();
}

bool FormatManager::hasInput( const QString& name )
{
	return m_inputFormats.contains( name );
}

const InputFormat* FormatManager::input( const QString& name )
{
	return m_inputFormats.contains( name ) ? m_inputFormats[name] : 0;
}

QStringList FormatManager::inputNames()
{
	return m_inputFormats.keys();
}

bool FormatManager::readSubtitle( Subtitle& subtitle, bool primary, const KUrl& url, QTextCodec* codec, Format::NewLine* newLine, QString* formatName )
{
	QString extension = QFileInfo( url.path() ).suffix();

	// attempt to parse subtitles based on extension information
	for ( QMap<QString,InputFormat*>::ConstIterator it = m_inputFormats.begin(), end = m_inputFormats.end(); it != end; ++it )
	{
		if ( it.value()->knowsExtension( extension ) )
		{
			if ( it.value()->readSubtitle( subtitle, newLine, primary, url, codec, true ) )
			{
				if ( formatName )
					*formatName = it.value()->name();
				return true;
			}
		}
	}

	// that didn't worked, attempt to parse subtitles based on content
	for ( QMap<QString,InputFormat*>::ConstIterator it = m_inputFormats.begin(), end = m_inputFormats.end(); it != end; ++it )
	{
		if ( ! it.value()->knowsExtension( extension ) )
		{
			if ( it.value()->readSubtitle( subtitle, newLine, primary, url, codec, true ) )
			{
				if ( formatName )
					*formatName = it.value()->name();
				return true;
			}
		}
	}

	return false;
}

bool FormatManager::hasOutput( const QString& name )
{
	return m_outputFormats.contains( name );
}

const OutputFormat* FormatManager::output( const QString& name )
{
	return m_outputFormats.contains( name ) ? m_outputFormats[name] : 0;
}

QStringList FormatManager::outputNames()
{
	return m_outputFormats.keys();
}

bool FormatManager::writeSubtitle( const Subtitle& subtitle, bool primary, const KUrl& url, QTextCodec* codec, Format::NewLine newLine, const QString& formatName, bool overwrite )
{
	const OutputFormat* format = output( formatName );
	if ( format == 0 )
	{
		QString extension = QFileInfo( url.path() ).suffix();
		// attempt find format based on extension information
		for ( QMap<QString,OutputFormat*>::ConstIterator it = m_outputFormats.begin(), end = m_outputFormats.end(); it != end; ++it )
			if ( it.value()->knowsExtension( extension ) )
			{
				format = *it;
				break;
			}
	}

	if ( format == 0 )
		return false;

	return format->writeSubtitle( subtitle, newLine, primary, url, codec, overwrite );
}
