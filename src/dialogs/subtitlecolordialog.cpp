/*
    SPDX-FileCopyrightText: 2007-2012 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2013-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "subtitlecolordialog.h"

#include <QDebug>
#include <QDialogButtonBox>
#include <QPushButton>

#include <KLocalizedString>

using namespace SubtitleComposer;

SubtitleColorDialog::SubtitleColorDialog(QWidget *parent)
	: QColorDialog(parent),
	  m_defaultColorSelected(false)
{
	QDialogButtonBox *buttons = findChild<QDialogButtonBox *>();

	QPushButton *defaultColor = buttons->addButton(i18n("Default Color"), QDialogButtonBox::AcceptRole);
	connect(defaultColor, &QAbstractButton::clicked, this, [&](){
		m_defaultColorSelected = true;
		accept();
	});

	setOption(DontUseNativeDialog, true);
}

SubtitleColorDialog::SubtitleColorDialog(const QColor &initial, QWidget *parent)
	: SubtitleColorDialog(parent)
{
	QSignalBlocker sb(this);
	setCurrentColor(initial);
}

QColor
SubtitleColorDialog::selectedColor() const
{
	return m_defaultColorSelected ? QColor(0, 0, 0, 0) : QColorDialog::selectedColor();
}

QColor
SubtitleColorDialog::getColor(const QColor &initial, QWidget *parent, const QString &title, ColorDialogOptions options)
{
	SubtitleColorDialog dlg(initial, parent);
	if(!title.isEmpty())
		dlg.setWindowTitle(title);
	dlg.setOptions(options | DontUseNativeDialog);
	return dlg.exec() == QDialog::Accepted ? dlg.selectedColor() : QColor();
}

