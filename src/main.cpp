/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2017 Mladen Milinkovic <max@smoothware.net>
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

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#ifdef HAVE_CONFIG_H
# include "config.h"
# if defined HAVE_XINE && !defined HAVE_XCB
#  include <X11/Xlib.h>
# endif
#endif

int
main(int argc, char **argv)
{
#if defined HAVE_XINE && !defined HAVE_XCB
	XInitThreads(); // needed for some VideoPlayer Backends (Xine)
#endif

	av_register_all();
	avcodec_register_all();

	SubtitleComposer::Application app(argc, argv);

	// hack(?) to find custom icons outside kde
	QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << QStringLiteral(CUSTOM_ICON_INSTALL_PATH));

	KLocalizedString::setApplicationDomain("subtitlecomposer");

	KAboutData aboutData(
		"subtitlecomposer",
		i18n("Subtitle Composer"),
		"0.6.6",
		i18n("A KDE subtitle editor."),
		KAboutLicense::GPL,
		QStringLiteral("&copy; 2007-2017 Subtitle Composer project"),
		QString(), // Additional text
		QStringLiteral("https://github.com/maxrd2/subtitlecomposer"),
		QStringLiteral("maxrd2@smoothware.net"));

	aboutData.addAuthor(i18n("Mladen Milinković"), i18n("Current Author & Maintainer"), "maxrd2@smoothware.net");
	aboutData.addAuthor(i18n("Sergio Pistone"), i18n("Original Author"), "sergio_pistone@yahoo.com.ar");

	aboutData.setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"), i18nc("EMAIL OF TRANSLATORS", "Your emails"));

	aboutData.addCredit("Goran Vidovic (gogo)", i18n("Croatian Translator"));
	aboutData.addCredit("Petar Toushkov", i18n("Bulgarian Translator"));
	aboutData.addCredit("Petr Gadula (Goliash)", i18n("Translator"));
	aboutData.addCredit("Thomas Gastine", i18n("Translator"));
	aboutData.addCredit("Panagiotis Papadopoulos", i18n("Translator"));
	aboutData.addCredit("Alessandro Polverini", i18n("Translator"));
	aboutData.addCredit("Tomasz Argasiński", i18n("Translator"));
	aboutData.addCredit("Marcio P. Moraes", i18n("Translator"));
	aboutData.addCredit("Alexander Antsev", i18n("Translator"));
	aboutData.addCredit("Slobodan Simic", i18n("Translator"));
	aboutData.addCredit("Yuri Chornoivan", i18n("Translator"));
	aboutData.addCredit("Alexandros Perdikomatis", i18n("Translator"));
	aboutData.addCredit("Barcza Károly", i18n("Translator"));
	aboutData.addCredit("Martin Steghöfer", i18n("code contributions"));
	aboutData.addCredit("Marius Kittler", i18n("code contributions, Arch Linux packaging, German translation"));

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
