/**
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2015 Mladen Milinkovic <max@smoothware.net>
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

#include "commondefs.h"

#include <cstdlib>
#include <climits>

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QRegExp>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>

#include <QDebug>
#include <QStandardPaths>

#include <kio/statjob.h>

#ifndef Q_WS_WIN
#include <unistd.h>
#endif

/**
 * String
 */

QString
String::title(const QString &text)
{
	QString auxText = text.toLower();
	const QString separators(" -_([:,;./\\\t\n\"");
	int len = separators.length(), sepIdx = 0;
	while(sepIdx < len) {
		QStringList auxWordsList;
		QStringList wordsList = auxText.split(separators[sepIdx]);
		for(QStringList::ConstIterator it = wordsList.begin(), end = wordsList.end(); it != end; ++it)
			if(!(*it).isEmpty())
				auxWordsList.append((*it)[0].toUpper() + (*it).mid(1));
			else
				auxWordsList.append("");
		auxText = auxWordsList.join(separators[sepIdx]);
		sepIdx++;
	}
	return auxText;
}

QString
String::capitalize(const QString &text)
{
	return text.isEmpty() ? "" : text[0].toUpper() + text.mid(1).toLower();
}

QString
String::sentence(const QString &text)
{
	return text.isEmpty() ? "" : text[0].toUpper() + text.mid(1).toLower();
}

int
String::rfindFunctionStart(const QString &text)
{
	int pos[5] = {
		text.lastIndexOf("capitalize{"),
		text.lastIndexOf("titlecase{"),
		text.lastIndexOf("uppercase{"),
		text.lastIndexOf("lowercase{"),
		text.lastIndexOf("ascii{"),
	};

	return qMax(qMax(qMax(qMax(pos[0], pos[1]), pos[2]), pos[3]), pos[4]);
}

int
String::rfindFunctionEnd(const QString &text, int startPos)
{
	int pos = startPos, len = text.length();
	while(pos < len) {
		// next is safe because idx will always be greater than 0
		if(text[pos] == '}' && text[pos - 1] != '\\')
			return pos;
		pos++;
	}
	return -1;
}

/**
 * System:
 */

bool
System::recursiveMakeDir(const QString &path, QStringList *createdDirsList)
{
	if(createdDirsList)
		createdDirsList->clear();

	QDir parcialPath("/");
	QStringList tokens = path.split('/', QString::SkipEmptyParts);
	for(QStringList::ConstIterator it = tokens.begin(), end = tokens.end(); it != end; ++it) {
		parcialPath.setPath(parcialPath.path() + '/' + *it);
		if(!QFileInfo(parcialPath.path()).exists()) {
			if(!QDir().mkdir(parcialPath.path()))
				return false;
			if(createdDirsList)
				createdDirsList->prepend(parcialPath.path());
		}
	}

	return true;
}

bool
System::copy(const QString &srcPath, const QString &dstPath)
{
	if(QFile::exists(dstPath))
		if(!System::remove(dstPath))
			return false;

	return QFile::copy(srcPath, dstPath);
}

bool
System::move(const QString &srcPath, const QString &dstPath)
{
	if(!System::copy(srcPath, dstPath)) {
		qDebug() << "move: error copying" << srcPath << "to" << dstPath;

		return false;
	}

	if(!QFile::remove(srcPath)) {
		qDebug() << "move: error removing" << srcPath;

		return false;
	}

	return true;
}

bool
System::remove(const QString &path)
{
	return QFile::remove(path);
}

bool
System::isReadable(const QString &path)
{
	QFileInfo fileInfo(path);
	return fileInfo.isFile() && fileInfo.isReadable();
}

bool
System::isWritable(const QString &path)
{
	QFileInfo fileInfo(path);
	return fileInfo.isFile() && fileInfo.isWritable();
}

QString
System::homeDir()
{
	return QDir::homePath();
}

QString
System::tempDir()
{
	QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
	tempDir.remove(QRegExp("[\\/]+$"));
	return tempDir;
}

QUrl
System::newUrl(const QUrl &baseUrl, const QString &fileName, const QString &extension, int retries)
{
	if(retries < 0)
		retries = INT_MAX;

	QString newFileName(fileName);
	QString newFileNameBuilder(fileName + "-%1");
	if(!extension.isEmpty()) {
		newFileName += '.' + extension;
		newFileNameBuilder += '.' + extension;
	}

	QString newFileDir = baseUrl.path();
	newFileDir.remove(QRegExp("[\\/]+$"));
	newFileDir += '/';

	if(baseUrl.isLocalFile()) {
		QFileInfo dirInfo(newFileDir);
		if(dirInfo.isDir() && dirInfo.isWritable()) {
			QString newFilePath = newFileDir + newFileName;
			if(!QFile::exists(newFilePath))
				return QUrl(newFilePath);

			for(int i = 2, limit = retries + i; i < limit; ++i) {
				newFilePath = newFileDir + newFileNameBuilder.arg(i);
				if(!QFile::exists(newFilePath))
					return QUrl(newFilePath);
			}
		}
	} else {
		QUrl newUrl = baseUrl;

		newUrl.setPath(newFileDir + newFileName);
		KIO::Job *job = KIO::stat(newUrl, KIO::StatJob::DestinationSide, 2);
		if(!job->exec())
			return newUrl;

		for(int i = 2, limit = retries + i; i < limit; ++i) {
			newUrl.setPath(newFileDir + newFileNameBuilder.arg(i));
			job = KIO::stat(newUrl, KIO::StatJob::DestinationSide, 2);
			if(!job->exec())
				return newUrl;
		}
	}

	// could not return a writable url in baseUrl so we return one in the temp dir
	newFileDir = tempDir() + '/';

	QString newFilePath = newFileDir + newFileName;
	if(!QFile::exists(newFilePath))
		return QUrl(newFilePath);

	int i = 2;
	do {
		newFilePath = newFileDir + newFileNameBuilder.arg(i++);
	} while(QFile::exists(newFilePath));

	return QUrl(newFilePath);
}

/*static*/ QUrl
System::urlFromPath(QString path)
{
	QUrl url(path);
	if(url.isRelative()) {
		url.setScheme("file");
		if(path[0] != QDir::separator())
			url.setPath(QDir::currentPath() + "/" + path);
	}
	return url;
}


/*static*/ bool
System::urlIsInside(const QUrl url, QString path)
{
	if(!url.scheme().isEmpty() && url.scheme() != "file")
		return false;
	QString urlPath = url.toLocalFile();
	return urlPath.startsWith(path);
}

/*static*/ bool
System::urlIsInside(const QUrl url, QStringList path)
{
	for(QStringList::const_iterator i = path.constBegin(), end = path.constEnd(); i != end; i++) {
		if(urlIsInside(url, *i))
			return true;
	}
	return false;
}
