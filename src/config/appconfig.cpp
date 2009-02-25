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

#include "appconfig.h"

#include <QtCore/QDir>

#include <KConfig>
#include <KSharedConfig>
#include <KDebug>

using namespace SubtitleComposer;

AppConfig::AppConfig():
	QObject(),
	m_groups()
{
}

AppConfig::AppConfig( const AppConfig& config ):
	QObject(),
	m_groups()
{
	for ( QMap<QString,AppConfigGroup*>::ConstIterator it = config.m_groups.begin(), end = config.m_groups.end();
		  it != end; ++it )
		setGroup( it.value()->clone() );
}

AppConfig& AppConfig::operator=( const AppConfig& config )
{
	if ( this == &config )
		return *this;

	if ( ! isCompatibleWith( config ) )
	{
		kDebug() << "can't assign imcompatible AppConfig instances";
		return *this;
	}

	for ( QMap<QString,AppConfigGroup*>::ConstIterator it = config.m_groups.begin(); it != config.m_groups.end(); ++it )
		setGroup( it.value()->clone() );

	return *this;
}

AppConfig::~AppConfig()
{
	for ( QMap<QString,AppConfigGroup*>::ConstIterator it = m_groups.begin(), end = m_groups.end(); it != end; ++it )
		delete it.value();
}

bool AppConfig::isCompatibleWith( const AppConfig& config )
{
	for ( QMap<QString,AppConfigGroup*>::ConstIterator it = m_groups.begin(), end = m_groups.end(); it != end; ++it )
	{
		if ( ! config.m_groups.contains( it.key() ) )
			return false;
		if ( ! it.value()->isCompatibleWith( *config.m_groups[it.key()] ) )
			return false;
	}

	for ( QMap<QString,AppConfigGroup*>::ConstIterator it = config.m_groups.begin(), end = config.m_groups.end();
		  it != end; ++it )
	{
		if ( ! m_groups.contains( it.key() ) )
			return false;
	}

	return true;
}

void AppConfig::loadDefaults()
{
	for ( QMap<QString,AppConfigGroup*>::ConstIterator it = m_groups.begin(), end = m_groups.end(); it != end; ++it )
		it.value()->loadDefaults();
}

void AppConfig::readFrom( const KConfig* config )
{
	for ( QMap<QString,AppConfigGroup*>::ConstIterator it = m_groups.begin(), end = m_groups.end(); it != end; ++it )
		it.value()->readFrom( config );
}

void AppConfig::readFrom( const KSharedConfig* config )
{
	for ( QMap<QString,AppConfigGroup*>::ConstIterator it = m_groups.begin(), end = m_groups.end(); it != end; ++it )
		it.value()->readFrom( config );
}

void AppConfig::writeTo( KConfig* config ) const
{
	for ( QMap<QString,AppConfigGroup*>::ConstIterator it = m_groups.begin(), end = m_groups.end(); it != end; ++it )
		it.value()->writeTo( config );
}

void AppConfig::writeTo( KSharedConfig* config ) const
{
	for ( QMap<QString,AppConfigGroup*>::ConstIterator it = m_groups.begin(), end = m_groups.end(); it != end; ++it )
		it.value()->writeTo( config );
}

AppConfigGroup* AppConfig::group( const QString& name )
{
 	return m_groups.contains( name ) ? m_groups[name] : 0;
}

const AppConfigGroup* const AppConfig::group( const QString& name ) const
{
	return m_groups.contains( name ) ? m_groups[name] : 0;
}

void AppConfig::setGroup( AppConfigGroup* group )
{
	if ( m_groups.contains( group->name() ) )
	{
		*m_groups[group->name()] = *group;
		delete group;
	}
	else
	{
		group->m_config = this;
		m_groups[group->name()] = group;

		connect( group, SIGNAL(optionChanged(const QString&,const QString&,const QString&)),
				this, SIGNAL(optionChanged(const QString&,const QString&,const QString&)) );
	}
}

AppConfigGroup* AppConfig::removeGroup( const QString& name )
{
	if ( m_groups.contains( name ) )
	{
		AppConfigGroup* group = m_groups[name];
		group->m_config = 0;

		disconnect( group, SIGNAL(optionChanged(const QString&,const QString&,const QString&)),
					this, SIGNAL(optionChanged(const QString&,const QString&,const QString&)) );

		m_groups.remove( name );

		return group;
	}
	else
		return 0;
}

#include "appconfig.moc"
