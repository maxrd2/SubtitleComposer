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
public:
	explicit SubtitleColorDialog(QWidget *parent = nullptr);
	explicit SubtitleColorDialog(const QColor &initial, QWidget *parent = nullptr);

	static QColor getColor(const QColor &initial = Qt::white,
				QWidget *parent = nullptr,
				const QString &title = QString(),
				ColorDialogOptions options = ColorDialogOptions());

	QColor selectedColor() const;

private:
	bool m_defaultColorSelected;
};
}

#endif /* SUBTITLECOLORDIALOG_H */
