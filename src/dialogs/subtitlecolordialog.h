/*
    SPDX-FileCopyrightText: 2007-2012 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2013-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SUBTITLECOLORDIALOG_H
#define SUBTITLECOLORDIALOG_H

#include <QColorDialog>

namespace SubtitleComposer {
class SubtitleColorDialog : public QColorDialog
{
	Q_OBJECT

public:
	explicit SubtitleColorDialog(QWidget *parent = 0);
	explicit SubtitleColorDialog(const QColor &initial, QWidget *parent = 0);
	~SubtitleColorDialog();

	static QColor getColor(const QColor &initial, QWidget *parent, const QString &title, ColorDialogOptions options = ColorDialogOptions());
	static QColor getColor(const QColor &initial = Qt::white, QWidget *parent = 0);

protected:
	bool defaultColorSelected;

public slots:
	void acceptDefaultColor();
};
}

#endif /* SUBTITLECOLORDIALOG_H */
