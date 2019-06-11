#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

/*
 * Copyright (C) 2015 Martin Steghöfer <martin@steghoefer.eu>
 * Copyright (C) 2015-2018 Mladen Milinkovic <max@smoothware.net>
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

#include <KConfigDialog>

#include <sonnet/configwidget.h>

namespace SubtitleComposer {

class ConfigDialog : public KConfigDialog
{
	Q_OBJECT

public:
	ConfigDialog(QWidget *parent, const QString &name, KCoreConfigSkeleton *config);

public slots:
	void widgetChanged();

public:
	virtual void updateSettings() override;

protected:
	virtual bool hasChanged() override;

private:
	bool m_hasWidgetChanged;
	Sonnet::ConfigWidget *m_sonnetConfigWidget;
};

}

#endif
