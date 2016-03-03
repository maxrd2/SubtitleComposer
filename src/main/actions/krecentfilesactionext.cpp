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

#include "krecentfilesactionext.h"

#include "common/commondefs.h"

#include <QFile>
#include <QDebug>
#include <QStandardPaths>

#include <KLocalizedString>

KRecentFilesActionExt::KRecentFilesActionExt(QObject *parent) :
	KSelectAction(parent),
	m_maxItems(10)
{
	setMenuAccelsEnabled(false);

	m_separatorAction = new QAction(QString(), 0);
	m_separatorAction->setSeparator(true);

	m_clearHistoryAction = new QAction(i18n("Clear List"), 0);

	connect(this, SIGNAL(triggered(QAction *)), this, SLOT(onActionTriggered(QAction *)));
	connect(this, SIGNAL(changed()), this, SLOT(onActionChanged()));
}

KRecentFilesActionExt::~KRecentFilesActionExt()
{
	selectableActionGroup()->removeAction(m_separatorAction);
	delete m_separatorAction;
	selectableActionGroup()->removeAction(m_clearHistoryAction);
	delete m_clearHistoryAction;
}

int
KRecentFilesActionExt::maxItems() const
{
	return m_maxItems;
}

void
KRecentFilesActionExt::setMaxItems(int maxItems)
{
	if(m_maxItems != maxItems && maxItems > 0) {
		m_maxItems = maxItems;

		while(count() > maxItems)
			removeAction(selectableActionGroup()->actions().last())->deleteLater();
	}
}

bool
KRecentFilesActionExt::isEmpty() const
{
	return m_urls.isEmpty();
}

int
KRecentFilesActionExt::count() const
{
	return m_urls.count();
}

QString
KRecentFilesActionExt::encodingForUrl(const QUrl &url) const
{
	QAction *action = actionForUrl(url);
	if(action) {
		QRegExp rx("encoding=([^&]*)");
		return rx.indexIn(url.query()) == -1 ? "" : rx.cap(1);
	}
	return "";
}

QList<QUrl>
KRecentFilesActionExt::urls() const
{
	return m_urls.keys();
}

void
KRecentFilesActionExt::setUrls(const QList<QUrl> &urls)
{
	QString entryText("%1 [%2]");
	QStringList tempList = QStandardPaths::standardLocations(QStandardPaths::TempLocation);

	clearUrls();

	for(QList<QUrl>::ConstIterator it = urls.begin(), end = urls.end(); it != end; ++it) {
		if(System::urlIsInside(*it, tempList))
			continue; // don't store temporary paths

		if(actionForUrl(*it))
			continue;

		QString path(it->path());
		int n = path.lastIndexOf('/');
		if(n != -1)
			path.chop(path.length() - n - 1);
		QAction *action = new QAction(entryText.arg(it->fileName()).arg(path), selectableActionGroup());

		m_urls[*it] = action;

		addAction(action);
	}

	addAction(m_separatorAction);
	addAction(m_clearHistoryAction);
}

void
KRecentFilesActionExt::addUrl(const QUrl &url)
{
	removeUrl(url); // avoid duplicates entries (without taking encoding into account)

	QList<QUrl> newUrls = urls();
	newUrls.prepend(url);

	setUrls(newUrls);
}

QAction *
KRecentFilesActionExt::removeAction(QAction *action)
{
	action = KSelectAction::removeAction(action);
	for(QMap<QUrl, QAction *>::Iterator it = m_urls.begin(), end = m_urls.end(); it != end; ++it) {
		if(*it == action) {
			m_urls.erase(it);
			break;
		}
	}
	return action;
}

void
KRecentFilesActionExt::removeUrl(const QUrl &url)
{
	while(QAction *action = actionForUrl(url))
		removeAction(action)->deleteLater();
}

void
KRecentFilesActionExt::clearUrls()
{
	while(!m_urls.empty())
		removeAction(m_urls.begin().value())->deleteLater();

	removeAction(m_clearHistoryAction);
	removeAction(m_separatorAction);
}

QAction *
KRecentFilesActionExt::actionForUrl(QUrl url) const
{
	url.setQuery(QString());
	for(QMap<QUrl, QAction *>::const_iterator it = m_urls.begin(); it != m_urls.end(); it++) {
		QUrl key = it.key();
		key.setQuery(QString());
		if(key == url)
			return *it;
	}
	return NULL;
}

void
KRecentFilesActionExt::loadEntries(const KConfigGroup &group)
{
	QList<QUrl> urls;

	QString key("File%1");
	for(int index = 0, size = qMin(group.readEntry<int>("Files", m_maxItems), m_maxItems); index < size; ++index) {
		QString value = group.readPathEntry(key.arg(index), QString());
		if(!value.isEmpty()) {
			QUrl url = System::urlFromPath(value);
			if(!QFile::exists(url.path()))
				continue;
			urls.append(url);
		}
	}

	setUrls(urls);
}

void
KRecentFilesActionExt::saveEntries(const KConfigGroup &g)
{
	KConfigGroup group(g);

	group.deleteGroup();

	QList<QUrl> urls = this->urls();

	int index = 0;
	QString key("File%1");
	for(QList<QUrl>::ConstIterator it = urls.constBegin(), end = urls.constEnd(); it != end; ++it)
		group.writePathEntry(key.arg(index++), (*it).toString(QUrl::PreferLocalFile));

	group.writeEntry("Files", urls.count());
}

void
KRecentFilesActionExt::onActionTriggered(QAction *action)
{
	if(action == m_clearHistoryAction) {
		clearUrls();
	} else {
		for(QMap<QUrl, QAction *>::ConstIterator it = m_urls.constBegin(), end = m_urls.constEnd(); it != end; ++it) {
			if(*it == action) {
				emit urlSelected(it.key());
				break;
			}
		}
	}
}

void
KRecentFilesActionExt::onActionChanged()
{
	if(isEmpty())
		setEnabled(false);
}


