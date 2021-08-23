/*
    SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRECENTFILESACTIONEXT_H
#define KRECENTFILESACTIONEXT_H

#include <QMap>
#include <QUrl>

#include <KRecentFilesAction>

class KRecentFilesActionExt : public KRecentFilesAction
{
	Q_OBJECT

public:
	explicit KRecentFilesActionExt(QObject *parent);
	virtual ~KRecentFilesActionExt();

	static QString encodingForUrl(const QUrl &url);

	void loadEntries(const KConfigGroup &configGroup);
	void saveEntries(const KConfigGroup &configGroup);

	void addUrl(const QUrl &url, const QString &encoding, const QString &name);
	inline void addUrl(const QUrl &url, const QString &encoding) { addUrl(url, encoding, QString()); }
};

#endif
