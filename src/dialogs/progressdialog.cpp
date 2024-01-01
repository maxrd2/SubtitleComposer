/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "progressdialog.h"

#include <QLabel>
#include <QProgressBar>
#include <QBoxLayout>
#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QPushButton>

#include <KLocalizedString>

using namespace SubtitleComposer;

ProgressDialog::ProgressDialog(const QString &caption, const QString &description, bool allowCancel, QWidget *parent) :
	QDialog(parent, Qt::WindowTitleHint)
{
	setWindowTitle(caption);
	setModal(true);

	m_buttonBox = new QDialogButtonBox(allowCancel ? QDialogButtonBox::Cancel : QDialogButtonBox::NoButton, this);
	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	m_label = new QLabel(this);
	m_label->setText(description);
	m_label->setAlignment(Qt::AlignHCenter);

	m_progressBar = new QProgressBar(this);
	m_progressBar->setFormat(i18nc("%p is the percent value, % is the percent sign", "%p%"));
	m_progressBar->setMaximum(1);
	m_progressBar->setValue(0);

	QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->addWidget(m_label);
	mainLayout->addWidget(m_progressBar);
	mainLayout->addWidget(m_buttonBox);

	setMinimumWidth(300);
	resize(QSize(300, 10));
}

void
ProgressDialog::closeEvent(QCloseEvent *event)
{
	event->ignore();
}

QString
ProgressDialog::description() const
{
	return m_label->text();
}

void
ProgressDialog::setDescription(const QString &description)
{
	m_label->setText(description);
}

int
ProgressDialog::value() const
{
	return m_progressBar->value();
}

void
ProgressDialog::setValue(int value)
{
	m_progressBar->setValue(value);
}

int
ProgressDialog::minimum() const
{
	return m_progressBar->minimum();
}

void
ProgressDialog::setMinimum(int minimum)
{
	m_progressBar->setMinimum(minimum);
}

void
ProgressDialog::incrementMinimum(int delta)
{
	m_progressBar->setMinimum(m_progressBar->minimum() + delta);
}

int
ProgressDialog::maximum() const
{
	return m_progressBar->maximum();
}

void
ProgressDialog::setMaximum(int maximum)
{
	m_progressBar->setMaximum(maximum);
}

void
ProgressDialog::incrementMaximum(int delta)
{
	m_progressBar->setMaximum(m_progressBar->maximum() + delta);
}

bool
ProgressDialog::isCancellable() const
{
	return m_buttonBox->button(QDialogButtonBox::Cancel)->isEnabled();
}

void
ProgressDialog::setCancellable(bool cancellable)
{
	m_buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(cancellable);
}


