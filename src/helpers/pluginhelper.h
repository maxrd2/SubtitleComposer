/*
 * SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef PLUGINHELPER_H
#define PLUGINHELPER_H

#include "config.h"

#include <QApplication>
#include <QDebug>
#include <QPluginLoader>
#include <QDir>
#include <QFile>
#include <QVariant>
#include <QStringList>

namespace SubtitleComposer {
template <class C, class T>
class PluginHelper
{
public:
	PluginHelper(C *container) : m_cont(container) {}

	void loadAll(const QString &buildLocation);
//	void unload(T *plugin);
	T * pluginLoad(const QString &filename);
	void pluginAdd(T *plugin);

private:
	C *m_cont;
};

template <class C, class T> void
PluginHelper<C, T>::loadAll(const QString &buildLocation)
{
	const QDir appDir(qApp->applicationDirPath());
	const QDir buildPluginPath(appDir.absoluteFilePath(buildLocation));
	if(buildPluginPath.exists()) {
		// if application is launched from build directory it must load plugins from build directory
		const QStringList subdirs = buildPluginPath.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
		for(const QString &subdir: subdirs) {
			const QDir path(buildPluginPath.filePath(subdir));
			const QStringList libs = path.entryList(QDir::Files);
			for(const QString &lib: libs) {
				if(QLibrary::isLibrary(lib))
					pluginLoad(path.filePath(lib));
			}
		}
	} else {
		const QDir pluginsDir(appDir.absoluteFilePath(QDir(QStringLiteral(SC_INSTALL_BIN)).relativeFilePath(QStringLiteral(SC_INSTALL_PLUGIN))));
		foreach(const QString pluginFile, pluginsDir.entryList(QDir::Files, QDir::Name)) {
			if(QLibrary::isLibrary(pluginFile))
				pluginLoad(pluginsDir.filePath(pluginFile));
		}
	}
}

template <class C, class T> T *
PluginHelper<C, T>::pluginLoad(const QString &filename)
{
	QPluginLoader *loader = new QPluginLoader(filename);
	QObject *pluginLib = loader->instance();
	Q_ASSERT(pluginLib->thread() == QApplication::instance()->thread());
	if(!pluginLib)
		return nullptr;

	// make sure library doesn't get unloaded and plugin gets destroyed from non-gui thread
	loader->setParent(QApplication::instance());

	pluginLib->setProperty("pluginPath", loader->fileName());
	T *plugin = qobject_cast<T *>(pluginLib);
	if(!plugin)
		return nullptr;

	const char *className = T::staticMetaObject.className();
	if(strlen(className) > 18 && className[17] == ':') className += 18;
	qInfo() << "Loaded" << className << plugin->name() << "from" << loader->fileName();

	pluginAdd(plugin);

	return plugin;
}

template <class C, class T> void
PluginHelper<C, T>::pluginAdd(T *plugin)
{
	// Plugin will be deleted with container
	plugin->setParent(m_cont);
	Q_ASSERT(plugin->thread() == QApplication::instance()->thread());

	if(m_cont->m_plugins.contains(plugin->name())) {
		qCritical() << "Attempted to insert duplicate SpeechProcessor plugin" << plugin->name();
		return;
	}

	m_cont->m_plugins[plugin->name()] = plugin;
}

//template <class C, class T> void
//PluginHelper<C, T>::unload(T *plugin)
//{
//	const QString &filename = plugin->property("pluginPath").toString();
//	qDebug() << "plugin" << plugin->staticMetaObject.className() << "unload" << plugin->name();
//	if(filename.isEmpty()) {
//		delete plugin;
//		return;
//	}
//	QPluginLoader loader(filename);
//	Q_ASSERT(loader.instance() == plugin);
//	delete plugin;
//	if(!loader.unload())
//		qWarning() << "Failed unloading plugin" << filename << "-" << loader.errorString();
//}

} // namespace

#endif // PLUGINHELPER_H
