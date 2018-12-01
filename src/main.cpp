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

#include "application.h"
#include "mainwindow.h"
#include "helpers/commondefs.h"

#include <KAboutData>
#include <KLocalizedString>

#include <QFile>
#include <QDir>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QProcessEnvironment>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

int
main(int argc, char **argv)
{
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

	KLocalizedString::setApplicationDomain("subtitlecomposer");

	KAboutData aboutData(
		"subtitlecomposer",
		i18n("Subtitle Composer"),
		"0.7.0",
		i18n("A KDE subtitle editor."),
		KAboutLicense::GPL,
		QStringLiteral("&copy; 2007-2017 Subtitle Composer project"),
		QString(), // Additional text
		QStringLiteral("https://github.com/maxrd2/subtitlecomposer"),
		QStringLiteral("https://github.com/maxrd2/subtitlecomposer/issues"));

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

	// Initialize command line args
	QCommandLineParser parser;
	aboutData.setupCommandLine(&parser);
	parser.setApplicationDescription(aboutData.shortDescription());
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addPositionalArgument("primary-url", i18n("Open location as primary subtitle"), "[primary-url]");
	parser.addPositionalArgument("translation-url", i18n("Open location as translation subtitle"), "[translation-url]");

	// do the command line parsing
	parser.process(app);

	// handle standard options
	aboutData.processCommandLine(&parser);

	app.init();

	app.mainWindow()->show();

	// load files
	const QStringList args = parser.positionalArguments();
	if(args.length() > 0)
		app.openSubtitle(System::urlFromPath(args[0]));
	if(args.length() > 1)
		app.openSubtitleTr(System::urlFromPath(args[1]));

	return app.exec();
}
