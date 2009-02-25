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

#include "appconfiggroup.h"
#include "appconfig.h"

#include <KConfig>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KDebug>

using namespace SubtitleComposer;

AppConfigGroup::AppConfigGroup( const QString& name, const QMap<QString,QString>& defaults ):
	QObject(),
	m_config( 0 ),
	m_name( name ),
	m_values(),
	m_defaultValues( defaults )
{
	loadDefaults();
}

AppConfigGroup::AppConfigGroup( const AppConfigGroup& configGroup ):
	QObject(),
	m_config( 0 ),
	m_name( configGroup.m_name ),
	m_values( configGroup.m_values ),
	m_defaultValues( configGroup.m_defaultValues )
{
}

bool AppConfigGroup::equals( const QString& str1, const QString& str2 )
{
	return (str1.isEmpty() && str2.isEmpty()) || str1 == str2;
}

bool AppConfigGroup::nonEquals( const QString& str1, const QString& str2 )
{
	return (!str1.isEmpty() || !str2.isEmpty()) && str1 != str2;
}

AppConfigGroup& AppConfigGroup::operator=( const AppConfigGroup& configGroup )
{
	if ( this == &configGroup )
		return *this;

	if ( ! isCompatibleWith( configGroup ) )
	{
		kDebug() << "can't assign imcompatible AppConfigGroup instances";
		return *this;
	}

	m_name = configGroup.m_name;

	m_defaultValues.clear();
	m_defaultValues = configGroup.m_defaultValues;

	// this makes sure we emit the corresponding optionChanged signals
	for ( QMap<QString,QString>::ConstIterator it = configGroup.m_values.begin(), end = configGroup.m_values.end();
		  it != end; ++it )
		setOption( it.key(), it.value() );

	return *this;
}

AppConfigGroup* AppConfigGroup::clone() const
{
	return new AppConfigGroup( *this );
}

AppConfigGroup::~AppConfigGroup()
{
}

const QString& AppConfigGroup::name() const
{
	return m_name;
}

void AppConfigGroup::loadDefaults()
{
	for ( QMap<QString,QString>::ConstIterator it = m_defaultValues.begin(), end = m_defaultValues.end(); it != end; ++it )
		setOption( it.key(), it.value() );
}

bool AppConfigGroup::isCompatibleWith( const AppConfigGroup& config ) const
{
	for ( QMap<QString,QString>::ConstIterator it = m_defaultValues.begin(), end = m_defaultValues.end();
		  it != end; ++it )
		if ( ! config.m_defaultValues.contains( it.key() ) )
			return false;

	for ( QMap<QString,QString>::ConstIterator it = config.m_defaultValues.begin(), end = config.m_defaultValues.end();
		  it != end; ++it )
		if ( ! m_defaultValues.contains( it.key() ) )
			return false;

	return true;
}

void AppConfigGroup::readFrom( const KConfig* config )
{
	if ( config->hasGroup( m_name ) )
	{
	 	KConfigGroup group( config->group( m_name ) );

		for ( QMap<QString,QString>::ConstIterator it = m_defaultValues.begin(), end = m_defaultValues.end(); it != end; ++it )
			setOption( it.key(), group.readEntry( it.key(), it.value() ) );
	}
}

void AppConfigGroup::readFrom( const KSharedConfig* config )
{
	if ( config->hasGroup( m_name ) )
	{
	 	KConfigGroup group( config->group( m_name ) );

		for ( QMap<QString,QString>::ConstIterator it = m_defaultValues.begin(), end = m_defaultValues.end(); it != end; ++it )
			setOption( it.key(), group.readEntry( it.key(), it.value() ) );
	}
}

void AppConfigGroup::writeTo( KConfig* config ) const
{
 	KConfigGroup group( config->group( m_name ) );

	for ( QMap<QString,QString>::ConstIterator it = m_values.begin(), end = m_values.end(); it != end; ++it )
		group.writeEntry( it.key(), it.value() );
}

void AppConfigGroup::writeTo( KSharedConfig* config ) const
{
 	KConfigGroup group( config->group( m_name ) );

	for ( QMap<QString,QString>::ConstIterator it = m_values.begin(), end = m_values.end(); it != end; ++it )
		group.writeEntry( it.key(), it.value() );
}

QString AppConfigGroup::option( const QString& optionName ) const
{
	return m_values.contains( optionName ) ? m_values[optionName] : QString();
}

bool AppConfigGroup::optionAsBool( const QString& optionName ) const
{
	return option( optionName ).trimmed().toLower() == "true";
}

long AppConfigGroup::optionAsLong( const QString& optionName ) const
{
	return option( optionName ).toLong();
}

int AppConfigGroup::optionAsInt( const QString& optionName ) const
{
	return option( optionName ).toInt();
}

QColor AppConfigGroup::optionAsColor( const QString& optionName ) const
{
	QColor color;
	color.setNamedColor( option( optionName ) );
	return color;
}

void AppConfigGroup::setOption( const QString& optionName, const QString& value )
{
	if ( ! m_defaultValues.contains( optionName ) )
		return;

	if ( ! m_values.contains( optionName ) )
	{
 		// NOTE: this condition can only triggered by operator=()
		m_values[optionName] = value;
		emit optionChanged( optionName, value );
		emit optionChanged( m_name, optionName, value );
	}
	else if ( nonEquals( m_values[optionName], value ) )
	{
		m_values[optionName] = value;
		emit optionChanged( optionName, value );
		emit optionChanged( m_name, optionName, value );
	}
}

void AppConfigGroup::setOption( const QString& optionName, const char* value )
{
	setOption( optionName, QString( value ) );
}

void AppConfigGroup::setOption( const QString& optionName, bool value )
{
	setOption( optionName, QString( value ? "true" : "false" ) );
}

void AppConfigGroup::setOption( const QString& optionName, long value )
{
	setOption( optionName, QString::number( value ) );
}

void AppConfigGroup::setOption( const QString& optionName, int value )
{
	setOption( optionName, QString::number( value ) );
}

void AppConfigGroup::setOption( const QString& optionName, const QColor& value )
{
	setOption( optionName, value.name() );
}

bool AppConfigGroup::operator==( const AppConfigGroup& configGroup ) const
{
	for ( QMap<QString,QString>::ConstIterator it = m_values.begin(), end = m_values.end(); it != end; ++it )
	{
		if ( ! configGroup.m_values.contains( it.key() ) )
			return false;

		if ( nonEquals( it.value(), configGroup.m_values[it.key()] ) )
			return false;
	}

	return true;
}

bool AppConfigGroup::operator!=( const AppConfigGroup& configGroup ) const
{
	return ! operator==( configGroup );
}

#include "appconfiggroup.moc"
