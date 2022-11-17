/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "filetrasher.h"

#include <kio/copyjob.h>

FileTrasher::FileTrasher(const QUrl &url)
	: m_url(url)
{}

FileTrasher::~FileTrasher()
{}

FileTrasher::FileTrasher(const QString &path) : m_url()
{
	m_url.setPath(path);
	m_url.setScheme(QStringLiteral("file"));
}

const QUrl &
FileTrasher::url()
{
	return m_url;
}

bool
FileTrasher::exec()
{
	KIO::CopyJob *job = KIO::trash(m_url);
	// NOTE: the call deletes job!
	return job->exec();
}
