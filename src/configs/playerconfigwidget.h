/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef PLAYERCONFIGWIDGET_H
#define PLAYERCONFIGWIDGET_H

#include "ui_playerconfigwidget.h"

namespace SubtitleComposer {
class PlayerConfigWidget : public QWidget, private Ui::PlayerConfigWidget
{
	Q_OBJECT

	friend class ConfigDialog;

public:
	explicit PlayerConfigWidget(QWidget *parent=nullptr);
	virtual ~PlayerConfigWidget();

private slots:
	void onFamilyChanged(const QString &family);
	void onSizeChanged(int size);
	void onPrimaryColorChanged(const QColor &color);
	void onOutlineColorChanged(const QColor &color);
	void onOutlineWidthChanged(int width);
};
}

#endif
