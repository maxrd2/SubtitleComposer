/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef FILESAVEHELPER_H
#define FILESAVEHELPER_H

#include <QUrl>

QT_FORWARD_DECLARE_CLASS(QFileDevice)

class FileSaveHelper
{
public:
	FileSaveHelper(const QUrl &url, bool overwrite);
	~FileSaveHelper();

	const QUrl & url();
	bool overwrite();
	QFileDevice * file();

	bool open();
	bool close();

	static bool exists(const QUrl &url);

private:
	QUrl m_url;
	bool m_overwrite;
	QFileDevice *m_file;
};

#endif
