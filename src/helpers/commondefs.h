/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMMONDEFS_H
#define COMMONDEFS_H

#include <QString>
#include <QStringList>

#include <QUrl>

class String
{
public:
	static QString title(const QString &text);
	static QString capitalize(const QString &text);
	static QString sentence(const QString &text);

protected:
	static int rfindFunctionStart(const QString &text);
	static int rfindFunctionEnd(const QString &text, int startPos);
};

class System
{
public:
	// returns false on error
	static bool copy(const QString &srcPath, const QString &dstPath);
	static bool move(const QString &srcPath, const QString &dstPath);
	static bool remove(const QString &path);
	static bool recursiveMakeDir(const QString &path, QStringList *createdDirsList = 0);

	static bool isReadable(const QString &path);
	static bool isWritable(const QString &path);

	static QString homeDir();
	static QString tempDir();

	static QUrl urlFromPath(const QString &path);

	static QUrl newUrl(const QUrl &baseUrl, const QString &fileName = QStringLiteral("tempfile"), const QString &extension = QString(), int retries = 10);

	static bool urlIsInside(const QUrl &url, const QString &path);
	static bool urlIsInside(const QUrl &url, QStringList &path);
};

#endif
