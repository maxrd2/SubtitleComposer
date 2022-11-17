/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTINPUTDIALOG_H
#define INTINPUTDIALOG_H

#include "ui_intinputdialog.h"
#include <QDialog>

QT_FORWARD_DECLARE_CLASS(QLineEdit)
QT_FORWARD_DECLARE_CLASS(QDialogButtonBox)
class KIntNumInput;

namespace SubtitleComposer {
class IntInputDialog : public QDialog, private Ui::IntInputDialog
{
	Q_OBJECT

public:
	IntInputDialog(const QString &caption, const QString &label, QWidget *parent = 0);
	IntInputDialog(const QString &caption, const QString &label, int min, int max, QWidget *parent = 0);
	IntInputDialog(const QString &caption, const QString &label, int min, int max, int value, QWidget *parent = 0);

	int minimum() const;
	int maximum() const;
	int value() const;

public slots:
	void setMinimum(int minimum);
	void setMaximum(int maximum);
	void setValue(int value);

private:
	void init(const QString &caption, const QString &label, int min, int max, int value);
};
}
#endif /*INTINPUTDIALOG_H*/
