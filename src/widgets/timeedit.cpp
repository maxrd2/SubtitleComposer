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

#include "timeedit.h"
#include "timevalidator.h"

#include <QtCore/QTime>
#include <QtCore/QEvent>
#include <QtGui/QKeyEvent>

#include <KDebug>

TimeEdit::TimeEdit( QWidget* parent ):
	QTimeEdit( parent ),
	m_secsStep( 100 )
{
	setDisplayFormat( "hh:mm:ss.zzz" );
	setMinimumTime( QTime( 0, 0, 0, 0 ) );
	setMaximumTime( QTime( 23, 59, 59, 999 ) );
	setWrapping( false );
	setAlignment( Qt::AlignHCenter );

	setCorrectionMode( QAbstractSpinBox::CorrectToNearestValue );

	connect( this, SIGNAL( timeChanged( const QTime& ) ), this, SLOT( onTimeChanged( const QTime& ) ) );
}

int TimeEdit::msecsStep() const
{
	return m_secsStep;
}

void TimeEdit::setMSecsStep( int msecs )
{
	m_secsStep = msecs;
}

int TimeEdit::value() const
{
	return time().msecsTo( QTime() );
}

void TimeEdit::setValue( int value )
{
	QTime newTime = QTime().addMSecs( value );
	if ( time() != newTime )
		setTime( newTime );
}

void TimeEdit::onTimeChanged( const QTime& time )
{
// 	emit valueChanged( time.msecsTo( QTime() ) );
}

void TimeEdit::keyPressEvent( QKeyEvent* event )
{
	static const QTime minTime( 0, 0, 0, 0 );
	static const QTime maxTime( 23, 59, 59, 999 );

	QTime time = this->time();
	int key = event->key();

	if ( key == Qt::Key_Return )
	{
		emit valueEntered( time.msecsTo( QTime() ) );
		return;
	}
	else if ( key == Qt::Key_Up )
	{
		QTime newTime = time;

		switch ( currentSection() )
		{
			case QDateTimeEdit::MSecSection:
				//if ( time.msec() == 999 )
				//	newTime = time.addMSecs( 1 );
				newTime = time.addMSecs( 100 );
				break;
			case QDateTimeEdit::SecondSection:
				if ( time.second() == 59 )
					newTime = time.addMSecs( 1000 );
				break;
			case QDateTimeEdit::MinuteSection:
				if ( time.minute() == 59 )
					newTime = time.addMSecs( 60000 );
				break;
			default:
				break;
		}

		if ( time != newTime )
		{
			setTime( wrapping() || newTime > time ? newTime : maxTime );
			setSelectedSection( currentSection() );
			return;
		}
	}
	else if ( key == Qt::Key_Down )
	{
		QTime newTime = time;

		switch ( currentSection() )
		{
			case QDateTimeEdit::MSecSection:
				//if ( time.msec() == 0 )
				//	newTime = time.addMSecs( -1 );
				newTime = time.addMSecs( -100 );
				break;
			case QDateTimeEdit::SecondSection:
				if ( time.second() == 0 )
					newTime = time.addMSecs( -1000 );
				break;
			case QDateTimeEdit::MinuteSection:
				if ( time.minute() == 0 )
					newTime = time.addMSecs( -60000 );
				break;
			default:
				break;
		}

		if ( time != newTime )
		{
			setTime( wrapping() || newTime < time ? newTime : minTime );
			setSelectedSection( currentSection() );
			return;
		}
	}
	else if ( key == Qt::Key_PageUp )
	{
		QTime newTime = time.addMSecs( m_secsStep );
		setTime( wrapping() || newTime > time ? newTime : maxTime );
		setSelectedSection( QDateTimeEdit::MSecSection );
		return;
	}
	else if ( key == Qt::Key_PageDown )
	{
		QTime newTime = time.addMSecs( -m_secsStep );
		setTime( wrapping() || newTime < time ? newTime : minTime );
		setSelectedSection( QDateTimeEdit::MSecSection );
		return;
	}

	QTimeEdit::keyPressEvent( event );
}

#include "timeedit.moc"
