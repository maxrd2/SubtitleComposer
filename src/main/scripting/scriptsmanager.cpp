
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

#include "scriptsmanager.h"
#include "scripting_rangesmodule.h"
#include "scripting_stringsmodule.h"
#include "scripting_subtitlemodule.h"
#include "scripting_subtitlelinemodule.h"

#include "../application.h"
#include "../actions/useraction.h"
#include "../actions/useractionnames.h"
#include "../dialogs/inputdialog.h"
#include "../../common/fileloadhelper.h"
#include "../../common/filetrasher.h"
#include "../../widgets/treeview.h"

#include <QtCore/QProcess>
#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QGridLayout>
#include <QtGui/QKeyEvent>
#include <QtGui/QStringListModel>

#include <KAction>
#include <KActionCollection>
#include <KMenuBar>
#include <KDialog>
#include <KFileDialog>
#include <KPushButton>
#include <KSeparator>
#include <KMessageBox>
#include <KStandardDirs>
#include <KRun>

#include <kross/core/manager.h>
#include <kross/core/interpreter.h>
#include <kross/core/action.h>

namespace SubtitleComposer {
class InstalledScriptsModel : public QStringListModel
{
public:
	InstalledScriptsModel(QObject *parent = 0) : QStringListModel(parent) {}

	virtual ~InstalledScriptsModel() {}

	virtual QVariant headerData(int /*section */, Qt::Orientation orientation, int role) const
	{
		if(role != Qt::DisplayRole || orientation == Qt::Vertical)
			return QVariant();
		return i18n("Installed Scripts");
	}

	virtual Qt::ItemFlags flags(const QModelIndex &index) const
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
	kDebug() << message;
}

void
Debug::warning(const QString &message)
{
	KMessageBox::sorry(app()->mainWindow(), message, i18n("Warning"));
	kWarning() << message;
}

void
Debug::error(const QString &message)
{
	KMessageBox::error(app()->mainWindow(), message, i18n("Error"));
	kWarning() << message;
}

ScriptsManager::ScriptsManager(QObject *parent) :
	QObject(parent)
{
	// kDebug() << "KROSS interpreters:" << Kross::Manager::self().interpreters();

	KGlobal::dirs()->addResourceType("script", "appdata", "scripts");

	m_dialog = new KDialog(app()->mainWindow());

	m_dialog->setCaption(i18n("Scripts Manager"));
	m_dialog->setButtons(0);
	m_dialog->setMainWidget(new QWidget(m_dialog));

	QWidget *mainWidget = m_dialog->mainWidget();

	m_scriptsWidget = new TreeView(mainWidget);
	m_scriptsWidget->installEventFilter(this);
	m_scriptsWidget->setModel(new InstalledScriptsModel);

	m_scriptsWidget->setRootIsDecorated(false);
	m_scriptsWidget->setSortingEnabled(false);

	QWidget *buttonsWidget = new QWidget(mainWidget);

	KPushButton *createScriptButton = new KPushButton(buttonsWidget);
	createScriptButton->setText(i18n("Create..."));
	createScriptButton->setWhatsThis(i18n("Creates a new script file."));
	createScriptButton->setIcon(KIcon("document-new"));
	connect(createScriptButton, SIGNAL(clicked()), this, SLOT(createScript()));

	KPushButton *addScriptButton = new KPushButton(buttonsWidget);
	addScriptButton->setText(i18n("Add..."));
	addScriptButton->setWhatsThis(i18n("Copies an existing script from a specified location."));
	addScriptButton->setIcon(KIcon("document-open"));
	connect(addScriptButton, SIGNAL(clicked()), this, SLOT(addScript()));

	KPushButton *removeScriptButton = new KPushButton(buttonsWidget);
	removeScriptButton->setText(i18n("Remove"));
	removeScriptButton->setWhatsThis(i18n("Sends the selected script to the trash."));
	removeScriptButton->setIcon(KIcon("user-trash"));
	connect(removeScriptButton, SIGNAL(clicked()), this, SLOT(removeScript()));

	KPushButton *editScriptButton = new KPushButton(buttonsWidget);
	editScriptButton->setText(i18n("Edit"));
	editScriptButton->setWhatsThis(i18n("Opens the selected script with an external editor."));
	editScriptButton->setIcon(KIcon("document-edit"));
	connect(editScriptButton, SIGNAL(clicked()), this, SLOT(editScript()));

	m_runScriptButton = new KPushButton(buttonsWidget);
	m_runScriptButton->setEnabled(false);
	m_runScriptButton->setText(i18n("Run"));
	m_runScriptButton->setWhatsThis(i18n("Executes the selected script."));
	m_runScriptButton->setIcon(KIcon("media-playback-start"));
	connect(m_runScriptButton, SIGNAL(clicked()), this, SLOT(runScript()));

	KPushButton *reloadScriptsButton = new KPushButton(buttonsWidget);
	reloadScriptsButton->setText(i18n("Refresh"));
	reloadScriptsButton->setWhatsThis(i18n("Reloads the installed scripts list."));
	reloadScriptsButton->setIcon(KIcon("view-refresh"));
	connect(reloadScriptsButton, SIGNAL(clicked()), this, SLOT(reloadScripts()));

	QGridLayout *buttonsLayout = new QGridLayout(buttonsWidget);
	buttonsLayout->setContentsMargins(0, 0, 0, 0);
	buttonsLayout->addWidget(createScriptButton, 0, 0);
	buttonsLayout->addWidget(addScriptButton, 1, 0);
	buttonsLayout->addWidget(removeScriptButton, 2, 0);
	buttonsLayout->addWidget(editScriptButton, 3, 0);
	buttonsLayout->addWidget(new KSeparator(mainWidget), 4, 0);
	buttonsLayout->addWidget(m_runScriptButton, 5, 0);
	buttonsLayout->addWidget(new KSeparator(mainWidget), 6, 0);
	buttonsLayout->addWidget(reloadScriptsButton, 7, 0);
	buttonsLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding), 8, 0);

	QGridLayout *mainLayout = new QGridLayout(mainWidget);
	mainLayout->setSpacing(5);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->addWidget(m_scriptsWidget, 0, 0);
	mainLayout->addWidget(buttonsWidget, 0, 1);

	m_dialog->resize(350, 10);
}

ScriptsManager::~ScriptsManager()
{}

void
ScriptsManager::setSubtitle(Subtitle *subtitle)
{
	m_runScriptButton->setEnabled(subtitle != 0);
}

void
ScriptsManager::showDialog()
{
	m_dialog->show();
}

QString
ScriptsManager::currentScriptName() const
{
	QModelIndex currentIndex = m_scriptsWidget->currentIndex();
	return currentIndex.isValid() ? m_scriptsWidget->model()->data(currentIndex, Qt::DisplayRole).toString() : 0;
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
		if(m_scripts.contains(scriptName) && KMessageBox::questionYesNo(app()->mainWindow(), i18n("You must enter an unused name to continue.\nWould you like to enter another name?"), i18n("Name Already Used"), KStandardGuiItem::cont(), KStandardGuiItem::cancel()
																		) != KMessageBox::Yes)
			return;

		TextInputDialog nameDlg(i18n("Create New Script"), i18n("Script name:"));
		if(nameDlg.exec() != QDialog::Accepted)
			return;
		scriptName = nameDlg.value();
	}

	QString scriptPath = KStandardDirs::locateLocal("script", scriptName, true);

	QFile scriptFile(scriptPath);
	if(!scriptFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		KMessageBox::sorry(app()->mainWindow(), i18n("There was an error creating the file <b>%1</b>.", scriptPath)
						   );
		return;
	}

	QTextStream outputStream(&scriptFile);
	QString scriptExtension = QFileInfo(scriptName).suffix().toLower();
	if(scriptExtension == "rb")
		outputStream << "#!/usr/bin/env ruby";
	else if(scriptExtension == "py")
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
	}

	return mimeTypes;
}

void
ScriptsManager::addScript(const KUrl &sSU)
{
	KUrl srcScriptUrl = sSU;

	if(srcScriptUrl.isEmpty()) {
		KFileDialog fileDialog(KUrl(), QString(), m_dialog);
		fileDialog.setCaption(i18n("Select Existing Script"));
		fileDialog.setOperationMode(KFileDialog::Opening);
		fileDialog.setMimeFilter(mimeTypes());
		fileDialog.setMode(KFile::File | KFile::ExistingOnly);
		fileDialog.setModal(true);

		if(QDialog::Accepted != fileDialog.exec())
			return;

		srcScriptUrl = fileDialog.selectedUrl();
	}

	QString scriptName = QFileInfo(srcScriptUrl.fileName()).fileName();

	while(m_scripts.contains(scriptName)) {
		if(m_scripts.contains(scriptName) && KMessageBox::questionYesNo(app()->mainWindow(), i18n("You must enter an unused name to continue.\nWould you like to enter another name?"), i18n("Name Already Used"), KStandardGuiItem::cont(), KStandardGuiItem::cancel()
																		) != KMessageBox::Yes)
			return;

		TextInputDialog nameDlg(i18n("Rename Script"), i18n("Script name:"));
		if(nameDlg.exec() != QDialog::Accepted)
			return;
		scriptName = nameDlg.value();
	}

	QString scriptPath = KStandardDirs::locateLocal("script", scriptName, true);

	FileLoadHelper fileLoadHelper(srcScriptUrl);

	if(!fileLoadHelper.open()) {
		KMessageBox::sorry(app()->mainWindow(), i18n("There was an error opening the file <b>%1</b>.", srcScriptUrl.prettyUrl())
						   );
		return;
	}

	if(!fileLoadHelper.file()->copy(scriptPath)) {
		KMessageBox::sorry(app()->mainWindow(), i18n("There was an error copying the file to <b>%1</b>.", scriptPath)
						   );
		return;
	}

	reloadScripts();
}

void
ScriptsManager::removeScript(const QString &sN)
{
	QString scriptName = sN.isEmpty() ? currentScriptName() : sN;
	if(scriptName.isEmpty() || !m_scripts.contains(scriptName)) {
		kWarning() << "unknow script specified";
		return;
	}

	if(KMessageBox::warningContinueCancel(app()->mainWindow(), i18n("Do you really want to send file <b>%1</b> to the trash?", scriptName), i18n("Move to Trash")
										  ) != KMessageBox::Continue)
		return;

	if(!FileTrasher(m_scripts[scriptName]).exec()) {
		KMessageBox::sorry(app()->mainWindow(), i18n("There was an error removing the file <b>%1</b>.", m_scripts[scriptName])
						   );
		return;
	}

	reloadScripts();
}

void
ScriptsManager::editScript(const QString &sN)
{
	QString scriptName = sN.isEmpty() ? currentScriptName() : sN;
	if(scriptName.isEmpty() || !m_scripts.contains(scriptName)) {
		kWarning() << "unknow script specified";
		return;
	}

	if(!KRun::runUrl(KUrl(m_scripts[scriptName]), "text/plain", app()->mainWindow(), false, false))
		KMessageBox::sorry(app()->mainWindow(), i18n("Could not launch external editor.\n"));
}

void
ScriptsManager::runScript(const QString &sN)
{
	if(!app()->subtitle()) {
		kWarning() << "attempt to run script without a working subtitle";
		return;
	}

	QString scriptName = sN;

	if(scriptName.isEmpty()) {
		scriptName = currentScriptName();
		if(scriptName.isEmpty())
			return;
	}

	if(!m_scripts.contains(scriptName)) {
		kWarning() << "unknow script file specified";
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
	// default javascript interpreter has weird (crash inducing) bugs
	if(krossAction.interpreter() == "javascript")
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

void
ScriptsManager::reloadScripts()
{
	QMenu *toolsMenu = ScriptsManager::toolsMenu();
	KActionCollection *actionCollection = app()->mainWindow()->actionCollection();
	UserActionManager *actionManager = UserActionManager::instance();

	QString selectedPath = m_scriptsWidget->model()->rowCount() && !currentScriptName().isEmpty() ? m_scripts[currentScriptName()] : QString();

	m_scripts.clear();                      // deletes all actions
	toolsMenu->clear();
	toolsMenu->addAction(app()->action(ACT_SCRIPTS_MANAGER));
	toolsMenu->addSeparator();

	QStringList scriptNames;
	QStringList scriptPaths = KGlobal::dirs()->findAllResources("script");
	scriptPaths.sort();
	int index = 0, newCurrentIndex = -1;
	for(QStringList::ConstIterator it = scriptPaths.begin(), end = scriptPaths.end(); it != end; ++it) {
		QString scriptName = QFileInfo(*it).fileName();
		if(!m_scripts.contains(scriptName)) {
			scriptNames << scriptName;
			if(newCurrentIndex < 0 && *it == selectedPath)
				newCurrentIndex = index;
			m_scripts[scriptName] = *it;

			QAction *scriptAction = toolsMenu->addAction(scriptName);
			scriptAction->setObjectName(scriptName);
			actionCollection->addAction(scriptName, scriptAction);
			actionManager->addAction(scriptAction, UserAction::SubOpened | UserAction::FullScreenOff);

			index++;
		}
	}

	static_cast<InstalledScriptsModel *>(m_scriptsWidget->model())->setStringList(scriptNames);
	if(!scriptNames.isEmpty()) {
		QModelIndex currentIndex = m_scriptsWidget->model()->index(newCurrentIndex < 0 ? 0 : newCurrentIndex, 0);
		m_scriptsWidget->setCurrentIndex(currentIndex);
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
	if(object == m_scriptsWidget) {
		if(event->type() == QEvent::KeyPress) {
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
			if(keyEvent->matches(QKeySequence::Delete)) {
				removeScript();
				return true;    // eat event
			}
		}
	}

	return QObject::eventFilter(object, event);
}

#include "scriptsmanager.moc"
