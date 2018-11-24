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

#include "filesavehelper.h"

#include <QDebug>
#include <QSaveFile>
#include <QTemporaryFile>
#include <QFileDevice>

#include <kio/filecopyjob.h>
#include <kio/statjob.h>

FileSaveHelper::FileSaveHelper(const QUrl &url, bool overwrite) :
	m_url(url),
	m_overwrite(overwrite),
	m_file(0)
{}

FileSaveHelper::~FileSaveHelper()
{
	if(m_file)
		close();
}

const QUrl &
FileSaveHelper::url()
{
	return m_url;
}

bool
FileSaveHelper::overwrite()
{
	return m_overwrite;
}

QFileDevice *
FileSaveHelper::file()
{
	return m_file;
}

bool
FileSaveHelper::open()
{
	if(m_file)
		return false;

	if(!m_overwrite && exists(m_url))
		return false;

	if(m_url.isLocalFile()) {
		m_file = new QSaveFile(m_url.path());
		if(!m_file->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
			qDebug() << "couldn't open output file" << m_file->fileName();
			delete m_file;
			m_file = 0;
			return false;
		}
	} else {
		m_file = new QTemporaryFile();
		if(!static_cast<QTemporaryFile *>(m_file)->open()) {
			qDebug() << "couldn't open output file" << m_file->fileName();
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
		static_cast<QSaveFile*>(m_file)->commit();
		delete m_file;
		m_file = 0;
		return true;
	} else {
		m_file->close();
		KIO::Job *job = KIO::file_copy(QUrl::fromLocalFile(m_file->fileName()), m_url, -1, m_overwrite ? KIO::Overwrite : KIO::DefaultFlags);
		bool success = job->exec();
		delete m_file;
		m_file = 0;

		return success;
	}
}

bool
FileSaveHelper::exists(const QUrl &url)
{
	KIO::Job *job = KIO::stat(url, KIO::StatJob::DestinationSide, 2);
	return job->exec();
}
