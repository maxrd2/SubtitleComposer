/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2017 Mladen Milinkovic <max@smoothware.net>
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

#include "mplayer/mp_msg.h"
#include "mplayer/vobsub.h"
#include "mplayer/spudec.h"

using namespace SubtitleComposer;

VobSubInputInitDialog::VobSubInputInitDialog(void *vob, void *spu, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::VobSubInputInitDialog),
	m_vob(vob),
	m_spu(spu)
{
	ui->setupUi(this);

	for(size_t i = 0; i < vobsub_get_indexes_count(m_vob); i++) {
		char const *const id = vobsub_get_id(m_vob, i);
		ui->comboStream->addItem(QString("Stream %1: %2").arg(i).arg(id ? id : "(no id)"));
	}
}

VobSubInputInitDialog::~VobSubInputInitDialog()
{
	delete ui;
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
