
/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
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

#include "scriptsmanager.h"
#include "scripting_rangesmodule.h"
#include "scripting_stringsmodule.h"
#include "scripting_subtitlemodule.h"
#include "scripting_subtitlelinemodule.h"

#include "application.h"
#include "actions/useraction.h"
#include "actions/useractionnames.h"
#include "dialogs/textinputdialog.h"
#include "helpers/fileloadhelper.h"
#include "helpers/filetrasher.h"
#include "widgets/treeview.h"

#include <QStringListModel>
#include <QStandardPaths>
#include <QDialog>
#include <QFileDialog>
#include <QMenuBar>
#include <QMenu>
#include <QDesktopServices>
#include <QKeyEvent>

#include <KMessageBox>
#include <KRun>
#include <KActionCollection>
#include <KLocalizedString>
#include <kio_version.h>

#include <kross/core/manager.h>
#include <kross/core/interpreter.h>
#include <kross/core/action.h>

namespace SubtitleComposer {
class InstalledScriptsModel : public QStringListModel
{
public:
	InstalledScriptsModel(QObject *parent = 0) : QStringListModel(parent) {}

	virtual ~InstalledScriptsModel() {}

	virtual QVariant headerData(int /*section */, Qt::Orientation orientation, int role) const override
	{
		if(role != Qt::DisplayRole || orientation == Qt::Vertical)
			return QVariant();
		return i18n("Installed Scripts");
	}

	virtual Qt::ItemFlags flags(const QModelIndex &index) const override
	{
		return QAbstractItemModel::flags(index);
	}
};
}

using namespace SubtitleComposer;

Debug::Debug()
{}

Debug::~Debug()
{}

void
Debug::information(const QString &message)
{
	KMessageBox::information(app()->mainWindow(), message, i18n("Information"));
	qDebug() << message;
}

void
Debug::warning(const QString &message)
{
	KMessageBox::sorry(app()->mainWindow(), message, i18n("Warning"));
	qWarning() << message;
}

void
Debug::error(const QString &message)
{
	KMessageBox::error(app()->mainWindow(), message, i18n("Error"));
	qWarning() << message;
}

ScriptsManager::ScriptsManager(QObject *parent)
	: QObject(parent)
{
	m_dialog = new QDialog(app()->mainWindow());
	setupUi(m_dialog);

	scriptsView->installEventFilter(this);
	scriptsView->setModel(new InstalledScriptsModel(scriptsView));
	scriptsView->setRootIsDecorated(false);
	scriptsView->setSortingEnabled(false);

	connect(btnCreate, SIGNAL(clicked()), this, SLOT(createScript()));
	connect(btnAdd, SIGNAL(clicked()), this, SLOT(addScript()));
	connect(btnRemove, SIGNAL(clicked()), this, SLOT(removeScript()));
	connect(btnEdit, SIGNAL(clicked()), this, SLOT(editScript()));
	connect(btnRun, SIGNAL(clicked()), this, SLOT(runScript()));
	connect(btnRefresh, SIGNAL(clicked()), this, SLOT(reloadScripts()));

//	m_dialog->resize(350, 10);
}

ScriptsManager::~ScriptsManager()
{

}

void
ScriptsManager::setSubtitle(Subtitle *subtitle)
{
	btnRun->setEnabled(subtitle != 0);
}

void
ScriptsManager::showDialog()
{
	m_dialog->show();
}

QString
ScriptsManager::currentScriptName() const
{
	QModelIndex currentIndex = scriptsView->currentIndex();
	return currentIndex.isValid() ? scriptsView->model()->data(currentIndex, Qt::DisplayRole).toString() : 0;
}

QStringList
ScriptsManager::scriptNames() const
{
	return m_scripts.keys();
}

void
ScriptsManager::createScript(const QString &sN)
{
	QString scriptName = sN;

	while(scriptName.isEmpty() || m_scripts.contains(scriptName)) {
		if(m_scripts.contains(scriptName)
			&& KMessageBox::questionYesNo(app()->mainWindow(),
				i18n("You must enter an unused name to continue.\nWould you like to enter another name?"),
				i18n("Name Already Used"), KStandardGuiItem::cont(), KStandardGuiItem::cancel()) != KMessageBox::Yes)
			return;

		TextInputDialog nameDlg(i18n("Create New Script"), i18n("Script name:"));
		if(nameDlg.exec() != QDialog::Accepted)
			return;
		scriptName = nameDlg.value();
	}

	QDir scriptPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
	if(!scriptPath.exists("scripts"))
		scriptPath.mkpath("scripts");
	scriptPath.cd("scripts");

	QFile scriptFile(scriptPath.absoluteFilePath(scriptName));
	if(!scriptFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		KMessageBox::sorry(app()->mainWindow(), i18n("There was an error creating the file <b>%1</b>.", scriptPath.absoluteFilePath(scriptName)));
		return;
	}

	QTextStream outputStream(&scriptFile);
	QString scriptExtension = QFileInfo(scriptName).suffix().toLower();
	if(scriptExtension == QLatin1String("rb"))
		outputStream << "#!/usr/bin/env ruby";
	else if(scriptExtension == QLatin1String("py"))
		outputStream << "#!/usr/bin/env python";
	outputStream << "\n";

	scriptFile.close();

	reloadScripts();
	editScript(scriptName);
}

const QStringList &
ScriptsManager::mimeTypes()
{
	static QStringList mimeTypes;

	if(mimeTypes.isEmpty()) {
		QHash<QString, Kross::InterpreterInfo *> infos = Kross::Manager::self().interpreterInfos();
		for(QHash<QString, Kross::InterpreterInfo *>::ConstIterator it = infos.begin(), end = infos.end(); it != end; ++it) {
			QStringList intMimeTypes = it.value()->mimeTypes();
			for(QStringList::ConstIterator it = intMimeTypes.begin(), end = intMimeTypes.end(); it != end; ++it)
				mimeTypes << *it;
		}
		if(!mimeTypes.contains("application/javascript") && !mimeTypes.contains("text/javascript") && !mimeTypes.contains("application/x-javascript"))
			mimeTypes.prepend("application/javascript");
	}

	return mimeTypes;
}

void
ScriptsManager::addScript(const QUrl &sSU)
{
	QUrl srcScriptUrl = sSU;

	if(srcScriptUrl.isEmpty()) {
		QFileDialog fileDialog(m_dialog, i18n("Select Existing Script"), QString(), QString());
		fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
		fileDialog.setFileMode(QFileDialog::ExistingFile);
		fileDialog.setMimeTypeFilters(mimeTypes());
		fileDialog.setModal(true);

		if(fileDialog.exec() != QDialog::Accepted)
			return;

		srcScriptUrl = fileDialog.selectedUrls().first();
	}

	QString scriptName = QFileInfo(srcScriptUrl.fileName()).fileName();

	while(m_scripts.contains(scriptName)) {
		if(m_scripts.contains(scriptName)
			&& KMessageBox::questionYesNo(app()->mainWindow(),
				i18n("You must enter an unused name to continue.\nWould you like to enter another name?"),
				i18n("Name Already Used"), KStandardGuiItem::cont(), KStandardGuiItem::cancel()) != KMessageBox::Yes)
			return;

		TextInputDialog nameDlg(i18n("Rename Script"), i18n("Script name:"));
		if(nameDlg.exec() != QDialog::Accepted)
			return;
		scriptName = nameDlg.value();
	}

	QDir scriptPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
	if(!scriptPath.exists("scripts"))
		scriptPath.mkpath("scripts");
	scriptPath.cd("scripts");

	FileLoadHelper fileLoadHelper(srcScriptUrl);

	if(!fileLoadHelper.open()) {
		KMessageBox::sorry(app()->mainWindow(), i18n("There was an error opening the file <b>%1</b>.", srcScriptUrl.toString(QUrl::PreferLocalFile)));
		return;
	}

	QFile dest(scriptPath.absoluteFilePath(scriptName));
	if(!dest.open(QIODevice::WriteOnly | QIODevice::Truncate)
			|| dest.write(fileLoadHelper.file()->readAll()) == -1
			|| !dest.flush()) {
		KMessageBox::sorry(app()->mainWindow(), i18n("There was an error copying the file to <b>%1</b>.", dest.fileName()));
		return;
	}

	reloadScripts();
}

void
ScriptsManager::removeScript(const QString &sN)
{
	QString scriptName = sN.isEmpty() ? currentScriptName() : sN;
	if(scriptName.isEmpty() || !m_scripts.contains(scriptName)) {
		qWarning() << "unknown script specified";
		return;
	}

	if(KMessageBox::warningContinueCancel(app()->mainWindow(), i18n("Do you really want to send file <b>%1</b> to the trash?", scriptName), i18n("Move to Trash")) != KMessageBox::Continue)
		return;

	if(!FileTrasher(m_scripts[scriptName]).exec()) {
		KMessageBox::sorry(app()->mainWindow(), i18n("There was an error removing the file <b>%1</b>.", m_scripts[scriptName]));
		return;
	}

	reloadScripts();
}

void
ScriptsManager::editScript(const QString &sN)
{
	QString scriptName = sN.isEmpty() ? currentScriptName() : sN;
	if(scriptName.isEmpty() || !m_scripts.contains(scriptName)) {
		qWarning() << "unknown script specified";
		return;
	}

	const QUrl script(m_scripts[scriptName]);
#if KIO_VERSION >= ((5<<16)|(31<<8)|(0))
	if(!KRun::runUrl(script, "text/plain", app()->mainWindow(), KRun::RunFlags(0))) {
#else
	if(!KRun::runUrl(script, "text/plain", app()->mainWindow(), false, false)) {
#endif
		if(!QDesktopServices::openUrl(script))
			KMessageBox::sorry(app()->mainWindow(), i18n("Could not launch external editor.\n"));
	}
}

void
ScriptsManager::runScript(const QString &sN)
{
	if(!app()->subtitle()) {
		qWarning() << "attempt to run script without a working subtitle";
		return;
	}

	QString scriptName = sN;

	if(scriptName.isEmpty()) {
		scriptName = currentScriptName();
		if(scriptName.isEmpty())
			return;
	}

	if(!m_scripts.contains(scriptName)) {
		qWarning() << "unknown script file specified";
		return;
	}

	Kross::Action krossAction(0, "Kross::Action");

	Scripting::RangesModule *rangesModule = new Scripting::RangesModule;
	Scripting::StringsModule *stringsModule = new Scripting::StringsModule;
	Scripting::SubtitleModule *subtitleModule = new Scripting::SubtitleModule;
	Scripting::SubtitleLineModule *subtitleLineModule = new Scripting::SubtitleLineModule;
	Debug *debug = new Debug();

	krossAction.addObject(rangesModule, "ranges");
	krossAction.addObject(stringsModule, "strings");
	krossAction.addObject(subtitleModule, "subtitle");
	krossAction.addObject(subtitleLineModule, "subtitleline");
	krossAction.addObject(debug, "debug");

	krossAction.setFile(m_scripts[scriptName]);
	if(krossAction.interpreter().isEmpty() && scriptName.right(3) == QLatin1String(".js"))
		krossAction.setInterpreter("qtscript");
	// default javascript interpreter has weird (crash inducing) bugs
	else if(krossAction.interpreter() == QLatin1String("javascript"))
		krossAction.setInterpreter("qtscript");

	{
		// everything done by the script will be undoable in a single step
		SubtitleCompositeActionExecutor executor(*(app()->subtitle()), scriptName);
		// execute the script file
		krossAction.trigger();
	}

	delete rangesModule;
	delete stringsModule;
	delete subtitleModule;
	delete subtitleLineModule;
	delete debug;

	if(krossAction.hadError()) {
		if(krossAction.errorTrace().isNull())
			KMessageBox::error(app()->mainWindow(), krossAction.errorMessage(), i18n("Error Running Script"));
		else
			KMessageBox::detailedError(app()->mainWindow(), krossAction.errorMessage(), krossAction.errorTrace(), i18n("Error Running Script"));
	}
}

QMenu *
ScriptsManager::toolsMenu()
{
	static QMenu *toolsMenu = 0;

	if(!toolsMenu) {
		toolsMenu = app()->mainWindow()->findChild<QMenu *>("tools");
		if(!toolsMenu) {
			toolsMenu = app()->mainWindow()->menuBar()->addMenu(i18n("Tools"));
			toolsMenu->setObjectName("tools");
		}
		connect(toolsMenu, SIGNAL(triggered(QAction *)), this, SLOT(onToolsMenuActionTriggered(QAction *)));
	}

	return toolsMenu;
}

/*static*/ void
ScriptsManager::findAllFiles(QString directory, QStringList &fileList)
{
	QDir path(directory);
	QFileInfoList files = path.entryInfoList();
	foreach(QFileInfo file, files) {
		if(file.isDir()) {
			if(file.fileName().at(0) != '.')
				findAllFiles(file.absoluteFilePath(), fileList);
		} else {
			fileList.append(file.absoluteFilePath());
		}
	}
}

void
ScriptsManager::reloadScripts()
{
	QMenu *toolsMenu = ScriptsManager::toolsMenu();
	KActionCollection *actionCollection = app()->mainWindow()->actionCollection();
	UserActionManager *actionManager = UserActionManager::instance();

	QString selectedPath = scriptsView->model()->rowCount() && !currentScriptName().isEmpty() ? m_scripts[currentScriptName()] : QString();

	m_scripts.clear();
	toolsMenu->clear();
	toolsMenu->addAction(app()->action(ACT_SCRIPTS_MANAGER));
	toolsMenu->addSeparator();

	qDebug() << "KROSS interpreters:" << Kross::Manager::self().interpreters();
	QStringList scriptExtensions;
	foreach(const QString interpreter, Kross::Manager::self().interpreters()) {
		if(interpreter == QStringLiteral("qtscript"))
			scriptExtensions.append(QStringLiteral(".js"));
		else if(interpreter == QStringLiteral("ruby"))
			scriptExtensions.append(QStringLiteral(".rb"));
		else if(interpreter == QStringLiteral("python"))
			scriptExtensions.append(QStringLiteral(".py"));
	}

	QStringList scriptDirs = QStandardPaths::locateAll(QStandardPaths::AppDataLocation, "scripts", QStandardPaths::LocateDirectory);
	QStringList scriptNames;
	int index = 0, newCurrentIndex = -1;
	foreach(const QString &path, scriptDirs) {
		int pathLen = QDir(path).absolutePath().length() + 1;
		QStringList scriptPaths;
		findAllFiles(path, scriptPaths);
		foreach(const QString &path, scriptPaths) {
			QString name = path.mid(pathLen);
			if(m_scripts.contains(name))
				continue;

			scriptNames << name;

			m_scripts[name] = path;

			QString suffix = name.right(3);
			if(scriptExtensions.contains(suffix)) {
				QAction *scriptAction = toolsMenu->addAction(name);
				scriptAction->setObjectName(name);
				actionCollection->addAction(name, scriptAction);
				actionManager->addAction(scriptAction, UserAction::SubOpened | UserAction::FullScreenOff);
			}

			if(newCurrentIndex < 0 && path == selectedPath)
				newCurrentIndex = index;
			index++;
		}
	}
	scriptNames.sort();

	static_cast<InstalledScriptsModel *>(scriptsView->model())->setStringList(scriptNames);
	if(!scriptNames.isEmpty()) {
		QModelIndex currentIndex = scriptsView->model()->index(newCurrentIndex < 0 ? 0 : newCurrentIndex, 0);
		scriptsView->setCurrentIndex(currentIndex);
	}
}

void
ScriptsManager::onToolsMenuActionTriggered(QAction *triggeredAction)
{
	if(triggeredAction == app()->action(ACT_SCRIPTS_MANAGER))
		return;

	runScript(triggeredAction->objectName());
}

bool
ScriptsManager::eventFilter(QObject *object, QEvent *event)
{
	if(object == scriptsView && event->type() == QEvent::KeyPress && static_cast<QKeyEvent *>(event)->matches(QKeySequence::Delete)) {
		removeScript();
		return true; // eat event
	}

	return QObject::eventFilter(object, event);
}


