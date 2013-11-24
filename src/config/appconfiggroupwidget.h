#ifndef APPCONFIGGROUPWIDGET_H
#define APPCONFIGGROUPWIDGET_H

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

#include "appconfiggroup.h"

#include <QtGui/QWidget>

class QGroupBox;
class QGridLayout;

namespace SubtitleComposer {
class AppConfigGroupWidget : public QWidget
{
	Q_OBJECT

public:
/// ownership of the config object is transferred to this object
	explicit AppConfigGroupWidget(AppConfigGroup *configGroup, QWidget *parent = 0);
	virtual ~AppConfigGroupWidget();

	const AppConfigGroup * config();

signals:
	void settingsChanged();

public slots:
/// If possible (i.e., configs are compatible), copies the config object into
/// the widget config. Ownership of config object it's not transferred.
	void setConfig(const AppConfigGroup *const config);

	void setControlsFromDefaults();
	virtual void setControlsFromConfig() = 0;
	virtual void setConfigFromControls() = 0;

protected:
	QGroupBox * createGroupBox(const QString &title = QString(), bool addToLayout = true);
	QGridLayout * createGridLayout(QGroupBox *groupBox);

protected:
	AppConfigGroup *m_config;
	QGridLayout *m_mainLayout;
};
}
#endif
