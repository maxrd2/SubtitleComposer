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

#include <KAboutData>
#include <KCmdLineArgs>
#include <KLocale>
#include <KStandardDirs>

int main( int argc, char** argv )
{
	KAboutData aboutData(
		"subtitlecomposer",							// The program name used internally.
		"subtitlecomposer",							// The message catalog name.
		ki18n( "Subtitle Composer" ),				// A displayable program name string.
		"0.5.3",									// The program version string.
		ki18n( "A KDE subtitle editor." ),			// A short description of what the program does.
		KAboutData::License_GPL,					// License identifier
		ki18n( "(C) 2007-2009 Sergio Pistone" )		// Copyright Statement
	);

	aboutData.addAuthor(
		ki18n( "Sergio Pistone" ),					// name
		ki18n( "Author & maintainer" ),				// task
		"sergio_pistone@yahoo.com.ar"				// email
	);

	// Initialize command line args
    KCmdLineArgs::init( argc, argv, &aboutData );

	// Define the command line options using KCmdLineOptions
	KCmdLineOptions options;
	options.add( "+[Primary URL]", ki18n( "Open location as primary subtitle") );
	options.add( "+[Translation URL]", ki18n( "Open location as translation subtitle") );

	// Register the supported options
	KCmdLineArgs::addCmdLineOptions( options );

	// Create application object
    SubtitleComposer::Application app;
	app.mainWindow()->show();

	// Handle our own options/arguments
	KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

	if ( args->count() )
		app.openSubtitle( KUrl( args->arg( 0 ) ) );

	if ( args->count() > 1 )
		app.openTrSubtitle( KUrl( args->arg( 1 ) ) );

	args->clear();

	return app.exec();
}

#endif // BUILD_MAIN_TESTS

