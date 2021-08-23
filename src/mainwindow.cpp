/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mainwindow.h"
#include "application.h"
#include "gui/playerwidget.h"
#include "gui/treeview/lineswidget.h"
#include "gui/currentlinewidget.h"
#include "videoplayer/videoplayer.h"
#include "gui/waveform/waveformwidget.h"

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

MainWindow::MainWindow()
	: KXmlGuiWindow(nullptr)
{
	VideoPlayer::instance()->setParent(this);

	QDockWidget *waveformDock = new QDockWidget(i18n("Waveform"), this);
	waveformDock->setObjectName(QStringLiteral("waveform_dock"));
	waveformDock->setAllowedAreas(Qt::AllDockWidgetAreas);
	waveformDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
	waveformDock->setFloating(false);

	m_waveformWidget = new WaveformWidget(waveformDock);
	m_waveformWidget->setContentsMargins(0, 0, 0, 0);

	waveformDock->setWidget(m_waveformWidget);
	waveformDock->setTitleBarWidget(m_waveformWidget->toolbarWidget());
	addDockWidget(Qt::RightDockWidgetArea, waveformDock);


	QDockWidget *playerDock = new QDockWidget(i18n("Video Player"), this);
	playerDock->setObjectName(QStringLiteral("player_dock"));
	playerDock->setAllowedAreas(Qt::AllDockWidgetAreas);
	playerDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetVerticalTitleBar);
	playerDock->setFloating(false);

	m_playerWidget = new PlayerWidget(playerDock);
	m_playerWidget->setContentsMargins(0, 0, 0, 0);

	playerDock->setWidget(m_playerWidget);
	playerDock->setTitleBarWidget(m_playerWidget->infoSidebarWidget());
	addDockWidget(Qt::TopDockWidgetArea, playerDock);


	QWidget *mainWidget = new QWidget(this);
	mainWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	m_linesWidget = new LinesWidget(mainWidget);

	m_curLineWidget = new CurrentLineWidget(mainWidget);
	m_curLineWidget->setMaximumHeight(m_curLineWidget->minimumSizeHint().height());

	QLayout *mainWidgetLayout = new QBoxLayout(QBoxLayout::TopToBottom, mainWidget);
	mainWidgetLayout->setContentsMargins(0, 1, 0, 0);
	mainWidgetLayout->setSpacing(2);
	mainWidgetLayout->addWidget(m_linesWidget);
	mainWidgetLayout->addWidget(m_curLineWidget);

	setCentralWidget(mainWidget);

	statusBar()->addPermanentWidget(m_waveformWidget->progressWidget());

	connect(m_linesWidget, &LinesWidget::currentLineChanged, m_waveformWidget, &WaveformWidget::onSubtitleChanged);

	statusBar()->show();
	toolBar()->show();
	menuBar()->show();
}

MainWindow::~MainWindow()
{
	app()->saveConfig();
}

void
MainWindow::loadConfig()
{
	setCorner(Qt::TopLeftCorner, Qt::TopDockWidgetArea);
	setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
	setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
	setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);
}

void
MainWindow::saveConfig()
{
}

void
MainWindow::setSubtitle(Subtitle */*subtitle*/)
{}

QMenu *
MainWindow::createPopupMenu()
{
	QMenu *menu = KXmlGuiWindow::createPopupMenu();

	menu->addSeparator();
	QAction *showStatusBarAction = actionCollection()->action(KStandardAction::name(KStandardAction::ShowStatusbar));
	menu->addAction(showStatusBarAction);

	return menu;
}

bool
MainWindow::queryClose()
{
	return app()->closeSubtitle();
}
