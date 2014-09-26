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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// #define BUILD_MAIN_TESTS
#ifdef BUILD_MAIN_TESTS
#include "main_tests.h"
#else

#include "application.h"

#include <QtCore/QFile>
#include <QtCore/QDir>

#include <KAboutData>
#include <KCmdLineArgs>
#include <KLocale>
#include <KStandardDirs>

#if defined HAVE_XINE && !defined HAVE_XCB
#include <X11/Xlib.h>
#endif

static inline KUrl
KUrlFromParam(const QString path)
{
	KUrl url(path);
	if(!url.isRelative())
		return url;
	url.setPath(QDir::currentPath());
	url.addPath(path);
	return url;
}

int
main(int argc, char **argv)
{
#if defined HAVE_XINE && !defined HAVE_XCB
	XInitThreads(); // needed for XinePlayerBackend
#endif

	KAboutData aboutData(
			"subtitlecomposer",     // The program name used internally.
			"subtitlecomposer",     // The message catalog name.
			ki18n("Subtitle Composer"),     // A displayable program name string.
			"0.5.7",        // The program version string.
			ki18n("A KDE subtitle editor."),        // A short description of what the program does.
			KAboutData::License_GPL,        // License identifier
			ki18n("&copy; 2007-2012 Sergio Pistone\n&copy; 2013-2014 Mladen Milinković"),        // Copyright Statement
			KLocalizedString(),     // Additional text
			// We are not a project under the KDE umbrella (hopefully, we will be someday)
			"https://github.com/maxrd2/subtitlecomposer",   // Project Homepage
			"max@smoothware.net"    // Address for bugs
			);

	aboutData.addAuthor(
			ki18n("Sergio Pistone"),        // name
			ki18n("Original Author"),       // task
			"Sergio Pistone <sergio_pistone@yahoo.com.ar>"  // email
			);
	aboutData.addAuthor(
			ki18n("Mladen Milinković"),     // name
			ki18n("Author & Maintainer"),   // task
			"Mladen Milinkovic <max@smoothware.net>"        // email
			);

	// Initialize command line args
	KCmdLineArgs::init(argc, argv, &aboutData);

	// Define the command line options using KCmdLineOptions
	KCmdLineOptions options;
	options.add("+[Primary URL]", ki18n("Open location as primary subtitle"));
	options.add("+[Translation URL]", ki18n("Open location as translation subtitle"));

	// Register the supported options
	KCmdLineArgs::addCmdLineOptions(options);

	// Create application object
	SubtitleComposer::Application app;
	app.mainWindow()->show();

	// Handle our own options/arguments
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	if(args->count())
		app.openSubtitle(KUrlFromParam(args->arg(0)));

	if(args->count() > 1)
		app.openSubtitleTr(KUrlFromParam(args->arg(1)));

	args->clear();

	return app.exec();
}

#endif                                                  // BUILD_MAIN_TESTS
