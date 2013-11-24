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

#include "filesavehelper.h"

#include <KDebug>
#include <KSaveFile>
#include <KTemporaryFile>
#include <KIO/NetAccess>

FileSaveHelper::FileSaveHelper(const KUrl &url, bool overwrite) :
	m_url(url),
	m_overwrite(overwrite),
	m_file(0)
{}

FileSaveHelper::~FileSaveHelper()
{
	if(m_file)
		close();
}

const KUrl &
FileSaveHelper::url()
{
	return m_url;
}

bool
FileSaveHelper::overwrite()
{
	return m_overwrite;
}

QFile *
FileSaveHelper::file()
{
	return m_file;
}

bool
FileSaveHelper::open()
{
	if(m_file)
		return false;

	if(!m_overwrite && KIO::NetAccess::exists(m_url, KIO::NetAccess::DestinationSide, 0))
		return false;

	if(m_url.isLocalFile()) {
		m_file = new KSaveFile(m_url.path());
		if(!m_file->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
			kDebug() << "couldn't open output file" << m_file->fileName();
			delete m_file;
			m_file = 0;
			return false;
		}
	} else {
		m_file = new KTemporaryFile();
		if(!((KTemporaryFile *)m_file)->open()) {
			kDebug() << "couldn't open output file" << m_file->fileName();
			delete m_file;
			m_file = 0;
			return false;
		}
	}
	return true;
}

bool
FileSaveHelper::close()
{
	if(!m_file)
		return false;

	if(m_url.isLocalFile()) {
		delete m_file;                  // the destructor calls finalize() which in turn calls close()
		m_file = 0;
		return true;
	} else {
		m_file->close();                // close the file to ensure everything has been written to it

		bool success = m_overwrite ? KIO::NetAccess::upload(m_file->fileName(), m_url, 0) : KIO::NetAccess::file_copy(KUrl(m_file->fileName()), m_url, 0);

		delete m_file;                  // the destructor removes the temporary file
		m_file = 0;

		return success;
	}
}

bool
FileSaveHelper::exists(const KUrl &url)
{
	return KIO::NetAccess::exists(url, KIO::NetAccess::DestinationSide, 0);
}
