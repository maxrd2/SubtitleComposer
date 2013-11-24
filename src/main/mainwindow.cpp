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
#include "statusbar.h"
#include "../services/player.h"
#include "../services/decoder.h"

#include <QtGui/QGridLayout>
#include <QtGui/QSplitter>

#include <KMenuBar>
#include <KToolBar>
#include <KStatusBar>
#include <KActionCollection>
#include <KConfigGroup>

using namespace SubtitleComposer;

MainWindow::MainWindow() :
	KXmlGuiWindow(0)
{
	QWidget *mainWidget = new QWidget(this);

	m_splitter = new QSplitter(mainWidget);

	m_playerWidget = new PlayerWidget(m_splitter);
	m_playerWidget->setContentsMargins(0, 0, 0, 0);

	m_linesWidget = new LinesWidget(m_splitter);
	m_linesWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	m_splitter->setOrientation(Qt::Vertical);
	m_splitter->setLineWidth(0);
	m_splitter->setCollapsible(1, false);
	m_splitter->setSizes(QList<int>() << 100 << 200);

	m_curLineWidget = new CurrentLineWidget(mainWidget);
	m_curLineWidget->setMaximumHeight(m_curLineWidget->minimumSizeHint().height());

	m_statusBar = new StatusBar2(this);

	QLayout *mainWidgetLayout = new QBoxLayout(QBoxLayout::TopToBottom, mainWidget);
	mainWidgetLayout->setContentsMargins(5, 1, 5, 2);
	mainWidgetLayout->setSpacing(5);
	mainWidgetLayout->addWidget(m_splitter);
	mainWidgetLayout->addWidget(m_curLineWidget);

	setStatusBar(m_statusBar);
	setCentralWidget(mainWidget);

	statusBar()->show();
	toolBar()->show();
	menuBar()->show();
}

MainWindow::~MainWindow()
{
	app()->saveConfig();

	// We must disconnect the player and the decoder when closing down, otherwise signal
	// handlers could be called with the some object destroyed, crashing the application
	disconnect(Player::instance(), 0, 0, 0);
	disconnect(Decoder::instance(), 0, 0, 0);

	Player::instance()->setApplicationClosingDown();
	Decoder::instance()->setApplicationClosingDown();
}

void
MainWindow::loadConfig()
{
	KConfigGroup group(KGlobal::config()->group("MainWindow Settings"));

	m_splitter->setSizes(group.readEntry("Splitter Sizes", m_splitter->sizes()));
}

void
MainWindow::saveConfig()
{
	KConfigGroup group(KGlobal::config()->group("MainWindow Settings"));

	group.writeEntry("Splitter Sizes", m_splitter->sizes());
}

void
MainWindow::setSubtitle(Subtitle * /*subtitle */)
{}

bool
MainWindow::queryClose()
{
	return app()->closeSubtitle();
}

#include "mainwindow.moc"
