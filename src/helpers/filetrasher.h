/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef FILETRASHER_H
#define FILETRASHER_H

#include <QUrl>

class FileTrasher
{
public:
	FileTrasher(const QUrl &url);
	FileTrasher(const QString &path);
	~FileTrasher();

	const QUrl & url();

	bool exec();

private:
	QUrl m_url;
};

#endif
