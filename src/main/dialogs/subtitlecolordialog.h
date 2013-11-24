#ifndef SUBTITLECOLORDIALOG_H
#define SUBTITLECOLORDIALOG_H
/***************************************************************************
 *   Copyright (C) 2007-2012 Sergio Pistone (sergio_pistone@yahoo.com.ar)  *
 *   Copyright (C) 2013 Mladen Milinkovic (max@smoothware.net)             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QColorDialog>

namespace SubtitleComposer {
class SubtitleColorDialog : public QColorDialog
{
	Q_OBJECT

public:
	explicit SubtitleColorDialog(QWidget *parent = 0);
	explicit SubtitleColorDialog(const QColor &initial, QWidget *parent = 0);
	~SubtitleColorDialog();

	static QColor getColor(const QColor &initial, QWidget *parent, const QString &title, ColorDialogOptions options = 0);
	static QColor getColor(const QColor &initial = Qt::white, QWidget *parent = 0);

protected:
	bool defaultColorSelected;

public slots:
	void acceptDefaultColor();
};
}

#endif /* SUBTITLECOLORDIALOG_H */
