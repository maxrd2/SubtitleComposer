/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "commondefs.h"

#include <cstdlib>
#include <climits>

#include <QStringBuilder>
#include <QDir>
#include <QFileInfo>

#include <QDebug>
#include <QStandardPaths>

#ifndef Q_OS_WIN
#include <unistd.h>
#endif

/**
 * String
 */

QString
String::title(const QString &text)
{
	QString auxText = text.toLower();
	for(const QChar sep: QStringLiteral(" -_([:,;./\\\t\n\"")) {
		QStringList auxWordsList;
		for(const QString &word: auxText.split(sep)) {
			if(!word.isEmpty())
				auxWordsList << (word.at(0).toUpper() + word.mid(1));
			else
				auxWordsList << QString();
		}
		auxText = auxWordsList.join(sep);
	}
	return auxText;
}

QString
String::capitalize(const QString &text)
{
	return text.isEmpty() ? QString() : text.at(0).toUpper() + text.mid(1).toLower();
}

QString
String::sentence(const QString &text)
{
	return text.isEmpty() ? QString() : text.at(0).toUpper() + text.mid(1).toLower();
}

int
String::rfindFunctionStart(const QString &text)
{
	qsizetype pos[5] = {
		text.lastIndexOf(QLatin1String("capitalize{")),
		text.lastIndexOf(QLatin1String("titlecase{")),
		text.lastIndexOf(QLatin1String("uppercase{")),
		text.lastIndexOf(QLatin1String("lowercase{")),
		text.lastIndexOf(QLatin1String("ascii{")),
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

	QDir parcialPath(QStringLiteral("/"));
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
	QStringList tokens = path.split(QChar('/'), QString::SkipEmptyParts);
#else
	QStringList tokens = path.split(QChar('/'), Qt::SkipEmptyParts);
#endif

	for(QStringList::ConstIterator it = tokens.constBegin(), end = tokens.constEnd(); it != end; ++it) {
		parcialPath.setPath(parcialPath.path() + '/' + *it);
		if(!QFileInfo::exists(parcialPath.path())) {
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
	while(tempDir.endsWith(QDir::separator()))
		tempDir.chop(1);
	return tempDir;
}

QUrl
System::newUrl(const QUrl &baseUrl, const QString &fileName, const QString &extension, int retries)
{
	if(retries < 0)
		retries = INT_MAX;

	QString newFileName(fileName);
	QString newFileNameBuilder(fileName + QStringLiteral("-%1"));
	if(!extension.isEmpty()) {
		newFileName += '.' + extension;
		newFileNameBuilder += '.' + extension;
	}

	QString newFileDir = baseUrl.path();
	if(!newFileDir.endsWith(QDir::separator()))
		newFileDir += QDir::separator();

	int i = 2;
	retries += i;

	QFileInfo dirInfo(newFileDir);
	if(dirInfo.isDir() && dirInfo.isWritable()) {
		QString newFilePath = newFileDir + newFileName;
		while(i < retries && QFile::exists(newFilePath))
			newFilePath = newFileDir + newFileNameBuilder.arg(i++);
		if(i < retries)
			return QUrl::fromLocalFile(newFilePath);
	}

	// could not return a writable url in baseUrl so we return one in the temp dir
	newFileDir = tempDir() + QDir::separator();

	i = 2;
	QString newFilePath = newFileDir + newFileName;
	while(QFile::exists(newFilePath))
		newFilePath = newFileDir + newFileNameBuilder.arg(i++);

	return QUrl::fromLocalFile(newFilePath);
}

/*static*/ QUrl
System::urlFromPath(const QString &path)
{
	const QFileInfo file(path);
	const QUrl url(path);
	if(file.exists() || url.isRelative())
		return QUrl::fromLocalFile(file.absoluteFilePath());
	return url;
}


/*static*/ bool
System::urlIsInside(const QUrl &url, const QString &path)
{
	if(!url.scheme().isEmpty() && url.scheme() != QLatin1String("file"))
		return false;
	QString urlPath = url.toLocalFile();
	return urlPath.startsWith(path);
}

/*static*/ bool
System::urlIsInside(const QUrl &url, QStringList &path)
{
	for(QStringList::const_iterator i = path.constBegin(), end = path.constEnd(); i != end; i++) {
		if(urlIsInside(url, *i))
			return true;
	}
	return false;
}
