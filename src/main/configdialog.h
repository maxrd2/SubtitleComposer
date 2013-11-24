#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

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

#include "../config/appconfig.h"

#include <QtCore/QList>

#include <KPageDialog>

namespace SubtitleComposer {
class AppConfigGroupWidget;

class ConfigDialog : public KPageDialog
{
	Q_OBJECT

public:
	typedef enum { General = 0, Errors, Spelling, Player, FirstBackend } Pages;

	explicit ConfigDialog(const AppConfig &config, QWidget *parent = 0);
	virtual ~ConfigDialog();

	void setConfig(const AppConfig &config);
	const AppConfig & config() const;

	void setCurrentPage(unsigned pageIndex);

public slots:
	void show();
	void hide();

signals:
	void accepted();

protected slots:
	void acceptConfig();
	void acceptConfigAndClose();
	void rejectConfig();
	void rejectConfigAndClose();

	void setControlsFromConfig();
	void setControlsFromConfig(unsigned pageIndex);

	void setConfigFromControls();
	void setConfigFromControls(unsigned pageIndex);

	void setActivePageControlsFromDefaults();
	void setControlsFromDefaults(unsigned pageIndex);

	void enableApply();

	void onOptionChanged(const QString &groupName, const QString &optionName, const QString &value);

protected:
	AppConfig m_acceptedConfig;
	AppConfig m_config;

	QList<AppConfigGroupWidget *> m_configWidgets;
	QList<KPageWidgetItem *> m_pageWidgets;
};
}
#endif
