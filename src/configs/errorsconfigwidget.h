#ifndef ERRORSCONFIGWIDGET_H
#define ERRORSCONFIGWIDGET_H

/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "ui_errorsconfigwidget.h"

namespace SubtitleComposer {
class ErrorsConfigWidget : public QWidget, private Ui::ErrorsConfigWidget
{
	Q_OBJECT

	friend class ConfigDialog;

public:
	explicit ErrorsConfigWidget(QWidget *parent = 0);
	virtual ~ErrorsConfigWidget();
};
}

#endif
