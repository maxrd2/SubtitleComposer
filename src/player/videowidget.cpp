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

#include "videowidget.h"

#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>

#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QResizeEvent>
#include <QtGui/QMoveEvent>
#include <QtGui/QMouseEvent>

#include <KDebug>

using namespace SubtitleComposer;

VideoWidget::VideoWidget( QWidget* parent ):
	QWidget( parent ),
	m_videoLayer( new QWidget( this ) ),
	m_videoWidth( 0 ),
	m_videoHeight( 0 ),
	m_videoDAR( 0 )
{
	m_videoLayer->hide();

	init( true );
}

VideoWidget::VideoWidget( QWidget* videoLayer, QWidget* parent ):
	QWidget( parent ),
	m_videoLayer( videoLayer ),
	m_videoWidth( 0 ),
	m_videoHeight( 0 ),
	m_videoDAR( 0 )
{
	m_videoLayer->setParent( this );
	m_videoLayer->hide();

	init( true );
}

void VideoWidget::init( bool setVideoLayerAttributes )
{
	if ( setVideoLayerAttributes )
	{
		m_videoLayer->setFocusPolicy( Qt::NoFocus );
		m_videoLayer->setAttribute( Qt::WA_StaticContents, true );
		m_videoLayer->setAttribute( Qt::WA_PaintOnScreen, true );
		m_videoLayer->setAttribute( Qt::WA_OpaquePaintEvent, true );
		m_videoLayer->setAttribute( Qt::WA_NoSystemBackground, true );
	}

	setFocusPolicy( Qt::NoFocus );
	setAttribute( Qt::WA_StaticContents, true );
	setAttribute( Qt::WA_OpaquePaintEvent, true );
	setAttribute( Qt::WA_PaintOnScreen, true );
	setAttribute( Qt::WA_NoSystemBackground, false );
	QPalette palette = this->palette();
	palette.setColor( QPalette::Background, Qt::black );
	setPalette( palette );
}

VideoWidget::~VideoWidget()
{
}

QWidget* VideoWidget::videoLayer()
{
	return m_videoLayer;
}

int VideoWidget::videoWidth() const { return m_videoWidth; }
int VideoWidget::videoHeight() const { return m_videoHeight; }
double VideoWidget::videoDAR() const { return m_videoDAR; }

void VideoWidget::setVideoResolution( int width, int height, double dar )
{
	if ( width <= 0 || height <= 0 )
	{
		kWarning() << "invalid video with or height reported";
		return;
	}

	m_videoWidth = width;
	m_videoHeight = height;
	m_videoDAR = dar > 0.0 ? dar : ((double)width/height);

	updateVideoLayerGeometry();
}

void VideoWidget::setMouseTracking( bool enable )
{
	QWidget::setMouseTracking( enable );
	m_videoLayer->setMouseTracking( enable );
}

void VideoWidget::resizeEvent( QResizeEvent* )
{
	updateVideoLayerGeometry();
}

void VideoWidget::mouseReleaseEvent( QMouseEvent* e )
{
	if ( e->button() == Qt::LeftButton )
	{
		e->accept();
		emit leftClicked( e->globalPos() );
	}
	else if ( e->button() == Qt::RightButton )
	{
		e->accept();
		emit rightClicked( e->globalPos() );
	}
	else
		e->ignore();
}

void VideoWidget::mouseDoubleClickEvent( QMouseEvent* e )
{
	e->accept();
	emit doubleClicked( e->globalPos() );
}

void VideoWidget::wheelEvent( QWheelEvent * e )
{
	e->accept();
	if ( e->delta() >= 0 )
		emit wheelUp();
	else
		emit wheelDown();
}

QSize VideoWidget::desktopSize()
{
	QRect rect = QApplication::desktop()->screenGeometry( this );
	return QSize( rect.width(), rect.height() );
}

double VideoWidget::desktopAspectRatio()
{
	QSize size = desktopSize();
	return (double)size.width() / size.height() ;
}

void VideoWidget::updateVideoLayerGeometry()
{
	QSize frameSize = size();

	int videoLayerX = 0;
	int videoLayerY = 0;
	int videoLayerWidth = frameSize.width();
	int videoLayerHeight = frameSize.height();

	if ( m_videoDAR > 0.0 )
	{
		if ( m_videoDAR >= ((double)videoLayerWidth / videoLayerHeight) )
		{
			videoLayerHeight = (int)(videoLayerWidth / m_videoDAR);
			videoLayerY = (frameSize.height() - videoLayerHeight) / 2;
		}
		else
		{
			videoLayerWidth = (int)(videoLayerHeight * m_videoDAR);
			videoLayerX = (frameSize.width() - videoLayerWidth) / 2;
		}
	}

	m_videoLayer->setGeometry( videoLayerX, videoLayerY, videoLayerWidth, videoLayerHeight );

	m_videoLayer->update();
	update();
}

QSize VideoWidget::sizeHint() const
{
	return QSize( m_videoWidth, m_videoHeight );
}

QSize VideoWidget::minimumSizeHint() const
{
	return QSize( 0, 0 );
}

#include "videowidget.moc"
