/*
 * Copyright (C) 2010-2018 Mladen Milinkovic <max@smoothware.net>
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
