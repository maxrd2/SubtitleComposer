#ifndef VOBSUBINPUTINITDIALOG_H
#define VOBSUBINPUTINITDIALOG_H

/*
 * SPDX-FileCopyrightText: 2017-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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

	VobSubInputInitDialog(QWidget *parent = 0);
	~VobSubInputInitDialog();

	void streamListSet(const QStringList streams);
	int streamIndex() const;

	quint32 postProcessingFlags() const;

private:
	Ui::VobSubInputInitDialog *ui;
};
}

#endif // VOBSUBINPUTINITDIALOG_H
