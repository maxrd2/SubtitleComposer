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

#include "fileloadhelper.h"

#include <KDebug>
#include <KSaveFile>
#include <KTemporaryFile>
#include <KIO/NetAccess>

FileLoadHelper::FileLoadHelper(const KUrl &url) :
	m_url(url),
	m_file(0)
{}

FileLoadHelper::~FileLoadHelper()
{
	if(m_file)
		close();
}

const KUrl &
FileLoadHelper::url()
{
	return m_url;
}

QFile *
FileLoadHelper::file()
{
	return m_file;
}

bool
FileLoadHelper::open()
{
	if(m_file)
		return false;

	if(!KIO::NetAccess::exists(m_url, KIO::NetAccess::SourceSide, 0))
		return false;

	if(m_url.isLocalFile()) {
		m_file = new QFile(m_url.path());
		if(!m_file->open(QIODevice::ReadOnly)) {
			kDebug() << "couldn't open input file" << m_file->fileName();
			delete m_file;
			m_file = 0;
			return false;
		}
	} else {
		QString tmpFile;
		if(!KIO::NetAccess::download(m_url, tmpFile, 0)) {
			kDebug() << "couldn't get input url:" << m_url.prettyUrl();
			kDebug() << KIO::NetAccess::lastErrorString();
			return false;
		}

		m_file = new QFile(tmpFile);
		if(!m_file->open(QIODevice::ReadOnly)) {
			kDebug() << "couldn't open input file" << m_file->fileName();
			delete m_file;
			m_file = 0;
			return false;
		}
	}
	return true;
}

bool
FileLoadHelper::close()
{
	if(!m_file)
		return false;

	QString tmpFilePath = m_file->fileName();

	delete m_file;                          // closes the file
	m_file = 0;

	if(!m_url.isLocalFile())
		KIO::NetAccess::removeTempFile(tmpFilePath);

	return true;
}

bool
FileLoadHelper::exists(const KUrl &url)
{
	return KIO::NetAccess::exists(url, KIO::NetAccess::SourceSide, 0);
}
