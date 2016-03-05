/**
 * Copyright (C) 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)
 * Copyright (C) 2015 Mladen Milinkovic <max@smoothware.net>
 * Copyright (C) 2015 Martin Steghöfer <martin@steghoefer.eu>
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

#include "configs/configdialog.h"

#include "configs/generalconfigwidget.h"
#include "configs/errorsconfigwidget.h"
#include "configs/playerconfigwidget.h"

#include "../videoplayer/videoplayer.h"
#include "../videoplayer/playerbackend.h"

#include <KConfigDialog>
#include <klocalizedstring.h>

#include <sonnet/configwidget.h>

using namespace SubtitleComposer;

ConfigDialog::ConfigDialog(QWidget *parent, const QString &name, KCoreConfigSkeleton *config) :
	KConfigDialog(parent, name, config),
	hasWidgetChanged(false)
{
	KPageWidgetItem *item;

	// General page
	item = addPage(new GeneralConfigWidget(NULL), i18nc("@title General settings", "General"));
	item->setHeader(i18n("General Settings"));
	item->setIcon(QIcon::fromTheme("preferences-other"));

	// Error Check page
	item = addPage(new ErrorsConfigWidget(NULL), i18nc("@title Error Check Settings", "Error Check"));
	item->setHeader(i18n("Error Check Settings"));
	item->setIcon(QIcon::fromTheme("games-endturn"));

	// Spelling page
	sonnetConfigWidget = new Sonnet::ConfigWidget(NULL);
	connect(sonnetConfigWidget, SIGNAL(configChanged()), this, SLOT(widgetChanged()));
	item = addPage(sonnetConfigWidget, i18nc("@title Spelling Settings", "Spelling"));
	item->setHeader(i18n("Spelling Settings"));
	item->setIcon(QIcon::fromTheme("tools-check-spelling"));

	// VideoPlayer page
	item = addPage(new PlayerConfigWidget(NULL), i18nc("@title VideoPlayer Settings", "VideoPlayer"));
	item->setHeader(i18n("VideoPlayer Settings"));
	item->setIcon(QIcon::fromTheme("mediaplayer-logo"));

	// Backend pages
	QStringList backendNames(VideoPlayer::instance()->backendNames());
	for(QStringList::ConstIterator it = backendNames.begin(); it != backendNames.end(); it++) {
		QWidget *configWidget = VideoPlayer::instance()->backend(*it)->newConfigWidget(0);
		if(configWidget) {
			item = addPage(configWidget, *it);
			item->setHeader(i18nc("@title VideoPlayer backend settings", "%1 Backend Settings", *it));
			item->setIcon(QIcon::fromTheme((*it).toLower() + "-logo"));
		}
	}
}

void
ConfigDialog::widgetChanged()
{
	hasWidgetChanged = true;
	updateButtons();
}

void
ConfigDialog::updateSettings()
{
	sonnetConfigWidget->save();
	hasWidgetChanged = false;
	KConfigDialog::updateSettings();
	settingsChangedSlot();
}

bool
ConfigDialog::hasChanged()
{
	return hasWidgetChanged || KConfigDialog::hasChanged();
}

