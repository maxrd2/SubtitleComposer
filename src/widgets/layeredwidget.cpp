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

#include "layeredwidget.h"

#include <typeinfo>

#include <QtGui/QResizeEvent>

LayeredWidget::LayeredWidget( QWidget* parent, Qt::WFlags f ):
	QWidget( parent, f )
{
}

void LayeredWidget::setWidgetMode( QWidget* widget, LayeredWidget::Mode mode )
{
	m_ignoredWidgets.removeAll( widget );
	if ( mode == IgnoreResize )
		m_ignoredWidgets.append( widget );
}

void LayeredWidget::setMouseTracking( bool enable )
{
	// propagates to our children and our children children's
	QWidget::setMouseTracking( enable );
	QList<QWidget*> children = findChildren<QWidget*>();
	for ( QList<QWidget*>::ConstIterator it = children.begin(), end = children.end(); it != end; ++it )
		(*it)->setMouseTracking( enable );
}

void LayeredWidget::resizeEvent( QResizeEvent* )
{
	// propagated to our children but not our children children's
	QSize size = this->size();
	const QObjectList& children = this->children();
	for ( QObjectList::ConstIterator it = children.begin(), end = children.end(); it != end; ++it )
	{
		if ( ! (*it)->isWidgetType() || m_ignoredWidgets.contains( *it ) )
			continue;
		(static_cast<QWidget*>( *it ))->resize( size );
	}
}

#include "layeredwidget.moc"
