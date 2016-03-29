/**
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2015 Mladen Milinkovic <max@smoothware.net>
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

#include "mainwindow.h"
#include "application.h"
#include "playerwidget.h"
#include "lineswidget.h"
#include "currentlinewidget.h"
#include "../videoplayer/videoplayer.h"
#include "../widgets/waveformwidget.h"

#include <QGridLayout>
#include <QDockWidget>
#include <QStatusBar>

#include <QMenuBar>
#include <QMainWindow>
#include <KToolBar>
#include <KActionCollection>
#include <KConfigGroup>

#include <KLocalizedString>

using namespace SubtitleComposer;

MainWindow::MainWindow() :
	KXmlGuiWindow(0)
{
	QDockWidget *waveformDock = new QDockWidget(i18n("Waveform"), this);
	waveformDock->setObjectName(QStringLiteral("waveform_dock"));
	waveformDock->setAllowedAreas(Qt::AllDockWidgetAreas);
	waveformDock->setFeatures(QDockWidget::DockWidgetMovable);
	waveformDock->setFloating(false);

	m_waveformWidget = new WaveformWidget(waveformDock);
	m_waveformWidget->setContentsMargins(0, 0, 0, 0);

	waveformDock->setWidget(m_waveformWidget);
	waveformDock->setTitleBarWidget(m_waveformWidget->toolbarWidget());
	addDockWidget(Qt::RightDockWidgetArea, waveformDock);


	QDockWidget *playerDock = new QDockWidget(i18n("Video Player"), this);
	playerDock->setObjectName(QStringLiteral("player_dock"));
	playerDock->setAllowedAreas(Qt::AllDockWidgetAreas);
	playerDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetVerticalTitleBar);
	playerDock->setFloating(false);

	m_playerWidget = new PlayerWidget(playerDock);
	m_playerWidget->setContentsMargins(0, 0, 0, 0);

	playerDock->setWidget(m_playerWidget);
	playerDock->setTitleBarWidget(m_playerWidget->infoSidebarWidget());
	addDockWidget(Qt::TopDockWidgetArea, playerDock);


	QWidget *mainWidget = new QWidget(this);
	mainWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	m_linesWidget = new LinesWidget(mainWidget);
	m_linesWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	m_curLineWidget = new CurrentLineWidget(mainWidget);
	m_curLineWidget->setMaximumHeight(m_curLineWidget->minimumSizeHint().height());

	QLayout *mainWidgetLayout = new QBoxLayout(QBoxLayout::TopToBottom, mainWidget);
	mainWidgetLayout->setContentsMargins(5, 1, 5, 2);
	mainWidgetLayout->setSpacing(5);
	mainWidgetLayout->addWidget(m_linesWidget);
	mainWidgetLayout->addWidget(m_curLineWidget);

	setCentralWidget(mainWidget);

	statusBar()->addPermanentWidget(m_waveformWidget->progressWidget());

	statusBar()->show();
	toolBar()->show();
	menuBar()->show();
}

MainWindow::~MainWindow()
{
	app()->saveConfig();

	// We must disconnect the player and the decoder when closing down, otherwise signal
	// handlers could be called with the some object destroyed, crashing the application
	disconnect(VideoPlayer::instance(), 0, 0, 0);

	VideoPlayer::instance()->setApplicationClosingDown();
}

void
MainWindow::loadConfig()
{
	KConfigGroup group(KSharedConfig::openConfig()->group("MainWindow Settings"));
//	m_mainSplitter->setSizes(group.readEntry("Splitter Sizes", m_mainSplitter->sizes()));
//	m_horizontalSplitter->setSizes(group.readEntry("Horizontal Splitter Sizes", m_horizontalSplitter->sizes()));

	setCorner(Qt::TopLeftCorner, Qt::TopDockWidgetArea);
	setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
	setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
	setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);
}

void
MainWindow::saveConfig()
{
	KConfigGroup group(KSharedConfig::openConfig()->group("MainWindow Settings"));
//	group.writeEntry("Splitter Sizes", m_mainSplitter->sizes());
//	group.writeEntry("Horizontal Splitter Sizes", m_horizontalSplitter->sizes());
}

void
MainWindow::setSubtitle(Subtitle * /*subtitle */)
{}

bool
MainWindow::queryClose()
{
	return app()->closeSubtitle();
}


