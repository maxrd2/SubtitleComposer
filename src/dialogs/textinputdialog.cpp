/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "textinputdialog.h"

#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>

using namespace SubtitleComposer;

TextInputDialog::TextInputDialog(const QString &caption, const QString &label, QWidget *parent)
	: QDialog(parent)
{
	init(caption, label, QString());
}

TextInputDialog::TextInputDialog(const QString &caption, const QString &label, const QString &value, QWidget *parent)
	: QDialog(parent)
{
	init(caption, label, value);
}

void
TextInputDialog::init(const QString &caption, const QString &labelText, const QString &value)
{
	setupUi(this);

	setWindowTitle(caption);

	lineEdit->setText(value);
	lineEdit->setFocus();

	label->setText(labelText);

	buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

	resize(300, 10);

	connect(lineEdit, &QLineEdit::textChanged, this, &TextInputDialog::onLineEditTextChanged);
}

const QString
TextInputDialog::value() const
{
	return lineEdit->text();
}

void
TextInputDialog::setValue(const QString &value)
{
	lineEdit->setText(value);
}

void
TextInputDialog::onLineEditTextChanged(const QString &text)
{
	buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!text.isEmpty());
}
