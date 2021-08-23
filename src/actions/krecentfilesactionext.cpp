/*
    SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krecentfilesactionext.h"

#include <KConfigGroup>
#include <QFile>

static QMap<QUrl, QString> s_subtitleEncodings;

KRecentFilesActionExt::KRecentFilesActionExt(QObject *parent)
	: KRecentFilesAction(parent)
{
}


KRecentFilesActionExt::~KRecentFilesActionExt()
{
}

QString
KRecentFilesActionExt::encodingForUrl(const QUrl &url)
{
	return s_subtitleEncodings.value(url);
}

void
KRecentFilesActionExt::addUrl(const QUrl &url, const QString &encoding, const QString &name)
{
	s_subtitleEncodings.insert(url, encoding);
	KRecentFilesAction::addUrl(url, name);
}

void
KRecentFilesActionExt::loadEntries(const KConfigGroup &configGroup)
{
	KRecentFilesAction::loadEntries(configGroup);

	for(int i = 1; ; i++) {
		const QUrl url = QUrl::fromUserInput(configGroup.readPathEntry(QStringLiteral("File%1").arg(i), QString()));
		if(url.isEmpty())
			break;
		const QString encoding = configGroup.readEntry(QStringLiteral("Encoding%1").arg(i), QString());
		if(!encoding.isEmpty())
			s_subtitleEncodings.insert(url, encoding);
	}
}

void
KRecentFilesActionExt::saveEntries(const KConfigGroup &configGroup)
{
	KConfigGroup config = configGroup;
	config.deleteGroup();

	KRecentFilesAction::saveEntries(config);

	for(int i = 1; ; i++) {
		const QUrl url = QUrl::fromUserInput(configGroup.readPathEntry(QStringLiteral("File%1").arg(i), QString()));
		if(url.isEmpty())
			break;
		const QString encoding = encodingForUrl(url);
		if(!encoding.isEmpty())
			config.writeEntry(QStringLiteral("Encoding%1").arg(i), encoding);
	}
}
