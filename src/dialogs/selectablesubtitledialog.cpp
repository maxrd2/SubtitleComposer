/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2019 Mladen Milinkovic <max@smoothware.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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

	connect(subtitleButton, SIGNAL(clicked()), SLOT(selectSubtitle()));

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
		m_subtitleUrlLineEdit->setText(openDlg.selectedUrls().first().toLocalFile());
}

QUrl
SelectableSubtitleDialog::subtitleUrl() const
{
	return QUrl::fromLocalFile(m_subtitleUrlLineEdit->text());
}
