/*****************************************************************************
 *   Copyright (C) 2006 by Adam Higerd                                       *
 *   ahigerd@stratitec.com                                                   *
 *                                                                           *
 *   QxtSignalWaiter is free software; you can redistribute it and/or modify *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation, version 2 or greater, or under the terms  *
 *   of the GNU Lesser General Public License, version 2.1 or greater, or    *
 *   under the terms of the Q Public License as published by Trolltech.      *
 *   This disjunctive license follows the license of the linked Qt toolkit.  *
 *   Specifically, if you use Qt under the GPL, QxtSignalWaiter is licensed  *
 *   to you under the GPL. If you use Qt under the QPL, this is licensed to  *
 *   you under the QPL. If you use Qt with a commercial license purchased    *
 *   from Trolltech, this is licensed to you under the LGPL.                 *
 *                                                                           *
 *   QxtSignalWaiter is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the            *
 *   appropriate license agreement for more details.                         *
 ****************************************************************************/

#include "qxtsignalwaiter.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QTimerEvent>

QxtSignalWaiter::QxtSignalWaiter( const QObject* sender, const char* signal, unsigned count ):
	QObject( 0 ),
	m_signals( count ),
	m_signalsCaught( 0 ),
	m_timeout( false ),
	m_eventFlags( QEventLoop::ExcludeUserInputEvents )
{
	if ( signal )
		connect( sender, signal, this, SLOT(signalCaught()) );
}

QxtSignalWaiter::QxtSignalWaiter( const QObject* sender, const char* signal1, const char* signal2, unsigned count ):
	QObject( 0 ),
	m_signals( count ),
	m_signalsCaught( 0 ),
	m_timeout( false ),
	m_eventFlags( QEventLoop::ExcludeUserInputEvents )
{
	if ( signal1 )
		connect( sender, signal1, this, SLOT(signalCaught()) );
	if ( signal2 )
		connect( sender, signal2, this, SLOT(signalCaught()) );
}

QxtSignalWaiter::QxtSignalWaiter( const QObject* sender, const char* signal1, const char* signal2, const char* signal3, unsigned count ):
	QObject( 0 ),
	m_signals( count ),
	m_signalsCaught( 0 ),
	m_timeout( false ),
	m_eventFlags( QEventLoop::ExcludeUserInputEvents )
{
	if ( signal1 )
		connect( sender, signal1, this, SLOT(signalCaught()) );
	if ( signal2 )
		connect( sender, signal2, this, SLOT(signalCaught()) );
	if ( signal3 )
		connect( sender, signal3, this, SLOT(signalCaught()) );
}

QxtSignalWaiter::QxtSignalWaiter( const QObject* sender, const char* signal1, const char* signal2, const char* signal3, const char* signal4, unsigned count ):
	QObject( 0 ),
	m_signals( count ),
	m_signalsCaught( 0 ),
	m_timeout( false ),
	m_eventFlags( QEventLoop::ExcludeUserInputEvents )
{
	if ( signal1 )
		connect( sender, signal1, this, SLOT(signalCaught()) );
	if ( signal2 )
		connect( sender, signal2, this, SLOT(signalCaught()) );
	if ( signal3 )
		connect( sender, signal3, this, SLOT(signalCaught()) );
	if ( signal4 )
		connect( sender, signal4, this, SLOT(signalCaught()) );
}

QxtSignalWaiter::~QxtSignalWaiter()
{
}

QEventLoop::ProcessEventsFlags QxtSignalWaiter::processEventFlags() const
{
	return m_eventFlags;
}

void QxtSignalWaiter::setProcessEventFlags( QEventLoop::ProcessEventsFlags eventFlags )
{
	m_eventFlags = eventFlags;
}

// Returns true if the signal was caught, returns false if the wait timed out
bool QxtSignalWaiter::wait( int msec )
{
// 	static int waitcount = 0;
// 	int waitID = waitcount++;
// 	qDebug( "starting wait %d", waitID );

	// Check input parameters
	if ( msec < -1 || m_signalsCaught >= m_signals )
		return m_signalsCaught >= m_signals;

	// activate the timeout
	if ( msec != -1 )
	{
		m_timerID = startTimer( msec );
		if ( m_timerID == 0 )
		{
			qDebug( "failed to initialize timer" );
			return false;
		}
	}
	// Begin waiting
	while( m_signalsCaught < m_signals && ! m_timeout )
// 		QApplication::eventLoop()->processEvents( QEventLoop::WaitForMore );
		// we must dissallow input events to prevent recursive calls to this function
		QCoreApplication::processEvents( m_eventFlags );

	// Clean up and return status
	killTimer( m_timerID );

// 	qDebug( "finishing wait %d", waitID );

	return m_signalsCaught >= m_signals || ! m_timeout;
}

void QxtSignalWaiter::reset()
{
	m_signalsCaught = 0;
}

void QxtSignalWaiter::signalCaught()
{
	m_signalsCaught++;
}

void QxtSignalWaiter::timerEvent( QTimerEvent* )
{
	killTimer( m_timerID );
	m_timeout = true;
}

#include "qxtsignalwaiter.moc"
