/*
    SPDX-FileCopyrightText: 2015 Martin Steghöfer <martin@steghoefer.eu>
    SPDX-FileCopyrightText: 2015-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

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
	void updateSettings() override;

protected:
	bool hasChanged() override;

private:
	bool m_hasWidgetChanged;
	Sonnet::ConfigWidget *m_sonnetConfigWidget;
};

}

#endif
