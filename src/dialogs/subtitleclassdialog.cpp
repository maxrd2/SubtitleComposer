/*
    SPDX-FileCopyrightText: 2023 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "subtitleclassdialog.h"
#include "ui_subtitleclassdialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>


using namespace SubtitleComposer;


SubtitleClassDialog::SubtitleClassDialog(QWidget *parent, Qt::WindowFlags flags)
	: QDialog(parent, flags),
	  ui(new Ui::SubtitleClassDialog)
{
	ui->setupUi(this);

	connect(ui->classList, &QComboBox::currentTextChanged, this, [&](){ emit currentClassChanged(currentClass()); });
	connect(this, &QDialog::accepted, this, [&](){ emit classSelected(currentClass()); });
}

SubtitleClassDialog::SubtitleClassDialog(const QString &initial, QWidget *parent, Qt::WindowFlags flags)
	: SubtitleClassDialog(parent, flags)
{
	QSignalBlocker sb(this);
	setCurrentClass(initial);
}

SubtitleClassDialog::~SubtitleClassDialog()
{
	delete ui;
}

void
SubtitleClassDialog::setCurrentClass(const QString &cssClass)
{
	ui->classList->setCurrentText(cssClass);
}

QString
SubtitleClassDialog::currentClass() const
{
	return ui->classList->currentText();
}

QString
SubtitleClassDialog::getClass(const QStringList &classes, const QString &initial, QWidget *parent, const QString &title)
{
	SubtitleClassDialog dlg(parent);
	if(!title.isEmpty())
		dlg.setWindowTitle(title);
	dlg.ui->classList->addItems(classes);
	dlg.setCurrentClass(initial);
	return dlg.exec() == QDialog::Accepted ? dlg.selectedClass() : QString();
}
