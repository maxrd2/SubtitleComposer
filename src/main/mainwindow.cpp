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
#include <QSplitter>
#include <QStatusBar>

#include <QMenuBar>
#include <KToolBar>
#include <KActionCollection>
#include <KConfigGroup>

using namespace SubtitleComposer;

MainWindow::MainWindow() :
	KXmlGuiWindow(0)
{
	m_horizontalSplitter = new QSplitter(this);

	QWidget *mainWidget = new QWidget(m_horizontalSplitter);
	mainWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	m_waveformWidget = new WaveformWidget(m_horizontalSplitter);
	m_waveformWidget->setContentsMargins(0, 0, 0, 0);

	m_horizontalSplitter->setOrientation(Qt::Horizontal);
	m_horizontalSplitter->setLineWidth(0);
	m_horizontalSplitter->setCollapsible(0, false);
	m_horizontalSplitter->setSizes(QList<int>() << 400 << 100);

	m_mainSplitter = new QSplitter(mainWidget);

	m_playerWidget = new PlayerWidget(m_mainSplitter);
	m_playerWidget->setContentsMargins(0, 0, 0, 0);

	m_linesWidget = new LinesWidget(m_mainSplitter);
	m_linesWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	m_mainSplitter->setOrientation(Qt::Vertical);
	m_mainSplitter->setLineWidth(0);
	m_mainSplitter->setCollapsible(1, false);
	m_mainSplitter->setSizes(QList<int>() << 100 << 200);

	m_curLineWidget = new CurrentLineWidget(mainWidget);
	m_curLineWidget->setMaximumHeight(m_curLineWidget->minimumSizeHint().height());

	QLayout *mainWidgetLayout = new QBoxLayout(QBoxLayout::TopToBottom, mainWidget);
	mainWidgetLayout->setContentsMargins(5, 1, 5, 2);
	mainWidgetLayout->setSpacing(5);
	mainWidgetLayout->addWidget(m_mainSplitter);
	mainWidgetLayout->addWidget(m_curLineWidget);

	setCentralWidget(m_horizontalSplitter);

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
	m_mainSplitter->setSizes(group.readEntry("Splitter Sizes", m_mainSplitter->sizes()));
	m_horizontalSplitter->setSizes(group.readEntry("Horizontal Splitter Sizes", m_horizontalSplitter->sizes()));
}

void
MainWindow::saveConfig()
{
	KConfigGroup group(KSharedConfig::openConfig()->group("MainWindow Settings"));
	group.writeEntry("Splitter Sizes", m_mainSplitter->sizes());
	group.writeEntry("Horizontal Splitter Sizes", m_horizontalSplitter->sizes());
}

void
MainWindow::setSubtitle(Subtitle * /*subtitle */)
{}

bool
MainWindow::queryClose()
{
	return app()->closeSubtitle();
}


