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

#include "appconfiggroupwidget.h"

#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>

using namespace SubtitleComposer;

AppConfigGroupWidget::AppConfigGroupWidget( AppConfigGroup* configGroup, QWidget* parent ):
	QWidget( parent, 0 ),
	m_config( configGroup )
{
	m_mainLayout = new QGridLayout( this );
	m_mainLayout->setAlignment( Qt::AlignTop );
	m_mainLayout->setMargin( 0 );
	m_mainLayout->setSpacing( 5 );
}

AppConfigGroupWidget::~AppConfigGroupWidget()
{
	delete m_config;
}

const AppConfigGroup* const AppConfigGroupWidget::config()
{
	return m_config;
}

void AppConfigGroupWidget::setConfig( const AppConfigGroup* const config )
{
	if ( config && m_config->isCompatibleWith( *config ) )
	{
		*m_config = *config;
		setControlsFromConfig();
	}
}

void AppConfigGroupWidget::setControlsFromDefaults()
{
	m_config->loadDefaults();
	setControlsFromConfig();
}

QGroupBox* AppConfigGroupWidget::createGroupBox( const QString& title, bool addToLayout )
{
	QGroupBox* groupBox = new QGroupBox( this );
	groupBox->setTitle( title );

	if ( addToLayout )
		m_mainLayout->addWidget( groupBox, m_mainLayout->rowCount(), 0 );

	return groupBox;
}

QGridLayout* AppConfigGroupWidget::createGridLayout( QGroupBox* groupBox )
{
	QGridLayout* gridLayout = new QGridLayout( groupBox );
	gridLayout->setAlignment( Qt::AlignTop );

	return gridLayout;
}

#include "appconfiggroupwidget.moc"
