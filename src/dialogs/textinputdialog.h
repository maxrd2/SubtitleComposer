#ifndef TEXTINPUTDIALOG_H
#define TEXTINPUTDIALOG_H

/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2018 Mladen Milinkovic <max@smoothware.net>
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

#include "ui_textinputdialog.h"
#include <QDialog>

namespace SubtitleComposer {
class TextInputDialog : public QDialog, private Ui::TextInputDialog
{
	Q_OBJECT

public:
	TextInputDialog(const QString &caption, const QString &label, QWidget *parent = 0);
	TextInputDialog(const QString &caption, const QString &label, const QString &value, QWidget *parent = 0);

	const QString value() const;

public slots:
	void setValue(const QString &value);

private:
	void init(const QString &caption, const QString &label, const QString &value);

private slots:
	void onLineEditTextChanged(const QString &text);
};
}
#endif /*TEXTINPUTDIALOG_H*/
