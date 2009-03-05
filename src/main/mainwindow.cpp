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

#include "mainwindow.h"
#include "application.h"
#include "audiolevelswidget.h"
#include "playerwidget.h"
#include "lineswidget.h"
#include "currentlinewidget.h"
#include "../player/player.h"

#include <QtGui/QGridLayout>
#include <QtGui/QSplitter>

#include <KMenuBar>
#include <KToolBar>
#include <KStatusBar>
#include <KActionCollection>

using namespace SubtitleComposer;

MainWindow::MainWindow():
	KXmlGuiWindow( 0 )
{
	QWidget* mainWidget = new QWidget( this );

	QSplitter* splitter = new QSplitter( mainWidget );

// 	m_audiolevelsWidget = new AudioLevelsWidget( splitter );
// 	m_audiolevelsWidget->hide();

	m_playerWidget = new PlayerWidget( splitter );
	m_playerWidget->setContentsMargins( 0, 0, 0, 0 );

	m_linesWidget = new LinesWidget( splitter );
	m_linesWidget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

	splitter->setOrientation( Qt::Vertical );
	splitter->setLineWidth( 0 );
	splitter->setCollapsible( 1, false );
	splitter->setSizes( QList<int>() << 100 << 200 );

	m_curLineWidget = new CurrentLineWidget( mainWidget );
	m_curLineWidget->setMaximumHeight( m_curLineWidget->minimumSizeHint().height() );

	QLayout* mainWidgetLayout = new QBoxLayout( QBoxLayout::TopToBottom, mainWidget );
	mainWidgetLayout->setContentsMargins( 5, 1, 5, 2 );
	mainWidgetLayout->setSpacing( 5 );
	mainWidgetLayout->addWidget( splitter );
	mainWidgetLayout->addWidget( m_curLineWidget );

	setCentralWidget( mainWidget ); // tell the KMainWindow that this is indeed the main widget

	statusBar()->show(); // a status bar
	toolBar()->show(); // and a tool bar
	menuBar()->show();
}


MainWindow::~MainWindow()
{
	app()->saveConfig();

	Player* player = Player::instance();

	// We must disconnect the player when closing down, otherwise signal handlers
	// could be called with the some object destroyed, crashing the application
	disconnect( player, 0, 0, 0 );

	player->setApplicationClosingDown();
}

void MainWindow::loadConfig()
{
}

void MainWindow::saveConfig()
{
}

void MainWindow::setSubtitle( Subtitle* /*subtitle*/ )
{
}

bool MainWindow::queryClose()
{
	return app()->closeSubtitle();
}

#include "mainwindow.moc"
