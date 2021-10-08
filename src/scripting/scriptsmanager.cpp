/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "config.h"

#include "scriptsmanager.h"
#include "scripting_rangesmodule.h"
#include "scripting_stringsmodule.h"
#include "scripting_subtitlemodule.h"
#include "scripting_subtitlelinemodule.h"

#include "application.h"
#include "actions/useraction.h"
#include "actions/useractionnames.h"
#include "dialogs/textinputdialog.h"
#include "helpers/common.h"
#include "helpers/fileloadhelper.h"
#include "helpers/filetrasher.h"
#include "gui/treeview/treeview.h"

#include <QStringListModel>
#include <QStandardPaths>
#include <QDialog>
#include <QFileDialog>
#include <QJSEngine>
#include <QMenuBar>
#include <QMenu>
#include <QDesktopServices>
#include <QKeyEvent>
#include <QStringBuilder>

#include <KMessageBox>
#include <KRun>
#include <KActionCollection>
#include <KLocalizedString>
#include <kio_version.h>
#if KIO_VERSION >= QT_VERSION_CHECK(5, 71, 0)
#include <KIO/OpenUrlJob>
#include <KIO/JobUiDelegate>
#endif

namespace SubtitleComposer {
class InstalledScriptsModel : public QStringListModel
{
public:
	InstalledScriptsModel(QObject *parent = 0) : QStringListModel(parent) {}

	virtual ~InstalledScriptsModel() {}

	QVariant headerData(int /*section */, Qt::Orientation orientation, int role) const override
	{
		if(role != Qt::DisplayRole || orientation == Qt::Vertical)
			return QVariant();
		return i18n("Installed Scripts");
	}

	Qt::ItemFlags flags(const QModelIndex &index) const override
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

	connect(btnCreate, &QPushButton::clicked, [this](){ createScript(); });
	connect(btnAdd, &QPushButton::clicked, [this](){ addScript(); });
	connect(btnRemove, &QPushButton::clicked, [this](){ removeScript(); });
	connect(btnEdit, &QPushButton::clicked, [this](){ editScript(); });
	connect(btnRun, &QPushButton::clicked, [this](){ runScript(); });
	connect(btnRefresh, &QAbstractButton::clicked, [this](){ reloadScripts(); });

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

	if(mimeTypes.isEmpty())
		mimeTypes.append("application/javascript");

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

		srcScriptUrl = fileDialog.selectedUrls().constFirst();
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

	const QUrl script = QUrl::fromLocalFile(m_scripts[scriptName]);
#ifdef SC_APPIMAGE
	{
#elif KIO_VERSION >= QT_VERSION_CHECK(5, 71, 0)
	KIO::OpenUrlJob *job = new KIO::OpenUrlJob(script);
	job->setUiDelegate(new KIO::JobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, app()->mainWindow()));
	if(!job->exec()) {
#elif KIO_VERSION >= QT_VERSION_CHECK(5, 31, 0)
	if(!KRun::runUrl(script, "text/plain", app()->mainWindow(), KRun::RunFlags())) {
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

	QJSEngine jse;
	jse.installExtensions(QJSEngine::ConsoleExtension);
	jse.globalObject().setProperty("ranges", jse.newQObject(new Scripting::RangesModule));
	jse.globalObject().setProperty("strings", jse.newQObject(new Scripting::StringsModule));
	jse.globalObject().setProperty("subtitle", jse.newQObject(new Scripting::SubtitleModule));
	jse.globalObject().setProperty("subtitleline", jse.newQObject(new Scripting::SubtitleLineModule));
	jse.globalObject().setProperty("debug", jse.newQObject(new Debug()));

	QString script;
	{
		QFile jsf(m_scripts[scriptName]);
		if(!jsf.open(QFile::ReadOnly | QFile::Text)) {
			KMessageBox::error(app()->mainWindow(), i18n("Error opening script %1.", m_scripts[scriptName]), i18n("Error Running Script"));
			return;
		}
		script = QTextStream(&jsf).readAll();
	}

	QJSValue res;
	{
		// everything done by the script will be undoable in a single step
		SubtitleCompositeActionExecutor executor(app()->subtitle(), scriptName);
		// execute the script file
		res = jse.evaluate(script, scriptName);
	}

	if(!res.isUndefined()) {
		if(res.isError()) {
			const QString details = i18n("Path: %1", m_scripts[scriptName]) % "\n"
				% res.property($("stack")).toString();
			KMessageBox::detailedError(app()->mainWindow(), res.toString(), details, i18n("Error Running Script"));
		} else {
			KMessageBox::error(app()->mainWindow(), res.toString(), i18n("Error Running Script"));
		}
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
		connect(toolsMenu, &QMenu::triggered, this, &ScriptsManager::onToolsMenuActionTriggered);
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

			if(name.right(3) == $(".js")) {
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


