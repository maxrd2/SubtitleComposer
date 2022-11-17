/*
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MPLAYERCONFIGWIDGET_H
#define MPLAYERCONFIGWIDGET_H

#include "ui_pocketsphinxconfigwidget.h"

namespace SubtitleComposer {
class PocketSphinxConfigWidget : public QWidget, Ui::PocketSphinxConfigWidget
{
	Q_OBJECT

public:
	explicit PocketSphinxConfigWidget(QWidget *parent = nullptr);
	virtual ~PocketSphinxConfigWidget();
};
}

#endif
