/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCRIPTSMANAGER_H
#define SCRIPTSMANAGER_H

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
class SCScript;
class Subtitle;

class ScriptsManager : public QObject, private Ui::ScriptsManager
{
	Q_OBJECT

public:
	explicit ScriptsManager(QObject *parent = 0);
	virtual ~ScriptsManager();

	bool eventFilter(QObject *object, QEvent *event) override;

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
	const SCScript * currentScript() const;
	const SCScript * findScript(const QString filename) const;

	static void findAllFiles(QString path, QStringList &findAllFiles);

private slots:
	void onToolsMenuActionTriggered(QAction *action);

private:
	QDialog *m_dialog;
};
}
#endif
