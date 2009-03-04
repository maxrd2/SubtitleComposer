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

#include "spellingconfigwidget.h"
#include "../../player/player.h"
#include "../../widgets/layeredwidget.h"
#include "../../widgets/textoverlaywidget.h"

#include <QtGui/QGridLayout>
#include <QtGui/QPalette>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#include <QtGui/QFontComboBox>

#include <KPushButton>
#include <KComboBox>
#include <KColorCombo>
#include <KNumInput>

#include <KDebug>
#include <KLocale>
#include <KGlobal>
#include <KConfig>
#include <KConfigGroup>
#include <sonnet/configwidget.h>

using namespace SubtitleComposer;

#define SONNET_CONFIG_GROUP "Spelling"

SpellingConfigWidget::SpellingConfigWidget( QWidget* parent ):
	AppConfigGroupWidget( new SpellingConfig(), parent ),
	m_sonnetConfigWidget( 0 ),
	m_globalConfig( KGlobal::config().data() )
{
	setControlsFromConfig();
}

SpellingConfigWidget::~SpellingConfigWidget()
{
}

void SpellingConfigWidget::setControlsFromConfig()
{
	// NOTE: Sonnet::ConfigWidget doesn't provide a way to change the controls state to reflect
	// a new settings state, as it is only evaluated at initialization. We work around that by
	// setting the state of the KConfig object used to read the controls state and (re)creating
	// a new Sonnet::ConfigWidget instance. However, we are not supposed to change (any) application
	// config state in this method so, after the Sonnet::ConfigWidget is (re)created, we restore
	// the KConfig state to how it was before.

	KConfigGroup group( m_globalConfig->group( SONNET_CONFIG_GROUP ) );
	QMap<QString,QString> previousSettings = group.entryMap();
	group.writeEntry( SpellingConfig::keyDefaultLanguage(), config()->defaultLanguage() );
	group.writeEntry( SpellingConfig::keyDefaultClient(), config()->defaultClient() );
	group.writeEntry( SpellingConfig::keyCheckUppercase(), config()->checkUppercase() );
	group.writeEntry( SpellingConfig::keySkipRunTogether(), config()->skipRunTogether() );

	setUpdatesEnabled( false );

	delete m_sonnetConfigWidget;
	m_sonnetConfigWidget = new Sonnet::ConfigWidget( m_globalConfig, this );
	layout()->addWidget( m_sonnetConfigWidget );

	setUpdatesEnabled( true );

	group.writeEntry( SpellingConfig::keyDefaultLanguage(), previousSettings[SpellingConfig::keyDefaultLanguage()] );
	group.writeEntry( SpellingConfig::keyDefaultClient(), previousSettings[SpellingConfig::keyDefaultClient()] );
	group.writeEntry( SpellingConfig::keyCheckUppercase(), previousSettings[SpellingConfig::keyCheckUppercase()] );
	group.writeEntry( SpellingConfig::keySkipRunTogether(), previousSettings[SpellingConfig::keySkipRunTogether()] );

	connect( m_sonnetConfigWidget, SIGNAL( configChanged() ), this, SIGNAL( settingsChanged() ) );
}

void SpellingConfigWidget::setConfigFromControls()
{
	// NOTE: we make sure the Sonnet::ConfigWidget dumps it's settings to the KConfig object.
	// Only then we can load our config object from it.

	m_sonnetConfigWidget->save();

	KConfigGroup group( m_globalConfig->group( SONNET_CONFIG_GROUP ) );
	config()->setOption( SpellingConfig::keyDefaultLanguage(), group.readEntry( SpellingConfig::keyDefaultLanguage() ) );
	config()->setOption( SpellingConfig::keyDefaultClient(), group.readEntry( SpellingConfig::keyDefaultClient() ) );
	config()->setOption( SpellingConfig::keyCheckUppercase(), group.readEntry( SpellingConfig::keyCheckUppercase() ) );
	config()->setOption( SpellingConfig::keySkipRunTogether(), group.readEntry( SpellingConfig::keySkipRunTogether() ) );
}

#include "spellingconfigwidget.moc"
