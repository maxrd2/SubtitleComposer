/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "actiondialog.h"

#include <QGroupBox>
#include <QGridLayout>

using namespace SubtitleComposer;

ActionDialog::ActionDialog(const QString &title, QWidget *parent)
	: QDialog(parent)
{
	setWindowTitle(title);

	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	m_mainWidget = new QWidget(this);
	m_mainLayout = new QVBoxLayout(m_mainWidget);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setAlignment(Qt::AlignTop);

	layout->addWidget(m_mainWidget);
	layout->addWidget(m_buttonBox);
}

int
ActionDialog::exec()
{
	// attempt at showing the full action name on the title...
	// it would work better if we could get the fontMetrics used by kwin
//  int minWidth = fontMetrics().width( windowTitle() ) + 120;
//  if ( minWidth > minimumWidth() )
//      setMinimumWidth( minWidth );

	resize(minimumSizeHint());
	return QDialog::exec();
}

void
ActionDialog::show()
{
	// attempt at showing the full action name on the title...
	// it would work better if we could get the fontMetrics used by kwin
//  int minWidth = fontMetrics().width( windowTitle() ) + 120;
//  if ( minWidth > minimumWidth() )
//      setMinimumWidth( minWidth );

	resize(minimumSizeHint());
	QDialog::show();
}

QGroupBox *
ActionDialog::createGroupBox(const QString &title, bool addToLayout)
{
	QGroupBox *groupBox = new QGroupBox(m_mainWidget);
	groupBox->setTitle(title);

	if(addToLayout)
		m_mainLayout->addWidget(groupBox);

	return groupBox;
}

QGridLayout *
ActionDialog::createLayout(QGroupBox *groupBox)
{
	QGridLayout *gridLayout = new QGridLayout(groupBox);
	gridLayout->setAlignment(Qt::AlignTop);
	gridLayout->setSpacing(5);
	return gridLayout;
}
