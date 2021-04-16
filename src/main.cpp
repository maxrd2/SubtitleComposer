/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2019 Mladen Milinkovic <max@smoothware.net>
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

#include "application.h"
#include "mainwindow.h"
#include "helpers/commondefs.h"
#include "videoplayer/backend/glrenderer.h"

#include <KAboutData>
#include <KLocalizedString>

#include <QFile>
#include <QDir>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QProcessEnvironment>
#include <QResource>
#include <QMimeDatabase>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "config-subtitlecomposer.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

using namespace SubtitleComposer;

static bool
mimeIsMedia(const QMimeType &mime)
{
	const static QStringList mimeList = {
		"video/webm", "video/x-msvideo", "video/mp2t", "application/x-matroska",
		"video/x-matroska", "video/mp4", "video/mpeg", "video/ogg"
	};
	for(const QString &m: mimeList) {
		if(mime.inherits(m))
			return true;
	}
	return false;
}

static bool
mimeIsSubtitle(const QMimeType &mime)
{
	const static QStringList mimeList = {
		"application/x-subrip", "text/x-ssa", "text/x-ass","text/x-microdvd",
		"text/x-mpsub", "text/x-subviewer", "text/x-mplsub", "text/x-tmplayer",
		"text/x-tmplayer+", "application/x-vobsub"
	};
	for(const QString &m: mimeList) {
		if(mime.inherits(m))
			return true;
	}
	return false;
}

void
handleCommandLine(SubtitleComposer::Application &app, KAboutData &aboutData)
{
	QCommandLineParser parser;
	aboutData.setupCommandLine(&parser);
	parser.setApplicationDescription(aboutData.shortDescription());

	QCommandLineOption subtitleOption("subtitle",
		i18n("The primary subtitle to be edited."),
		"file name");
	parser.addOption(subtitleOption);

	QCommandLineOption videoOption("video",
		i18n("The video to be subtitled."),
		"file name");
	parser.addOption(videoOption);

	QCommandLineOption translationOption("translation",
		i18n("The translation subtitle to be edited.\n"
			"If included, a second pane gets added to the right of the primary subtitle "
			"for side-by-side translation."),
		"file name");
	parser.addOption(translationOption);

	// parse command line
	parser.process(app);
	aboutData.processCommandLine(&parser);

	app.init();

	app.mainWindow()->show();

	QString fileSub;
	QString fileTrans;
	QString fileVideo;

	// load files by specific options
	if(parser.isSet(subtitleOption))
		fileSub = parser.value(subtitleOption);
	if(parser.isSet(videoOption))
		fileVideo = parser.value(videoOption);
	if(parser.isSet(translationOption))
		fileTrans = parser.value(translationOption);

	// load sub/video if positional param is provided
	const QStringList args = parser.positionalArguments();
	for(const QString &arg: args) {
		const QMimeType mime = QMimeDatabase().mimeTypeForFile(arg);
		if(mimeIsSubtitle(mime)) {
			// try to open primary subtitle if not already opened
			if(fileSub.isEmpty()) {
				fileSub = arg;
				continue;
			}
			// try to open translation if not already opened
			if(fileTrans.isEmpty()) {
				fileTrans = arg;
				continue;
			}
		} else if(mimeIsMedia(mime)) {
			// try to open video if not already opened
			if(fileVideo.isEmpty()) {
				fileVideo = arg;
				continue;
			}
		}
	}

	if(!fileSub.isEmpty())
		app.openSubtitle(System::urlFromPath(fileSub));
	else if(!fileTrans.isEmpty())
		app.newSubtitle();
	if(!fileTrans.isEmpty())
		app.openSubtitleTr(System::urlFromPath(fileTrans));
	if(!fileVideo.isEmpty())
		app.openVideo(System::urlFromPath(fileVideo));
}

int
main(int argc, char **argv)
{
	GLRenderer::setupProfile();

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 9, 100)
	av_register_all();
	avcodec_register_all();
#endif

	SubtitleComposer::Application app(argc, argv);

	// find custom icons outside kde
	QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << QDir(qApp->applicationDirPath())
		.absoluteFilePath(QDir(QStringLiteral(SC_INSTALL_BIN)).relativeFilePath(QStringLiteral(CUSTOM_ICON_INSTALL_PATH))));

	// force breeze theme outside kde environment
	if(QProcessEnvironment::systemEnvironment().value(QStringLiteral("XDG_CURRENT_DESKTOP")).toLower() != QLatin1String("kde"))
		QIcon::setThemeName("breeze");

#ifdef Q_OS_WIN
	const QStringList themes {"/icons/breeze/breeze-icons.rcc", "/icons/breeze-dark/breeze-icons-dark.rcc"};
	for(const QString theme : themes) {
		const QString themePath = QStandardPaths::locate(QStandardPaths::AppDataLocation, theme);
		if(!themePath.isEmpty()) {
			const QString iconSubdir = theme.left(theme.lastIndexOf('/'));
			if(QResource::registerResource(themePath, iconSubdir)) {
				if(QFileInfo::exists(QLatin1Char(':') + iconSubdir + QStringLiteral("/index.theme"))) {
					qDebug() << "Loaded icon theme:" << theme;
				} else {
					qWarning() << "No index.theme found in" << theme;
					QResource::unregisterResource(themePath, iconSubdir);
				}
			} else {
				qWarning() << "Invalid rcc file" << theme;
			}
		}
	}
#endif

	KLocalizedString::setApplicationDomain("subtitlecomposer");

	KAboutData aboutData(
		QStringLiteral("subtitlecomposer"),
		i18n("Subtitle Composer"),
		SUBTITLECOMPOSER_VERSION_STRING,
		i18n("A KDE subtitle editor."),
		KAboutLicense::GPL_V2,
		QStringLiteral("&copy; 2007-2017 Subtitle Composer project"),
		QString(), // Additional text
		QStringLiteral("https://invent.kde.org/kde/subtitlecomposer"),
		QStringLiteral("https://invent.kde.org/kde/subtitlecomposer/issues"));

	aboutData.addAuthor(i18n("Mladen Milinković"), i18n("Current Author & Maintainer"), "maxrd2@smoothware.net");
	aboutData.addAuthor(i18n("Sergio Pistone"), i18n("Original Author"), "sergio_pistone@yahoo.com.ar");

	aboutData.setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"), i18nc("EMAIL OF TRANSLATORS", "Your emails"));

	aboutData.addCredit("Martin Steghöfer", i18n("code contributions"));
	aboutData.addCredit("Marius Kittler", i18n("code contributions, Arch Linux packaging"));

	aboutData.addCredit(i18n("All people who have contributed and I have forgotten to mention"));

	// register about data
	KAboutData::setApplicationData(aboutData);

	// set app stuff from about data component name
	app.setApplicationName(aboutData.componentName());
	app.setOrganizationDomain(aboutData.organizationDomain());
	app.setApplicationVersion(aboutData.version());
	app.setApplicationDisplayName(aboutData.displayName());
	app.setWindowIcon(QIcon::fromTheme(aboutData.componentName()));

	handleCommandLine(app, aboutData);

	return app.exec();
}
