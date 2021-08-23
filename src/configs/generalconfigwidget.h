/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GENERALCONFIGWIDGET_H
#define GENERALCONFIGWIDGET_H

#include "ui_generalconfigwidget.h"

namespace SubtitleComposer {
class GeneralConfigWidget : public QWidget, private Ui::GeneralConfigWidget
{
	Q_OBJECT

public:
	explicit GeneralConfigWidget(QWidget *parent=NULL);
	virtual ~GeneralConfigWidget();
};
}

#endif
