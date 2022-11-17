/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "scriptsmanager.h"

#include "appglobal.h"
#include "application.h"
#include "actions/useraction.h"
#include "actions/useractionnames.h"
#include "dialogs/textinputdialog.h"
#include "helpers/common.h"
#include "helpers/fileloadhelper.h"
#include "helpers/filetrasher.h"
#include "scripting/scripting_rangesmodule.h"
#include "scripting/scripting_stringsmodule.h"
#include "scripting/scripting_subtitlemodule.h"
#include "scripting/scripting_subtitlelinemodule.h"

#include <QAbstractItemModel>
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
#if KIO_VERSION >= QT_VERSION_CHECK(5, 98, 0)
#include <KIO/JobUiDelegateFactory>
#endif
#include <kwidgetsaddons_version.h>

inline static const QDir &
userScriptDir()
{
	static const QDir *dir = nullptr;
	if(dir == nullptr) {
		const QString userScriptDirName = $("scripts");
		static QDir d(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
		if(!d.exists(userScriptDirName))
			d.mkpath(userScriptDirName);
		d.cd(userScriptDirName);
		dir = &d;
	}
	return *dir;
}

namespace SubtitleComposer {
class SCScript
{
	friend class InstalledScriptsModel;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	template<class> friend struct QtPrivate::QGenericArrayOps;
	template<typename iterator, typename N>
	friend void QtPrivate::q_relocate_overlap_n_left_move(iterator first, N n, iterator d_first);
	template<typename> friend class QList;
#endif

	SCScript(const QString &path, const QString &name)
		: m_name(name),
		  m_path(path),
		  m_isScript(name.right(3) == $(".js")),
		  m_title(name)
	{
		if(m_isScript)
			initScript();
		else
			m_category = i18n("Non-Script Files");
	}

	void initScript()
	{
		const QString script = content();

		QRegularExpressionMatch m;
		staticRE$(reCat, "@category\\s+(.+)\\s*$", REm);
		if((m = reCat.match(script)).hasMatch())
			m_category = m.captured(1);
		staticRE$(reName, "@name\\s+(.+)\\s*$", REm);
		if((m = reName.match(script)).hasMatch())
			m_title = m.captured(1);
		staticRE$(reVer, "@version\\s+(.+)\\s*$", REm);
		if((m = reVer.match(script)).hasMatch())
			m_version = m.captured(1);
		staticRE$(reSummary, "@summary\\s+(.+)\\s*$", REm);
		if((m = reSummary.match(script)).hasMatch())
			m_description = m.captured(1);
		staticRE$(reAuthor, "@author\\s+(.+)\\s*$", REm);
		if((m = reAuthor.match(script)).hasMatch())
			m_author = m.captured(1);
	}

	friend QVector<SCScript>;
	SCScript() noexcept {}
	SCScript(const SCScript &) noexcept = default;
	SCScript & operator=(const SCScript &) noexcept = default;
	SCScript(SCScript &&) noexcept = default;
	SCScript & operator=(SCScript &&) noexcept = default;

public:
	QString content() const
	{
		QFile jsf(m_path);
		if(!jsf.open(QFile::ReadOnly | QFile::Text))
			return QString();
		return QTextStream(&jsf).readAll();
	}

	inline bool isScript() const { return m_isScript; }

	inline const QString & name() const { return m_name; }
	inline const QString & path() const { return m_path; }

	inline const QString & title() const { return m_title; }
	inline const QString & version() const { return m_version; }
	inline const QString & category() const { return m_category; }
	inline const QString & description() const { return m_description; }
	inline const QString & author() const { return m_author; }

	inline int compare(const SCScript &other, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
	{ return m_title.compare(other.m_title, cs); }

private:
	QString m_name;
	QString m_path;

	bool m_isScript;
	QString m_title;
	QString m_version;
	QString m_category;
	QString m_description;
	QString m_author;
};

class InstalledScriptsModel : public QAbstractItemModel
{
	Q_OBJECT

	enum { Title, Version, Author, Path, ColumnCount };

public:
	InstalledScriptsModel(QObject *parent = 0) : QAbstractItemModel(parent) {}
	~InstalledScriptsModel() {}

	QVariant headerData(int section, Qt::Orientation orientation, int role) const override
	{
		if(role != Qt::DisplayRole || orientation == Qt::Vertical)
			return QVariant();
		switch(section) {
		case Title: return i18n("Title");
		case Version: return i18n("Version");
		case Author: return i18n("Author");
		case Path: return i18n("Path");
		default: return QString();
		}
	}

	Qt::ItemFlags flags(const QModelIndex &index) const override
	{
		if(index.internalId() > 0)
			return Qt::ItemNeverHasChildren | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
		return Qt::ItemIsEnabled;
	}

	int childIndex(const QString &categoryName, int row) const
	{
		for(int i = 0, si = 0; i < m_scripts.size(); i++) {
			const SCScript &s = m_scripts.at(i);
			if(s.category() != categoryName)
				continue;
			if(si == row)
				return i;
			si++;
		}
		Q_ASSERT(false);
		return -1;
	}

	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override
	{
		if(!hasIndex(row, column, parent))
			return QModelIndex();

		if(!parent.isValid()) {
			// category
			if(row < m_categories.size())
				return createIndex(row, column, quintptr(0ULL));
			// script (root)
			return createIndex(row, column, childIndex(QString(), row - m_categories.size()) + 1);
		} else {
			// script (child)
			Q_ASSERT(!parent.parent().isValid());
			Q_ASSERT(parent.row() < m_categories.size());
			const QString catName = m_categories.at(parent.row());
			return createIndex(row, column, childIndex(catName, row) + 1);
		}
	}

	QModelIndex parent(const QModelIndex &child) const override
	{
		if(!child.isValid() || child.internalId() == 0)
			return QModelIndex();

		// script
		Q_ASSERT(int(child.internalId()) <= m_scripts.size());
		const SCScript &s = m_scripts.at(child.internalId() - 1);
		const int i = m_categories.indexOf(s.category());
		if(i >= 0) // script (child)
			return createIndex(i, 0, quintptr(0ULL));
		return QModelIndex(); // script (root)
	}

	int scriptCount(const QString &categoryName) const
	{
		int n = 0;
		for(int i = 0; i < m_scripts.size(); i++) {
			if(m_scripts.at(i).category() == categoryName)
				n++;
		}
		return n;
	}

	int rowCount(const QModelIndex &parent = QModelIndex()) const override
	{
		if(!parent.isValid())
			return m_categories.size() + scriptCount(QString());
		if(parent.row() >= m_categories.size())
			return 0;
		return scriptCount(m_categories.at(parent.row()));
	}

	int columnCount(const QModelIndex &parent = QModelIndex()) const override
	{
		Q_UNUSED(parent);
		return ColumnCount;
	}

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
	{
		if(index.internalId() > 0) {
			if(role == Qt::DisplayRole) {
				switch(index.column()) {
				case Title: return m_scripts.at(index.internalId() - 1).title();
				case Version: return m_scripts.at(index.internalId() - 1).version();
				case Author: return m_scripts.at(index.internalId() - 1).author();
				case Path: return m_scripts.at(index.internalId() - 1).path();
				}
			} else if(role == Qt::ToolTipRole) {
				if(index.column() == Path)
					return m_scripts.at(index.internalId() - 1).path();
				return m_scripts.at(index.internalId() - 1).description();
			}
		} else {
			if(role == Qt::DisplayRole && index.column() == Title) {
				const QString title = m_categories.at(index.row());
				return title.isEmpty() ? $("Misc") : title;
			}
		}
		return QVariant();

	}

	const SCScript * findByFilename(const QString &name) const
	{
		if(name.isEmpty())
			return nullptr;
		for(auto it = m_scripts.begin(); it != m_scripts.end(); ++it) {
			if(it->name() == name)
				return &(*it);
		}
		return nullptr;
	}

	const SCScript * findByTitle(const QString &title) const
	{
		if(title.isEmpty())
			return nullptr;
		for(auto it = m_scripts.begin(); it != m_scripts.end(); ++it) {
			if(it->title() == title)
				return &(*it);
		}
		return nullptr;
	}

	void removeAll()
	{
		beginRemoveRows(QModelIndex(), 0, m_categories.size());
		m_categories.clear();
		m_scripts.clear();
		endRemoveRows();
	}

	inline const SCScript * at(int index) const { return &m_scripts.at(index); }

	template<class T>
	const T * insertSorted(QVector<T> *vec, T &&el)
	{
		for(int i = 0; i < vec->size(); i++) {
			if(vec->at(i).compare(el, Qt::CaseInsensitive) >= 0) {
				vec->insert(i, std::move(el));
				return &vec->at(i);
			}
		}
		vec->push_back(std::move(el));
		return &vec->last();
	}

	const SCScript * add(const QString &path, int prefixLen)
	{
		const QString name = path.mid(prefixLen);
		if(findByFilename(name))
			return nullptr;

		const SCScript *script = insertSorted(&m_scripts, SCScript(path, name));
		const QString catTitle = script->category();

		int catIndex = m_categories.indexOf(catTitle);
		if(catIndex < 0 && !catTitle.isEmpty()) {
			catIndex = m_categories.size();
			beginInsertRows(QModelIndex(), catIndex, catIndex + 1);
			insertSorted(&m_categories, QString(catTitle));
			endInsertRows();
		}

		const int n = m_scripts.size();
		beginInsertRows(catTitle.isEmpty() ? QModelIndex() : createIndex(catIndex, 0, quintptr(0ULL)), n - 1, n + 1);
		endInsertRows();
		return script;
	}

private:
	QVector<SCScript> m_scripts;
	QVector<QString> m_categories;
};

class Debug : public QObject
{
	Q_OBJECT

public:
	Debug() {}
	~Debug() {}

public slots:
	void information(const QString &message)
	{
		KMessageBox::information(app()->mainWindow(), message, i18n("Information"));
		qDebug() << message;
	}

	void warning(const QString &message)
	{
		KMessageBox::error(app()->mainWindow(), message, i18n("Warning"));
		qWarning() << message;
	}

	void error(const QString &message)
	{
		KMessageBox::error(app()->mainWindow(), message, i18n("Error"));
		qWarning() << message;
	}
};
}

using namespace SubtitleComposer;

ScriptsManager::ScriptsManager(QObject *parent)
	: QObject(parent)
{
	m_dialog = new QDialog(app()->mainWindow());
	setupUi(m_dialog);

	scriptsView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	scriptsView->installEventFilter(this);
	if(QItemSelectionModel *m = scriptsView->selectionModel())
		m->deleteLater();
	scriptsView->setModel(new InstalledScriptsModel(scriptsView));
	scriptsView->setSortingEnabled(false);
	scriptsView->expandAll();
	connect(scriptsView, &QTreeView::doubleClicked, this, [&](){ editScript(); });
	connect(scriptsView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, [&](){
		const SCScript *s = currentScript();
		const bool isScriptSelected = s && s->isScript();
		const bool isFileWritable = s && QFileInfo(s->path()).isWritable();
		btnRemove->setEnabled(isFileWritable);
		if(isFileWritable) {
			btnEdit->setText(i18n("Edit"));
			btnEdit->setIcon(QIcon::fromTheme($("document-edit")));
		} else {
			btnEdit->setText(i18n("View"));
			btnEdit->setIcon(QIcon::fromTheme($("document-open")));
		}
		btnEdit->setEnabled(isScriptSelected);
		btnRun->setEnabled(isScriptSelected);
	});

	connect(btnCreate, &QPushButton::clicked, this, [&](){ createScript(); });
	connect(btnAdd, &QPushButton::clicked, this, [&](){ addScript(); });
	connect(btnRemove, &QPushButton::clicked, this, [&](){ removeScript(); });
	connect(btnEdit, &QPushButton::clicked, this, [&](){ editScript(); });
	connect(btnRun, &QPushButton::clicked, this, [&](){ runScript(); });
	connect(btnRefresh, &QAbstractButton::clicked, this, [&](){ reloadScripts(); });
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

const SCScript *
ScriptsManager::findScript(const QString filename) const
{
	const SCScript *s = filename.isEmpty() ? nullptr : static_cast<InstalledScriptsModel *>(scriptsView->model())->findByFilename(filename);
	return s ?: currentScript();
}

const SCScript *
ScriptsManager::currentScript() const
{
	const QModelIndex ci = scriptsView->currentIndex();
	if(ci.isValid() && ci.internalId() > 0)
		return static_cast<InstalledScriptsModel *>(scriptsView->model())->at(ci.internalId() - 1);
	return nullptr;
}

#if KWIDGETSADDONS_VERSION < QT_VERSION_CHECK(5, 100, 0)
#define questionTwoActions questionYesNo
#define PrimaryAction Yes
#endif

void
ScriptsManager::createScript(const QString &sN)
{
	QString scriptName = sN;

	InstalledScriptsModel *model = static_cast<InstalledScriptsModel *>(scriptsView->model());

	const SCScript *script = nullptr;
	while(scriptName.isEmpty() || (script = model->findByTitle(scriptName))) {
		if(script) {
			QFileInfo fp(script->path());
			if(!fp.canonicalFilePath().startsWith(userScriptDir().canonicalPath())) {
				// a system script can be overridden by user
				scriptName = script->name();
				break;
			}
			if(KMessageBox::questionTwoActions(app()->mainWindow(),
					i18n("You must enter an unused name to continue.\nWould you like to enter another name?"),
					i18n("Name Already Used"),
					KStandardGuiItem::cont(), KStandardGuiItem::cancel()) != KMessageBox::PrimaryAction)
				return;
		}

		TextInputDialog nameDlg(i18n("Create New Script"), i18n("Script name:"));
		if(nameDlg.exec() != QDialog::Accepted)
			return;
		scriptName = nameDlg.value();
	}

	if(!scriptName.endsWith($(".js")))
		scriptName.append($(".js"));

	QFile scriptFile(userScriptDir().absoluteFilePath(scriptName));
	if(!scriptFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		KMessageBox::error(app()->mainWindow(), i18n("There was an error creating the file <b>%1</b>.", userScriptDir().absoluteFilePath(scriptName)));
		return;
	}

	QTextStream outputStream(&scriptFile);
	outputStream << "/*\n"
		"\t@name " << scriptName.chopped(3) << " Title\n"
		"\t@version 1.0\n"
		"\t@summary " << scriptName << " summary/short desription.\n"
		"\t@author Author's Name\n"
		"*/\n";

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

	InstalledScriptsModel *model = static_cast<InstalledScriptsModel *>(scriptsView->model());
	while(model->findByFilename(scriptName)) {
		if(KMessageBox::questionTwoActions(app()->mainWindow(),
				i18n("You must enter an unused name to continue.\nWould you like to enter another name?"),
				i18n("Name Already Used"),
				KStandardGuiItem::cont(), KStandardGuiItem::cancel()) != KMessageBox::PrimaryAction)
			return;

		TextInputDialog nameDlg(i18n("Rename Script"), i18n("Script name:"));
		if(nameDlg.exec() != QDialog::Accepted)
			return;
		scriptName = nameDlg.value();
	}

	FileLoadHelper fileLoadHelper(srcScriptUrl);

	if(!fileLoadHelper.open()) {
		KMessageBox::error(app()->mainWindow(), i18n("There was an error opening the file <b>%1</b>.", srcScriptUrl.toString(QUrl::PreferLocalFile)));
		return;
	}

	QFile dest(userScriptDir().absoluteFilePath(scriptName));
	if(!dest.open(QIODevice::WriteOnly | QIODevice::Truncate)
			|| dest.write(fileLoadHelper.file()->readAll()) == -1
			|| !dest.flush()) {
		KMessageBox::error(app()->mainWindow(), i18n("There was an error copying the file to <b>%1</b>.", dest.fileName()));
		return;
	}

	reloadScripts();
}

void
ScriptsManager::removeScript(const QString &sN)
{
	const SCScript *script = findScript(sN);
	if(!script)
		return;

	if(KMessageBox::warningContinueCancel(app()->mainWindow(),
			i18n("Do you really want to send file <b>%1</b> to the trash?", script->path()), i18n("Move to Trash")) != KMessageBox::Continue)
		return;

	if(!FileTrasher(script->path()).exec()) {
		KMessageBox::error(app()->mainWindow(), i18n("There was an error removing the file <b>%1</b>.", script->path()));
		return;
	}

	reloadScripts();
}

void
ScriptsManager::editScript(const QString &sN)
{
	const SCScript *script = findScript(sN);
	if(!script) {
		qWarning() << "unknown script specified";
		return;
	}

	const QUrl scriptUrl = QUrl::fromLocalFile(script->path());
#ifdef SC_APPIMAGE
	{
#elif KIO_VERSION >= QT_VERSION_CHECK(5, 98, 0)
	KIO::OpenUrlJob *job = new KIO::OpenUrlJob(scriptUrl);
	job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, app()->mainWindow()));
	if(!job->exec()) {
#elif KIO_VERSION >= QT_VERSION_CHECK(5, 71, 0)
	KIO::OpenUrlJob *job = new KIO::OpenUrlJob(scriptUrl);
	job->setUiDelegate(new KIO::JobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, app()->mainWindow()));
	if(!job->exec()) {
#elif KIO_VERSION >= QT_VERSION_CHECK(5, 31, 0)
	if(!KRun::runUrl(scriptUrl, "text/plain", app()->mainWindow(), KRun::RunFlags())) {
#else
	if(!KRun::runUrl(scriptUrl, "text/plain", app()->mainWindow(), false, false)) {
#endif
		if(!QDesktopServices::openUrl(scriptUrl))
			KMessageBox::error(app()->mainWindow(), i18n("Could not launch external editor.\n"));
	}
}

void
ScriptsManager::runScript(const QString &sN)
{
	const SCScript *script = findScript(sN);
	if(!script || !script->isScript())
		return;

	QJSEngine jse;
	jse.installExtensions(QJSEngine::ConsoleExtension);
	jse.globalObject().setProperty("ranges", jse.newQObject(new Scripting::RangesModule));
	jse.globalObject().setProperty("strings", jse.newQObject(new Scripting::StringsModule));
	jse.globalObject().setProperty("subtitle", jse.newQObject(new Scripting::SubtitleModule));
	jse.globalObject().setProperty("subtitleline", jse.newQObject(new Scripting::SubtitleLineModule));
	jse.globalObject().setProperty("debug", jse.newQObject(new Debug()));

	QString scriptData = script->content();
	if(scriptData.isNull()) {
		KMessageBox::error(app()->mainWindow(), i18n("Error opening script %1.", script->path()), i18n("Error Running Script"));
		return;
	}

	QJSValue res;
	{
		// everything done by the script will be undoable in a single step
		SubtitleCompositeActionExecutor executor(appSubtitle(), script->title());
		res = jse.evaluate(scriptData, script->name());
	}

	if(!res.isUndefined()) {
		if(res.isError()) {
			const QString details = i18n("Path: %1", script->path()) % "\n"
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

void
ScriptsManager::findAllFiles(QString directory, QStringList &fileList)
{
	QDir path(directory);
	const QFileInfoList files = path.entryInfoList();
	for(const QFileInfo &file: files) {
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

	InstalledScriptsModel *model = static_cast<InstalledScriptsModel *>(scriptsView->model());

	toolsMenu->clear();
	toolsMenu->addAction(app()->action(ACT_SCRIPTS_MANAGER));
	toolsMenu->addSeparator();

	QMap<QString, QMenu *> categoryMenus;

	model->removeAll();

	// make sure userScriptDir is first on the list so it will override system scripts
	const QString userDir = userScriptDir().absolutePath();
	QStringList scriptDirs = QStandardPaths::locateAll(QStandardPaths::AppDataLocation, userScriptDir().dirName(), QStandardPaths::LocateDirectory);
	int pos = scriptDirs.indexOf(userDir);
	if(pos == -1)
		scriptDirs.prepend(userDir);
	else if(pos > 0)
		scriptDirs.move(pos, 0);

	foreach(const QString &path, scriptDirs) {
		const int pathLen = QDir(path).absolutePath().length() + 1;
		QStringList scriptPaths;
		findAllFiles(path, scriptPaths);
		foreach(const QString &path, scriptPaths) {
			const SCScript *script = model->add(path, pathLen);
			if(!script)
				continue;

			if(script->isScript()) {
				QMenu *parentMenu = toolsMenu;
				if(!script->category().isEmpty()) {
					auto it = categoryMenus.constFind(script->category());
					if(it != categoryMenus.cend()) {
						parentMenu = it.value();
					} else {
						parentMenu = new QMenu(script->category(), toolsMenu);
						categoryMenus[script->category()] = parentMenu;
					}
				}

				QAction *scriptAction = parentMenu->addAction(script->title());
				scriptAction->setObjectName(script->name());
				if(!script->description().isEmpty())
					scriptAction->setStatusTip(script->description());
				actionCollection->addAction(script->name(), scriptAction);
				actionManager->addAction(scriptAction, UserAction::SubOpened | UserAction::FullScreenOff);
			}
		}
	}

	for(auto it = categoryMenus.cbegin(); it != categoryMenus.cend(); ++it)
		toolsMenu->addMenu(it.value());
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

#include "scriptsmanager.moc"
