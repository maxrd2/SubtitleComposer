/**
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2015 Mladen Milinkovic <max@smoothware.net>
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

	connect(lineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(onLineEditTextChanged(const QString &)));
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
