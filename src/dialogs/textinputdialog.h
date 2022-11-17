/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TEXTINPUTDIALOG_H
#define TEXTINPUTDIALOG_H

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
