#ifndef VOBSUBINPUTINITDIALOG_H
#define VOBSUBINPUTINITDIALOG_H

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

#include <QDialog>

namespace Ui {
class VobSubInputInitDialog;
}

namespace SubtitleComposer {
class VobSubInputInitDialog : public QDialog
{
	Q_OBJECT

public:
	enum PostProcessFlags {
		APOSTROPHE_TO_QUOTES = 1,
		SPACE_PUNCTUATION = 2,
		SPACE_NUMBERS = 4,
		SPACE_PARENTHESES = 8,
		CHARS_OCR = 16
	};

	VobSubInputInitDialog(void *vob, void *spu, QWidget *parent = 0);
	~VobSubInputInitDialog();

	int streamIndex() const;

	quint32 postProcessingFlags() const;

private:
	Ui::VobSubInputInitDialog *ui;

	void *m_vob;
	void *m_spu;
};
}

#endif // VOBSUBINPUTINITDIALOG_H
