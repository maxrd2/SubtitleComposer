#ifndef QXTSIGNALWAITER_H
#define QXTSIGNALWAITER_H

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

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include <QtCore/QObject>
#include <QtCore/QEventLoop>

class QTimerEvent;

// QxtSignalWaiter is, sadly, not reentrant. In particular, only one QxSignalWaiter
// object can be safely waiting at a time. If a second QxSignalWaiter is used while
// the first is waiting, the first will not return until the second has timed out or
// successfully caught its signal. A later revision of the class may be able to solve
// this problem. Until then, be careful not to rely on two QxtSignalWaiter objects
// at the same time.

class QxtSignalWaiter : public QObject
{
	Q_OBJECT

	public:

		QxtSignalWaiter( const QObject* sender, const char* signal, unsigned count=1 );
		QxtSignalWaiter( const QObject* sender, const char* signal1, const char* signal2, unsigned count=1 );
		QxtSignalWaiter( const QObject* sender, const char* signal1, const char* signal2, const char* signal3, unsigned count=1 );
		QxtSignalWaiter( const QObject* sender, const char* signal1, const char* signal2, const char* signal3, const char* signal4, unsigned count=1 );
		virtual ~QxtSignalWaiter();

		QEventLoop::ProcessEventsFlags processEventFlags() const;
		void setProcessEventFlags( QEventLoop::ProcessEventsFlags eventFlags );

		bool wait( int msec=-1 );
		void reset();

	protected:

		void timerEvent( QTimerEvent* event );

	private slots:

		void signalCaught();

	private:

		const int m_signals;
		int m_signalsCaught;
		int m_timeout;
		int m_timerID;
		QEventLoop::ProcessEventsFlags m_eventFlags;
};


#endif
