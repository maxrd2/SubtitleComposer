/*
 * Copyright (C) 2017-2018 Mladen Milinkovic <max@smoothware.net>
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

#include "vobsubinputinitdialog.h"
#include "ui_vobsubinputinitdialog.h"

using namespace SubtitleComposer;

VobSubInputInitDialog::VobSubInputInitDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::VobSubInputInitDialog)
{
	ui->setupUi(this);
}

VobSubInputInitDialog::~VobSubInputInitDialog()
{
	delete ui;
}

void
VobSubInputInitDialog::streamListSet(const QStringList streams)
{
	ui->comboStream->addItems(streams);
}

int
VobSubInputInitDialog::streamIndex() const
{
	return ui->comboStream->currentIndex();
}

quint32
VobSubInputInitDialog::postProcessingFlags() const
{
	quint32 flags = 0;

	if(ui->ppAposQuote->isChecked())
		flags |= APOSTROPHE_TO_QUOTES;
	if(ui->ppSpacePunct->isChecked())
		flags |= SPACE_PUNCTUATION;
	if(ui->ppSpaceNumber->isChecked())
		flags |= SPACE_NUMBERS;
	if(ui->ppSpaceParen->isChecked())
		flags |= SPACE_PARENTHESES;
	if(ui->ppCharsOCR->isChecked())
		flags |= CHARS_OCR;

	return flags;
}
