/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "filesavehelper.h"

#include <QDebug>
#include <QSaveFile>
#include <QTemporaryFile>
#include <QFileDevice>

#include <kio_version.h>
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
		m_file = new QSaveFile(m_url.toLocalFile());
		if(!m_file->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
			qDebug() << "Couldn't open output file" << m_file->fileName();
			delete m_file;
			m_file = nullptr;
			return false;
		}
	} else {
		m_file = new QTemporaryFile();
		if(!static_cast<QTemporaryFile *>(m_file)->open()) {
			qDebug() << "Couldn't open output file" << m_file->fileName();
			delete m_file;
			m_file = nullptr;
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
		m_file = nullptr;
		return true;
	} else {
		m_file->close();
		KIO::Job *job = KIO::file_copy(QUrl::fromLocalFile(m_file->fileName()), m_url, -1, m_overwrite ? KIO::Overwrite : KIO::DefaultFlags);
		bool success = job->exec();
		delete m_file;
		m_file = nullptr;

		return success;
	}
}

bool
FileSaveHelper::exists(const QUrl &url)
{
#if KIO_VERSION < QT_VERSION_CHECK(5, 69, 0)
	KIO::Job *job = KIO::stat(url, KIO::StatJob::DestinationSide, 2);
#else
	KIO::Job *job = KIO::statDetails(url, KIO::StatJob::DestinationSide, KIO::StatDefaultDetails, KIO::HideProgressInfo);
#endif
	return job->exec();
}
