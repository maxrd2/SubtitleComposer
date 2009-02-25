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

#include <QtCore/QEvent>
#include <QtGui/QKeyEvent>

TimeEdit::TimeEdit( QWidget* parent ):
	KIntSpinBox( parent ),
	m_validator( new TimeValidator( this ) )
{
	setSingleStep( 100 );
	setMinimum( 0 );
	setMaximum( 86399999 );
}

QSize TimeEdit::sizeHint() const
{
	QSize size = KIntSpinBox::sizeHint();
	size.setWidth( size.width() + 5 );
	return size;
}

QString TimeEdit::textFromValue( int value ) const
{
	int hours = value / 3600000;
	int minutes = (value % 3600000) / 60000;
	int seconds = (value % 60000) / 1000;
	int mseconds = value % 1000;

	return QString().sprintf( "%02d:%02d:%02d.%03d", hours, minutes, seconds, mseconds );
}

int TimeEdit::valueFromText( const QString& text ) const
{
	int timeMillis;
	bool valid = m_validator->parse( text, timeMillis );
	return valid ? timeMillis : 0;
}

QValidator::State TimeEdit::validate( QString& input, int& pos ) const
{
	return m_validator->validate( input, pos );
}


bool TimeEdit::eventFilter( QObject* object, QEvent* event )
{
	bool ret = KIntSpinBox::eventFilter( object, event );

	if ( event->type() == QEvent::KeyRelease )
	{
		// special processing for key press
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>( event );
		if ( keyEvent->key() == Qt::Key_Return )
			emit valueEntered( value() );
	}

	return ret;
}

#include "timeedit.moc"
