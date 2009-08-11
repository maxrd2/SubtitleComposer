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

#include "service.h"

#include <KDebug>

using namespace SubtitleComposer;

Service::Service():
	m_activeBackend( 0 ),
	m_widgetParent( 0 ),
	m_applicationClosingDown( false ),
	m_state( Service::Uninitialized )
{
}

Service::~Service()
{
}

bool Service::initialize( QWidget* widgetParent, const QString& prefBackendName )
{
	if ( isInitialized() )
	{
		kError() << "Service has already been initialized";
		return false;
	}

	m_widgetParent = widgetParent;

	if ( m_backendsMap.contains( prefBackendName ) )
	{
		// we first try to set the requested backend as active
		initializeBackendPrivate( m_backendsMap[prefBackendName] );
	}

	// if that fails, we set the first available backend as active
	if ( ! m_activeBackend )
	{
		for ( QMap<QString,ServiceBackend*>::ConstIterator it = m_backendsMap.begin(), end = m_backendsMap.end(); it != end; ++it )
			if ( initializeBackendPrivate( it.value() ) )
				break;
	}

	if ( ! m_activeBackend )
		kError() << "Failed to initialize a player backend";

	return m_activeBackend;
}

bool Service::reinitialize( const QString& prefBackendName )
{
	if ( ! isInitialized() )
		return false;

	ServiceBackend* targetBackend = m_backendsMap.contains( prefBackendName ) ? m_backendsMap[prefBackendName] : m_activeBackend;

	finalize();

	if ( ! initializeBackendPrivate( targetBackend ) )
	{
		for ( QMap<QString,ServiceBackend*>::ConstIterator it = m_backendsMap.begin(), end = m_backendsMap.end(); it != end; ++it )
			if ( initializeBackendPrivate( it.value() ) )
				break;
	}

	if ( ! m_activeBackend )
		kError() << "Failed to initialize a player backend";

	return m_activeBackend;
}

void Service::finalize()
{
	if ( ! isInitialized() )
		return;

	ServiceBackend* wasActiveBackend = m_activeBackend;

	finalizeBackend( m_activeBackend );

	m_state = Service::Uninitialized;
	m_activeBackend = 0;

	emit backendFinalized( wasActiveBackend );
}

bool Service::initializeBackendPrivate( ServiceBackend* backend )
{
	if ( m_activeBackend == backend )
		return true;

	if ( m_activeBackend )
		return false;

	if ( initializeBackend( backend, m_widgetParent ) )
	{
		m_state = Service::Initialized;
		m_activeBackend = backend;
		emit backendInitialized( backend );
	}

	return m_activeBackend == backend;
}


bool Service::isApplicationClosingDown() const
{
	return m_applicationClosingDown;
}

void Service::setApplicationClosingDown()
{
	m_applicationClosingDown = true;
}

QString Service::activeBackendName() const
{
	for ( QMap<QString,ServiceBackend*>::ConstIterator it = m_backendsMap.begin(), end = m_backendsMap.end(); it != end; ++it )
		if ( it.value() == m_activeBackend )
			return it.key();
	return QString();
}

QStringList Service::backendNames() const
{
	return m_backendsMap.keys();
}

// bool Service::initializeBackend( ServiceBackend* backend, QWidget* widgetParent )
// {
// 	if ( (m_videoWidget = backend->initialize( widgetParent )) )
// 	{
// 		connect( m_videoWidget, SIGNAL( destroyed() ), this, SLOT( onVideoWidgetDestroyed() ) );
// 		connect( m_videoWidget, SIGNAL( doubleClicked( const QPoint& ) ), this, SIGNAL( doubleClicked( const QPoint& ) ) );
// 		connect( m_videoWidget, SIGNAL( leftClicked( const QPoint& ) ), this, SIGNAL( leftClicked( const QPoint& ) ) );
// 		connect( m_videoWidget, SIGNAL( rightClicked( const QPoint& ) ), this, SIGNAL( rightClicked( const QPoint& ) ) );
// 		connect( m_videoWidget, SIGNAL( wheelUp() ), this, SIGNAL( wheelUp() ) );
// 		connect( m_videoWidget, SIGNAL( wheelDown() ), this, SIGNAL( wheelDown() ) );
//
// 		m_videoWidget->show();
//
// 		// NOTE: next is used to make videoWidgetParent update it's geometry
// 		QRect geometry = m_widgetParent->geometry();
// 		geometry.setHeight( geometry.height() + 1 );
// 		videoWidgetParent->setGeometry( geometry );
// 	}
// }

#include "service.moc"
