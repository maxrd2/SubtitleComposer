/*
    SPDX-FileCopyrightText: 2007-2012 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2013-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
	connect(defaultColor, &QAbstractButton::clicked, this, &SubtitleColorDialog::acceptDefaultColor);

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
	return getColor(initial, parent, QString(), ColorDialogOptions());
}

void
SubtitleColorDialog::acceptDefaultColor()
{
	defaultColorSelected = true;
	accept();
}
