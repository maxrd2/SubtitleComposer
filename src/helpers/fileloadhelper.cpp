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

#include "fileloadhelper.h"

#include <QIODevice>
#include <QBuffer>
#include <QDebug>

#include <kio_version.h>
#include <kio/statjob.h>
#include <kio/storedtransferjob.h>

FileLoadHelper::FileLoadHelper(const QUrl &url) :
	m_url(url),
	m_file(0)
{}

FileLoadHelper::~FileLoadHelper()
{
	if(m_file)
		close();
}

const QUrl &
FileLoadHelper::url()
{
	return m_url;
}

QIODevice *
FileLoadHelper::file()
{
	return m_file;
}

bool
FileLoadHelper::open()
{
	if(m_file)
		return false;

	if(m_url.isLocalFile()) {
		m_file = new QFile(m_url.toLocalFile());
		if(!m_file->open(QIODevice::ReadOnly)) {
			qDebug() << "Couldn't open file" << static_cast<QFile *>(m_file)->fileName();
			delete m_file;
			m_file = nullptr;
			return false;
		}
	} else {
#if KIO_VERSION < QT_VERSION_CHECK(5, 69, 0)
		KIO::Job *job = KIO::stat(m_url, KIO::StatJob::SourceSide, 2);
#else
		KIO::Job *job = KIO::statDetails(m_url, KIO::StatJob::SourceSide, KIO::StatDefaultDetails, KIO::HideProgressInfo);
#endif
		if(!job->exec()) {
			qDebug() << "Failed to start KIO::stat job" << m_url;
			return false;
		}

		KIO::StoredTransferJob *xjob = KIO::storedGet(m_url);
		if(!xjob) {
			qDebug() << "Couldn't open url" << m_url;
			qDebug() << xjob->errorString();
			return false;
		}
		connect(xjob, &KIO::StoredTransferJob::result, this, &FileLoadHelper::downloadComplete);
		m_file = new QBuffer(&m_data);
	}

	return true;
}

bool
FileLoadHelper::close()
{
	if(!m_file)
		return false;

	delete m_file;
	m_file = nullptr;

	return true;
}

bool
FileLoadHelper::exists(const QUrl &url)
{
#if KIO_VERSION < QT_VERSION_CHECK(5, 69, 0)
	KIO::Job *job = KIO::stat(url, KIO::StatJob::SourceSide, 2);
#else
	KIO::Job *job = KIO::statDetails(url, KIO::StatJob::SourceSide, KIO::StatDefaultDetails, KIO::HideProgressInfo);
#endif
	return job->exec();
}

void
FileLoadHelper::downloadComplete(KJob *job)
{
	m_data = static_cast<KIO::StoredTransferJob *>(job)->data();
}
