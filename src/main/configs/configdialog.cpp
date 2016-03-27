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

#include "config.h"

#include <KConfigDialog>
#include <klocalizedstring.h>

#include <sonnet/configwidget.h>

using namespace SubtitleComposer;

ConfigDialog::ConfigDialog(QWidget *parent, const QString &name, KCoreConfigSkeleton *config) :
	KConfigDialog(parent, name, config),
	m_hasWidgetChanged(false)
{
	KPageWidgetItem *item;

	// General page
	item = addPage(new GeneralConfigWidget(nullptr), i18nc("@title General settings", "General"));
	item->setHeader(i18n("General Settings"));
	item->setIcon(QIcon::fromTheme(QStringLiteral("preferences-other")));

	// Error Check page
	item = addPage(new ErrorsConfigWidget(nullptr), i18nc("@title Error Check Settings", "Error check"));
	item->setHeader(i18n("Error Check Settings"));
	item->setIcon(QIcon::fromTheme(QStringLiteral("games-endturn")));

	// Spelling page
	m_sonnetConfigWidget = new Sonnet::ConfigWidget(nullptr);
	connect(m_sonnetConfigWidget, SIGNAL(configChanged()), this, SLOT(widgetChanged()));
	item = addPage(m_sonnetConfigWidget, i18nc("@title Spelling settings", "Spelling"));
	item->setHeader(i18n("Spelling Settings"));
	item->setIcon(QIcon::fromTheme(QStringLiteral("tools-check-spelling")));

	// VideoPlayer page
	item = addPage(new PlayerConfigWidget(nullptr), i18nc("@title Video player Settings", "Video player"));
	item->setHeader(i18n("Video player settings"));
	item->setIcon(QIcon::fromTheme(QStringLiteral("mediaplayer-logo"), QIcon(QStringLiteral(CUSTOM_ICON_INSTALL_PATH "mediaplayer-logo"))));

	// Backend pages
	for(const QString backendName : VideoPlayer::instance()->backendNames()) {
		if(QWidget *configWidget = VideoPlayer::instance()->backend(backendName)->newConfigWidget(nullptr)) {
			item = addPage(configWidget, backendName);
			item->setHeader(i18nc("@title Video player backend settings", "%1 backend settings", backendName));
			item->setIcon(QIcon::fromTheme(backendName.toLower() + QStringLiteral("-logo"), QIcon(QStringLiteral(CUSTOM_ICON_INSTALL_PATH) + backendName.toLower() + QStringLiteral("-logo"))));
		}
	}
}

void
ConfigDialog::widgetChanged()
{
	m_hasWidgetChanged = true;
	updateButtons();
}

void
ConfigDialog::updateSettings()
{
	m_sonnetConfigWidget->save();
	m_hasWidgetChanged = false;
	KConfigDialog::updateSettings();
	settingsChangedSlot();
}

bool
ConfigDialog::hasChanged()
{
	return m_hasWidgetChanged || KConfigDialog::hasChanged();
}

