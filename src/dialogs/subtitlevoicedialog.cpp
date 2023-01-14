/*
    SPDX-FileCopyrightText: 2023 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "subtitlevoicedialog.h"
#include "ui_subtitlevoicedialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>


using namespace SubtitleComposer;


SubtitleVoiceDialog::SubtitleVoiceDialog(QWidget *parent, Qt::WindowFlags flags)
	: QDialog(parent, flags),
	  ui(new Ui::SubtitleVoiceDialog)
{
	ui->setupUi(this);

	connect(ui->voiceList, &QComboBox::currentTextChanged, this, [&](){ emit currentVoiceChanged(currentVoice()); });
	connect(this, &QDialog::accepted, this, [&](){ emit voiceSelected(currentVoice()); });
}

SubtitleVoiceDialog::SubtitleVoiceDialog(const QString &initial, QWidget *parent, Qt::WindowFlags flags)
	: SubtitleVoiceDialog(parent, flags)
{
	QSignalBlocker sb(this);
	setCurrentVoice(initial);
}

SubtitleVoiceDialog::~SubtitleVoiceDialog()
{
	delete ui;
}

void
SubtitleVoiceDialog::setCurrentVoice(const QString &cssVoice)
{
	ui->voiceList->setCurrentText(cssVoice);
}

QString
SubtitleVoiceDialog::currentVoice() const
{
	return ui->voiceList->currentText();
}

QString
SubtitleVoiceDialog::getVoice(const QStringList &voices, const QString &initial, QWidget *parent, const QString &title)
{
	SubtitleVoiceDialog dlg(parent);
	if(!title.isEmpty())
		dlg.setWindowTitle(title);
	dlg.ui->voiceList->addItems(voices);
	if(!initial.isEmpty() && !voices.contains(initial))
		dlg.ui->voiceList->addItem(initial);
	dlg.setCurrentVoice(initial);
	return dlg.exec() == QDialog::Accepted ? dlg.selectedVoice() : QString();
}
