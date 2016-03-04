#ifndef SCRIPTSMANAGER_H
#define SCRIPTSMANAGER_H

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QObject>
#include <QMap>
#include <QUrl>

#include "ui_scriptsmanager.h"

QT_FORWARD_DECLARE_CLASS(QAction)
QT_FORWARD_DECLARE_CLASS(QDialog)
QT_FORWARD_DECLARE_CLASS(QMenu)
QT_FORWARD_DECLARE_CLASS(QPushButton)
class TreeView;

namespace SubtitleComposer {
class Subtitle;

class ScriptsManager : public QObject, private Ui::ScriptsManager
{
	Q_OBJECT

public:
	explicit ScriptsManager(QObject *parent = 0);
	virtual ~ScriptsManager();

	QString currentScriptName() const;
	QStringList scriptNames() const;

	virtual bool eventFilter(QObject *object, QEvent *event);

public slots:
	void setSubtitle(Subtitle *subtitle = 0);

	void showDialog();

	void createScript(const QString &scriptName = QString());
	void addScript(const QUrl &srcScriptUrl = QUrl());
	void removeScript(const QString &scriptName = QString());
	void editScript(const QString &scriptName = QString());
	void runScript(const QString &scriptName = QString());
	void reloadScripts();

private:
	static const QStringList & mimeTypes();
	QMenu * toolsMenu();

	static void findAllFiles(QString path, QStringList &findAllFiles);

private slots:
	void onToolsMenuActionTriggered(QAction *action);

private:
	QMap<QString, QString> m_scripts;               // name => path
	QDialog *m_dialog;
};

class Debug : public QObject
{
	Q_OBJECT

public:
	Debug();
	~Debug();

public slots:
	void information(const QString &message);
	void warning(const QString &message);
	void error(const QString &message);
};
}
#endif
