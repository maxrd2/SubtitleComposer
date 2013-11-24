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

#include "subtitlecolordialog.h"

#include <QDialogButtonBox>
#include <QPushButton>

#include <QDebug>

using namespace SubtitleComposer;

SubtitleColorDialog::SubtitleColorDialog(QWidget *parent /* = 0*/) :
	QColorDialog(parent),
	defaultColorSelected(false)
{
	QDialogButtonBox *buttons = findChild<QDialogButtonBox *>();

	QPushButton *defaultColor = buttons->addButton("Default Color", QDialogButtonBox::AcceptRole);
	connect(defaultColor, SIGNAL(clicked()), this, SLOT(acceptDefaultColor()));

	setOption(DontUseNativeDialog, true);
}

SubtitleColorDialog::~SubtitleColorDialog()
{}

/*static*/ QColor
SubtitleColorDialog::getColor(const QColor &initial, QWidget *parent, const QString &title, ColorDialogOptions options /* = 0*/)
{
	SubtitleColorDialog dlg(parent);
	if(!title.isEmpty())
		dlg.setWindowTitle(title);
	dlg.setOptions(options | DontUseNativeDialog);
	dlg.setCurrentColor(initial);
	dlg.exec();

	return dlg.defaultColorSelected ? QColor(0, 0, 0, 0) : dlg.selectedColor();
}

/*static*/ QColor
SubtitleColorDialog::getColor(const QColor &initial /* = Qt::white*/, QWidget *parent /* = 0*/)
{
	return getColor(initial, parent, QString(), ColorDialogOptions(0));
}

void
SubtitleColorDialog::acceptDefaultColor()
{
	defaultColorSelected = true;
	accept();
}
