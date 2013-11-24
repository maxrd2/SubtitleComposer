/***************************************************************************
 *   Copyright (C) 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)  *
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

#include "progressdialog.h"

#include <QtGui/QLabel>
#include <QtGui/QProgressBar>
#include <QtGui/QBoxLayout>
#include <QtGui/QCloseEvent>

#include <KLocale>

using namespace SubtitleComposer;

ProgressDialog::ProgressDialog(const QString &caption, const QString &description, bool allowCancel, QWidget *parent) :
//  KDialog(parent, Qt::FramelessWindowHint)
	KDialog(parent, Qt::WindowTitleHint)
{
	setCaption(caption);

	setModal(true);
	setButtons(allowCancel ? KDialog::Cancel : (QFlags<KDialog::ButtonCode>) 0);

	QWidget *mainWidget = new QWidget(this);
	setMainWidget(mainWidget);

	m_label = new QLabel(mainWidget);
	m_label->setText(description);
	m_label->setAlignment(Qt::AlignHCenter);

	m_progressBar = new QProgressBar(mainWidget);
	m_progressBar->setMaximum(1);
	m_progressBar->setValue(0);

	QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, mainWidget);
	mainLayout->setMargin(0);
	mainLayout->addWidget(m_label);
	mainLayout->addWidget(m_progressBar);

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
	return isButtonEnabled(KDialog::Cancel);
}

void
ProgressDialog::setCancellable(bool cancellable)
{
	enableButton(KDialog::Cancel, cancellable);
}

#include "progressdialog.moc"
