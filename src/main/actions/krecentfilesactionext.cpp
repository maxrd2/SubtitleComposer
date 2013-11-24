/***************************************************************************
 *   Copyright (C) 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#include "krecentfilesactionext.h"

#include <QtCore/QFile>

#include <KLocale>
#include <KDebug>
#include <KStandardDirs>

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
KRecentFilesActionExt::encodingForUrl(const KUrl &url) const
{
	QAction *action = actionForUrl(url);
	return action ? m_urls[action].fileEncoding() : QString();
}

KUrl::List
KRecentFilesActionExt::urls() const
{
	KUrl::List urls;
	QList<QAction *> actions = this->actions();
	QAction *action;
	for(QList<QAction *>::ConstIterator it = actions.constBegin(), end = actions.constEnd(); it != end; ++it) {
		action = *it;
		if(action != m_separatorAction && action != m_clearHistoryAction)
			urls.append(m_urls[action]);
	}
	return urls;
}

void
KRecentFilesActionExt::setUrls(const KUrl::List &urls, bool ignoreCollisions)
{
	clearUrls();

	QString entryText("%1 [%2]");

	for(KUrl::List::ConstIterator it = urls.begin(), end = urls.end(); it != end; ++it) {
		if((*it).isLocalFile() && !KGlobal::dirs()->relativeLocation("tmp", (*it).path()).startsWith('/'))
			continue; // don't store temporary paths

		if(!ignoreCollisions && actionForUrl(*it))
			continue;

		QAction *action = new QAction(entryText.arg((*it).fileName()).arg((*it).isLocalFile() ? (*it).path() : (*it).pathOrUrl()
																		  ),
									  selectableActionGroup()
									  );

		m_urls[action] = *it;
		m_actions[*it] = action;

		addAction(action);
	}

	addAction(m_separatorAction);
	addAction(m_clearHistoryAction);
}

void
KRecentFilesActionExt::setUrls(const KUrl::List &urls)
{
	setUrls(urls, false);
}

void
KRecentFilesActionExt::addUrl(const KUrl &url)
{
	removeUrl(url); // avoid duplicates entries (without taking encoding into account)

	KUrl::List newUrls = urls();
	newUrls.prepend(url);

	setUrls(newUrls, true);
}

QAction *
KRecentFilesActionExt::removeAction(QAction *action)
{
	action = KSelectAction::removeAction(action);
	if(m_urls.contains(action)) {
		m_actions.remove(m_urls[action]);
		m_urls.remove(action);
	}
	return action;
}

void
KRecentFilesActionExt::removeUrl(const KUrl &url)
{
	if(QAction * action = actionForUrl(url))
		removeAction(action)->deleteLater();
}

void
KRecentFilesActionExt::clearUrls()
{
	while(!m_actions.empty())
		removeAction(m_actions.begin().value())->deleteLater();

	removeAction(m_clearHistoryAction);
	removeAction(m_separatorAction);
}

QAction *
KRecentFilesActionExt::actionForUrl(const KUrl &url) const
{
	KUrl refUrl(url);
	refUrl.setFileEncoding(QString());

	for(QMap<KUrl, QAction *>::ConstIterator it = m_actions.begin(), end = m_actions.end(); it != end; ++it) {
		KUrl curUrl(it.key());
		curUrl.setFileEncoding(QString());
		if(curUrl == refUrl)
			return it.value();
	}
	return 0;
}

void
KRecentFilesActionExt::loadEntries(const KConfigGroup &group)
{
	KUrl::List urls;

	QString key("File%1");
	for(int index = 0, size = qMin(group.readEntry<int>("Files", m_maxItems), m_maxItems); index < size; ++index) {
		QString value = group.readPathEntry(key.arg(index), QString());
		if(!value.isEmpty()) {
			KUrl url(value);
			if(url.isLocalFile() && !QFile::exists(url.path()))
				continue; // Don't restore if file doesn't exist anymore
			urls.append(url);
		}
	}

	setUrls(urls, true);
}

void
KRecentFilesActionExt::saveEntries(const KConfigGroup &g)
{
	KConfigGroup group(g);

	group.deleteGroup();

	KUrl::List urls = this->urls();

	int index = 0;
	QString key("File%1");
	for(KUrl::List::ConstIterator it = urls.constBegin(), end = urls.constEnd(); it != end; ++it)
		group.writePathEntry(key.arg(index++), (*it).pathOrUrl());

	group.writeEntry("Files", urls.count());
}

void
KRecentFilesActionExt::onActionTriggered(QAction *action)
{
	if(action == m_clearHistoryAction)
		clearUrls();
	else
		emit urlSelected(m_urls[action]);
}

void
KRecentFilesActionExt::onActionChanged()
{
	if(isEmpty())
		setEnabled(false);
}

#include "krecentfilesactionext.moc"
