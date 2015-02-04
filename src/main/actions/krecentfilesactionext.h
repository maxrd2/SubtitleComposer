#ifndef KRECENTFILESACTIONEXT_H
#define KRECENTFILESACTIONEXT_H

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QUrl>
#include <KConfigGroup>
#include <KSelectAction>

/// Taken from KDELibs and rewritten almost entirely because the
/// KDE implementations is buggy and limited for our needs.
class KRecentFilesActionExt : public KSelectAction
{
	Q_OBJECT

	Q_PROPERTY(int maxItems READ maxItems WRITE setMaxItems)

public:
	KRecentFilesActionExt(QObject *parent = 0);
	virtual ~KRecentFilesActionExt();

	int maxItems() const;

	bool isEmpty() const;
	int count() const;

	QList<QUrl> urls() const;

	QString encodingForUrl(const QUrl &url) const;

	virtual QAction * removeAction(QAction *action);

public slots:
	void setMaxItems(int maxItems);

	void setUrls(const QList<QUrl> &urls);

	void addUrl(const QUrl &url);
	void removeUrl(const QUrl &url);
	void clearUrls();

	void loadEntries(const KConfigGroup &group);
	void saveEntries(const KConfigGroup &group);

signals:
	void urlSelected(const QUrl &url);

protected:
	void setUrls(const QList<QUrl> &urls, bool ignoreCollisions);

	virtual QAction * actionForUrl(const QUrl &url) const;

protected slots:
	void onActionTriggered(QAction *action);
	void onActionChanged();

protected:
	int m_maxItems;

	QMap<QAction *, QUrl> m_urls;
	QMap<QUrl, QAction *> m_actions;

	QAction *m_separatorAction;
	QAction *m_clearHistoryAction;
};

#endif
