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

#include "actiondialog.h"

#include <QtGui/QGroupBox>
#include <QtGui/QGridLayout>

#include <KDebug>
#include <KApplication>
#include <KAboutData>

using namespace SubtitleComposer;

ActionDialog::ActionDialog(const QString &title, QWidget *parent) :
	KDialog(parent)
{
	setCaption(title);

	setButtons(KDialog::Ok | KDialog::Cancel);
	setDefaultButton(KDialog::Ok);

	m_mainWidget = new QWidget(this);
	m_mainLayout = new QVBoxLayout(m_mainWidget);
	m_mainLayout->setAlignment(Qt::AlignTop);
	m_mainLayout->setContentsMargins(0, 0, 0, 0);
	m_mainLayout->setSpacing(5);

	setMainWidget(m_mainWidget);
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
	return KDialog::exec();
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
	KDialog::show();
}

QGroupBox *
ActionDialog::createGroupBox(const QString &title, bool addToLayout)
{
	QGroupBox *groupBox = new QGroupBox(m_mainWidget);
	groupBox->setTitle(title);

	if(addToLayout)
//      m_mainLayout->addWidget( groupBox, m_mainLayout->rowCount(), 0 );
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

#include "actiondialog.moc"
