/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "selectablesubtitledialog.h"
#include "application.h"

#include <QFileDialog>
#include <QTextCodec>
#include <QLabel>
#include <QGroupBox>
#include <QGridLayout>
#include <QPushButton>
#include <QIcon>

#include <KUrlCompletion>
#include <KComboBox>
#include <KLineEdit>
#include <KLocalizedString>

using namespace SubtitleComposer;

SelectableSubtitleDialog::SelectableSubtitleDialog(const QString &title, QWidget *parent)
	: ActionWithTargetDialog(title, parent)
{}

QGroupBox *
SelectableSubtitleDialog::createSubtitleGroupBox(const QString &title, bool addToLayout)
{
	m_subtitleGroupBox = createGroupBox(title, addToLayout);

	m_subtitleUrlLineEdit = new KLineEdit(m_subtitleGroupBox);
	m_subtitleUrlLineEdit->setCompletionObject(new KUrlCompletion());

	QLabel *subtitlePathLabel = new QLabel(m_subtitleGroupBox);
	subtitlePathLabel->setText(i18n("Path:"));
	subtitlePathLabel->setBuddy(m_subtitleUrlLineEdit);

	QPushButton *subtitleButton = new QPushButton(m_subtitleGroupBox);
	subtitleButton->setIcon(QIcon::fromTheme("document-open"));
	subtitleButton->setToolTip(i18n("Select subtitle"));
	int buttonSize = subtitleButton->sizeHint().height();
	subtitleButton->setFixedSize(buttonSize, buttonSize);

	connect(subtitleButton, &QAbstractButton::clicked, this, &SelectableSubtitleDialog::selectSubtitle);

	QHBoxLayout *subtitlePathLayout = new QHBoxLayout();
	subtitlePathLayout->addWidget(m_subtitleUrlLineEdit, 2);
	subtitlePathLayout->addWidget(subtitleButton);

	m_subtitleLayout = createLayout(m_subtitleGroupBox);
	m_subtitleLayout->setColumnStretch(1, 2);
	m_subtitleLayout->addWidget(subtitlePathLabel, 0, 0, Qt::AlignRight | Qt::AlignVCenter);
	m_subtitleLayout->addLayout(subtitlePathLayout, 0, 1, 1, 2);

	return m_subtitleGroupBox;
}

void
SelectableSubtitleDialog::selectSubtitle()
{
	QFileDialog openDlg(app()->mainWindow(), i18n("Open Subtitle"), QString(), Application::buildSubtitleFilesFilter());
	openDlg.setModal(true);
	openDlg.selectUrl(subtitleUrl().isEmpty() ? app()->lastSubtitleDirectory() : subtitleUrl());

	if(openDlg.exec() == QDialog::Accepted)
		m_subtitleUrlLineEdit->setText(openDlg.selectedUrls().constFirst().toLocalFile());
}

QUrl
SelectableSubtitleDialog::subtitleUrl() const
{
	return QUrl::fromLocalFile(m_subtitleUrlLineEdit->text());
}
