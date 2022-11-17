/*
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WAVEFORMCONFIGWIDGET_H
#define WAVEFORMCONFIGWIDGET_H

#include "ui_waveformconfigwidget.h"

namespace SubtitleComposer {
class WaveformConfigWidget : public QWidget, private Ui::WaveformConfigWidget
{
	Q_OBJECT
public:
	explicit WaveformConfigWidget(QWidget *parent = 0);

signals:

public slots:
};
}

#endif // WAVEFORMCONFIGWIDGET_H
