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

#include "errorsdialog.h"
#include "application.h"
#include "configs/errorsconfig.h"
#include "../core/subtitle.h"
#include "../core/subtitleline.h"

#include <QtGui/QLabel>
#include <QtGui/QGridLayout>

#include <KDebug>
#include <KLocale>
#include <KApplication>
#include <KConfig>
#include <KConfigGroup>
#include <KMenu>
#include <KPushButton>

using namespace SubtitleComposer;

ErrorsDialog::ErrorsDialog( QWidget* parent ):
	KDialog( parent )
{
	setCaption( i18n( "Subtitle Errors" ) );

	setButtons( 0 );

	QWidget* mainWidget = new QWidget( this );
	setMainWidget( mainWidget );

	QGridLayout* mainLayout = new QGridLayout( mainWidget );
	mainLayout->setAlignment( Qt::AlignTop );
	mainLayout->setSpacing( 5 );

	m_clearFixedButton = new KPushButton( mainWidget );
	m_clearFixedButton->setText( i18n( "Clear Fixed Errors" ) );
	m_clearFixedButton->setEnabled( ! app()->errorsConfig()->autoClearFixed() );

	KPushButton* checkErrorsButton = new KPushButton( mainWidget );
	checkErrorsButton->setText( i18n( "Check Errors..." ) );

	KPushButton* clearErrorsButton = new KPushButton( mainWidget );
	clearErrorsButton->setText( i18n( "Clear Errors..." ) );

	KPushButton* settingsButton = new KPushButton( mainWidget );
	settingsButton->setText( i18n( "Settings..." ) );

	m_errorsWidget = new ErrorsWidget( mainWidget );

	m_statsLabel = new QLabel( mainWidget );

	mainLayout->addWidget( m_errorsWidget, 1, 0, 1, 5 );
	mainLayout->addWidget( m_statsLabel, 0, 0, 1, 5 );
	mainLayout->addWidget( m_clearFixedButton, 2, 0 );
	mainLayout->addItem( new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum ), 2, 1 );
	mainLayout->addWidget( checkErrorsButton, 2, 2 );
	mainLayout->addWidget( clearErrorsButton, 2, 3 );
	mainLayout->addWidget( settingsButton, 2, 4 );

	resize( 500, height() + 50 );

	onStatsChanged();

	connect( m_clearFixedButton, SIGNAL( clicked() ), app(), SLOT( recheckAllErrors() ) );
	connect( checkErrorsButton, SIGNAL( clicked() ), app(), SLOT( checkErrors() ) );
	connect( clearErrorsButton, SIGNAL( clicked() ), app(), SLOT( clearErrors() ) );
	connect( settingsButton, SIGNAL( clicked() ), app(), SLOT( showErrorsConfig() ) );

	connect( m_errorsWidget->model(), SIGNAL( statsChanged() ), this, SLOT( onStatsChanged() ) );

	connect( app()->errorsConfig(), SIGNAL( optionChanged( const QString&, const QString& ) ),
			 this, SLOT( onOptionChanged( const QString&, const QString& ) ) );
}

ErrorsWidget* ErrorsDialog::errorsWidget()
{
	return m_errorsWidget;
}

void ErrorsDialog::loadConfig()
{
	KConfigGroup group( KGlobal::config()->group( "ErrorsDialog Settings" ) );

	resize( group.readEntry<int>( "Width", width() ), group.readEntry<int>( "Height", height() ) );

	m_errorsWidget->loadConfig();
}

void ErrorsDialog::saveConfig()
{
	KConfigGroup group( KGlobal::config()->group( "ErrorsDialog Settings" ) );

	group.writeEntry( "Width", width() );
	group.writeEntry( "Height", height() );

	m_errorsWidget->saveConfig();
}

void ErrorsDialog::onStatsChanged()
{
	ErrorsModel* model = m_errorsWidget->model();

	QString lines = i18np( "Showing 1 line with issues", "Showing %1 lines with issues", model->lineWithErrorsCount() );
	if ( model->lineWithErrorsCount() )
	{
		QString marks = i18np( "1 mark", "%1 marks", model->markCount() );
		QString errors = i18np( "1 error", "%1 errors", model->errorCount() - model->markCount() );
		m_statsLabel->setText( QString( "%1 (%2, %3)" ).arg( lines ).arg( marks ).arg( errors ) );
	}
	else
		m_statsLabel->setText( lines );
}

void ErrorsDialog::onOptionChanged( const QString& option, const QString& value )
{
	if ( option == ErrorsConfig::keyAutoClearFixed() )
		m_clearFixedButton->setEnabled( value != "true" );
}

#include "errorsdialog.moc"
