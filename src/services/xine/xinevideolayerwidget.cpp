/***************************************************************************
 *   Copyright (C) 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)  *
 *   based on Kaffeine by Jürgen Kofler                                    *
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

#include "xinevideolayerwidget.h"

#include <QtGui/QPaintEvent>

#include <X11/Xlib.h>
#ifdef HAVE_XCB
#include <xcb/xcb.h>
#endif

using namespace SubtitleComposer;

XineVideoLayerWidget::XineVideoLayerWidget(QWidget *parent) :
	QWidget(parent),
	m_videoDriver(0)
{}

XineVideoLayerWidget::~XineVideoLayerWidget()
{}

xine_video_port_t *
XineVideoLayerWidget::videoDriver() const
{
	return m_videoDriver;
}

void
XineVideoLayerWidget::setVideoDriver(xine_video_port_t *videoDriver)
{
	m_videoDriver = videoDriver;
}

#ifdef HAVE_XCB
void
XineVideoLayerWidget::paintEvent(QPaintEvent *event)
{
	if(m_videoDriver) {
		const QRect &rect = event->rect();
		xcb_expose_event_t xcb_event;
		memset(&xcb_event, 0, sizeof(xcb_event));
		xcb_event.window = winId();
		xcb_event.x = rect.x();
		xcb_event.y = rect.y();
		xcb_event.width = rect.width();
		xcb_event.height = rect.height();
		xcb_event.count = 0;
		xine_port_send_gui_data(m_videoDriver, XINE_GUI_SEND_EXPOSE_EVENT, &xcb_event);
	}

	QWidget::paintEvent(event);
}

#else
bool
XineVideoLayerWidget::x11Event(XEvent *event)
{
	if(m_videoDriver && event->type == Expose && event->xexpose.count == 0)
		xine_port_send_gui_data(m_videoDriver, XINE_GUI_SEND_EXPOSE_EVENT, event);

	return false;
}

#endif

void
XineVideoLayerWidget::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);
	emit geometryChanged();

/*	QSize size = videoWidget()->videoLayer()->size();
        QPoint globalPos = videoWidget()->videoLayer()->mapToGlobal( QPoint( 0, 0 ) );
        m_videoLayerGeometry = QRect( globalPos.x(), globalPos.y(), size.width(), size.height() );
 */
}

void
XineVideoLayerWidget::moveEvent(QMoveEvent *event)
{
	QWidget::moveEvent(event);
	emit geometryChanged();
}

#include "xinevideolayerwidget.moc"
