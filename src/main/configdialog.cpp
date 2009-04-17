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

#include "configdialog.h"
#include "configs/generalconfigwidget.h"
#include "configs/spellingconfigwidget.h"
#include "configs/errorsconfigwidget.h"
#include "configs/playerconfigwidget.h"
#include "../common/commondefs.h"
#include "../player/player.h"
#include "../player/playerbackend.h"

#include <QtGui/QCheckBox>
#include <KPushButton>
#include <QtGui/QLabel>
#include <QtGui/QGroupBox>
#include <QtGui/QGridLayout>

#include <KLocale>
#include <KStandardDirs>
#include <KDebug>

using namespace SubtitleComposer;

ConfigDialog::ConfigDialog( const AppConfig& config, QWidget* parent ):
	KPageDialog( parent ),
	m_acceptedConfig( config ),
	m_config( config )
{
	setCaption( i18n( "Configure" ) );

	setFaceType( KPageDialog::List );
	setButtons( KDialog::Ok | KDialog::Apply | KDialog::Cancel | KDialog::Default );
	setDefaultButton( KDialog::Ok );
	showButtonSeparator( true );

	m_configWidgets.insert( General, new GeneralConfigWidget() );
	m_pageWidgets.insert( General, addPage( m_configWidgets.at( General ), i18nc( "@title General settings", "General" ) ) );
	m_pageWidgets.at( General )->setHeader( i18n( "General Settings" ) );
	m_pageWidgets.at( General )->setIcon( KIcon( "preferences-other" ) );

	m_configWidgets.insert( Errors, new ErrorsConfigWidget() );
	m_pageWidgets.insert( Errors, addPage( m_configWidgets.at( Errors ), i18nc( "@title Error checks settings", "Error Checks" ) ) );
	m_pageWidgets.at( Errors )->setHeader( i18n( "Error Checks Settings" ) );
	m_pageWidgets.at( Errors )->setIcon( KIcon( "games-endturn" ) );

	m_configWidgets.insert( Spelling, new SpellingConfigWidget() );
	m_pageWidgets.insert( Spelling, addPage( m_configWidgets.at( Spelling ), i18nc( "@title Spelling settings", "Spelling" ) ) );
	m_pageWidgets.at( Spelling )->setHeader( i18n( "Spelling Settings" ) );
	m_pageWidgets.at( Spelling )->setIcon( KIcon( "tools-check-spelling" ) );

	m_configWidgets.insert( Player, new PlayerConfigWidget() );
	m_pageWidgets.insert( Player, addPage( m_configWidgets.at( Player ), i18nc( "@title Player settings", "Player" ) ) );
	m_pageWidgets.at( Player )->setHeader( i18n( "Player Settings" ) );
	m_pageWidgets.at( Player )->setIcon( KIcon( KIcon( "mediaplayer" ) ) );

	unsigned idx = FirstBackend;
	QStringList backendNames( Player::instance()->backendNames() );
	for ( QStringList::ConstIterator it = backendNames.begin(), end = backendNames.end(); it != end; ++it, ++idx )
	{
		AppConfigGroupWidget* configWidget = Player::instance()->backend( *it )->newAppConfigGroupWidget( 0 );
		if ( configWidget )
		{
			m_configWidgets.append( configWidget );
			m_pageWidgets.insert( idx, addPage( configWidget, *it ) );
			m_pageWidgets.at( idx )->setHeader( i18nc( "@title Player backend settings", "%1 Backend Settings", *it ) );
			m_pageWidgets.at( idx )->setIcon( KIcon( (*it).toLower() + "-logo" ) );
		}
		else
			--idx;
	}

	setControlsFromConfig();

	enableButtonApply( false );

	connect( &config, SIGNAL( optionChanged( const QString&, const QString&, const QString& ) ),
			 this, SLOT( onOptionChanged( const QString&, const QString&, const QString& ) ) );

	connect( this, SIGNAL( applyClicked(void) ), this, SLOT( acceptConfig(void) ) );
	connect( this, SIGNAL( okClicked(void) ), this, SLOT( acceptConfigAndClose(void) ) );
	connect( this, SIGNAL( cancelClicked(void) ), this, SLOT( rejectConfigAndClose(void) ) );
	connect( this, SIGNAL( defaultClicked(void) ), this, SLOT( setActivePageControlsFromDefaults(void) ) );
	connect( this, SIGNAL( defaultClicked(void) ), this, SLOT( enableApply(void) ) );

	// Apply button connections/setup
	connect( this, SIGNAL( defaultClicked(void) ), this, SLOT( enableApply(void) ) );
	for ( QList<AppConfigGroupWidget*>::ConstIterator it = m_configWidgets.begin(), end = m_configWidgets.end(); it != end; ++it )
		connect( *it, SIGNAL( settingsChanged() ), this, SLOT( enableApply(void) ) );

	QSize size = minimumSizeHint();
	resize( size.width() + 35, size.height() + 35 );
}

ConfigDialog::~ConfigDialog()
{
}

const AppConfig& ConfigDialog::config() const
{
	return m_acceptedConfig;
}

void ConfigDialog::setConfig( const AppConfig& config )
{
	bool visible = isVisible();

	if ( visible )
		hide();

	m_acceptedConfig = m_config = config;
	setControlsFromConfig();

	enableButtonApply( false );

	if ( visible )
		show();
}

void ConfigDialog::setCurrentPage( unsigned pageIndex )
{
	if ( pageIndex < (unsigned)m_pageWidgets.count() )
		KPageDialog::setCurrentPage( m_pageWidgets.at( pageIndex ) );
}

void ConfigDialog::show()
{
	KPageDialog::show();
}

void ConfigDialog::hide()
{
	KPageDialog::hide();
}

void ConfigDialog::setControlsFromConfig()
{
	for ( int pageIndex = 0; pageIndex < m_configWidgets.count(); ++pageIndex )
		setControlsFromConfig( pageIndex );
}

void ConfigDialog::setControlsFromConfig( unsigned pageIndex )
{
	if ( pageIndex < (unsigned)m_configWidgets.count() )
	{
		AppConfigGroup* configGroup = m_config.group( m_configWidgets.at( pageIndex )->config()->name() );
		if ( configGroup )
		{
			m_configWidgets.at( pageIndex )->setConfig( configGroup );
			m_configWidgets.at( pageIndex )->setControlsFromConfig();
		}
	}
}

void ConfigDialog::setActivePageControlsFromDefaults()
{
	int activePageIndex = m_pageWidgets.indexOf( currentPage() );
	if ( activePageIndex >= 0 )
		setControlsFromDefaults( activePageIndex );
}

void ConfigDialog::setControlsFromDefaults( unsigned pageIndex )
{
	if ( pageIndex < (unsigned)m_configWidgets.count() )
		m_configWidgets.at( pageIndex )->setControlsFromDefaults();
}


void ConfigDialog::setConfigFromControls()
{
	for ( int pageIndex = 0; pageIndex < m_configWidgets.count(); ++pageIndex )
		setConfigFromControls( pageIndex );
}

void ConfigDialog::setConfigFromControls( unsigned pageIndex )
{
	if ( pageIndex < (unsigned)m_configWidgets.count() )
	{
		m_configWidgets.at( pageIndex )->setConfigFromControls();
		m_config.setGroup( m_configWidgets.at( pageIndex )->config()->clone() ); // replace the group
	}
}

void ConfigDialog::acceptConfig()
{
	setConfigFromControls();
	m_acceptedConfig = m_config;

	enableButtonApply( false );

	emit accepted();
}

void ConfigDialog::acceptConfigAndClose()
{
	acceptConfig();
	hide();
}

void ConfigDialog::rejectConfig()
{
	m_config = m_acceptedConfig;
	setControlsFromConfig();
	enableButtonApply( false );
}

void ConfigDialog::rejectConfigAndClose()
{
	hide();
	rejectConfig();
}

void ConfigDialog::enableApply()
{
	enableButtonApply( true );
}

void ConfigDialog::onOptionChanged( const QString& groupName, const QString& optionName, const QString& value )
{
	if ( ! m_config.group( groupName ) )
		return;

	m_config.group( groupName )->setOption( optionName, value );
	m_acceptedConfig.group( groupName )->setOption( optionName, value );

	bool wasApplyButtonEnabled = button( KDialog::Apply )->isEnabled();

	for ( int pageIndex = 0; pageIndex < m_configWidgets.count(); ++pageIndex )
	{
		if ( m_configWidgets.at( pageIndex )->config()->name() == groupName )
		{
			AppConfigGroup* configGroup = m_acceptedConfig.group( groupName );
			if ( configGroup )
			{
				m_configWidgets.at( pageIndex )->setConfig( configGroup );
				m_configWidgets.at( pageIndex )->setControlsFromConfig();
			}
			break;
		}
	}

	if ( ! wasApplyButtonEnabled )
		enableButtonApply( false );
}

#include "configdialog.moc"
