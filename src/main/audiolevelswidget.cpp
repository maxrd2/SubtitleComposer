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

#include "audiolevelswidget.h"
#include "../core/subtitleline.h"
#include "../player/player.h"

#include <QtCore/QRect>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QRegion>
#include <QtGui/QPolygon>

#include <KDebug>

using namespace SubtitleComposer;

AudioLevelsWidget::AudioLevelsWidget( QWidget* parent ):
	QWidget( parent ),
	m_subtitle( 0 ),
	m_audiolevels( 0 ),
	m_lowerPosition( 0 ),
	m_playingPosition( 5000 ),
	m_upperPosition( 10000 ),
	m_regions( 0 ),
	m_points( 0 ),
	m_playingX( -1 )
{
}

AudioLevelsWidget::~AudioLevelsWidget()
{
	delete [] m_regions;
	delete [] m_points;
}

void AudioLevelsWidget::loadConfig()
{
}

void AudioLevelsWidget::saveConfig()
{
}

Time AudioLevelsWidget::windowSize() const
{
	return m_upperPosition - m_lowerPosition;
}

void AudioLevelsWidget::setWindowSize( const Time& size )
{
	if ( size != windowSize() )
	{
		m_upperPosition = m_lowerPosition + size;

		rebuildRegions();
		update();
	}
}

void AudioLevelsWidget::setSubtitle( Subtitle* subtitle )
{
	m_subtitle = subtitle;
}

void AudioLevelsWidget::setAudioLevels( AudioLevels* audiolevels )
{
	delete [] m_regions;
	m_regions = 0;
	delete [] m_points;
	m_points = 0;
	m_playingX = -1;

	disconnect( Player::instance(), SIGNAL(positionChanged(double)), this, SLOT(onPlayerPositionChanged(double)) );

	m_audiolevels = audiolevels;

	if ( m_audiolevels )
	{
		connect( Player::instance(), SIGNAL(positionChanged(double)), this, SLOT(onPlayerPositionChanged(double)) );

		m_regions = new QRegion[m_audiolevels->channelsCount()];
		m_points = new QPolygon[m_audiolevels->channelsCount()];

		rebuildRegions();
	}

	update();
}

void AudioLevelsWidget::paintEvent( QPaintEvent* e )
{
	QPainter painter( this );

	QRect rect = this->rect();

	painter.fillRect( e->rect(), Qt::white );

	if ( ! m_audiolevels )
		return;

	painter.drawText( rect, Qt::AlignLeft|Qt::AlignTop, m_lowerPosition.toString() );
	painter.drawText( rect, Qt::AlignRight|Qt::AlignTop, m_upperPosition.toString() );

	for ( unsigned channel = 0; channel < m_audiolevels->channelsCount(); ++channel )
	{
	/*	painter.setClipRegion( m_regions[channel] );
		painter.fillRect( rect, Qt::red );
		painter.setClipping( false );*/

		painter.setPen( QPen( Qt::green, 2, Qt::SolidLine ) );
		const unsigned size = m_points[channel].size();
		for ( unsigned index = 1; index < size; ++index )
			painter.drawLine( m_points[channel].point( index-1 ), m_points[channel].point( index-1 ) );

		painter.setPen( QPen( Qt::red, 4, Qt::SolidLine ) );
		painter.drawPolygon( m_points[channel] );
	}

	if ( m_playingX >= 0 )
		painter.drawLine( m_playingX, 0, m_playingX, height() );
}

void AudioLevelsWidget::resizeEvent( QResizeEvent* e )
{
	QWidget::resizeEvent( e );

	if ( m_audiolevels )
	{
		rebuildRegions();
		update();
	}
}

void AudioLevelsWidget::onPlayerPositionChanged( double seconds )
{
	Time playingPosition;
	playingPosition.setSecondsTime( seconds );

	// FIXME we have to deal with playingPosition being larger than audiolevels length

	if ( m_playingPosition != playingPosition )
	{
		m_playingPosition = playingPosition;

		bool rebuild = false;
		Time windowSize = this->windowSize();

		if ( m_playingPosition > m_upperPosition )
		{
			m_lowerPosition = m_playingPosition - 2000; // 2000 can't be harcoded (must be relative to windowSize)
			m_upperPosition = m_lowerPosition + windowSize;
			rebuild = true;
		}
		else if ( m_playingPosition < m_lowerPosition )
		{
			m_lowerPosition = m_playingPosition;
			m_upperPosition = m_lowerPosition + windowSize;
			rebuild = true;
		}

		m_playingX = width()*(m_playingPosition - m_lowerPosition).toMillis()/windowSize.toMillis();

		if ( rebuild )
			rebuildRegions();

		update();
	}
}

#define ABS( x ) ((x) >= 0 ? (-x) : (x))

void AudioLevelsWidget::rebuildRegions()
{
	const unsigned lowerSample = m_audiolevels->sampleForPosition( m_lowerPosition );
	const unsigned upperSample = m_audiolevels->sampleForPosition( m_upperPosition );

	const int width = this->width();
	const int channelHeight = this->height()/m_audiolevels->channelsCount();
	const int heightScale = channelHeight*10;

	if ( lowerSample == upperSample )
	{
		for ( unsigned channel = 0; channel < m_audiolevels->channelsCount(); ++channel )
		{
			const unsigned midHeight = channel*channelHeight + channelHeight/2;

			int y = (int)(ABS(m_audiolevels->dataBySample( channel, lowerSample )*heightScale)) + midHeight;

			m_points[channel].resize( 2 );
			m_points[channel].setPoint( 0, QPoint( 0, y ) );
			m_points[channel].setPoint( 1, QPoint( width, y ) );

			QPolygon regionPoints( 5 );
			regionPoints.setPoint( 0, QPoint( 0, midHeight ) );
			regionPoints.setPoint( 1, m_points[channel].point( 0 ) );
			regionPoints.setPoint( 2, m_points[channel].point( 1 ) );
			regionPoints.setPoint( 3, QPoint( width, midHeight ) );
			regionPoints.setPoint( 4, QPoint( 0, midHeight ) );

			m_regions[channel] = QRegion( regionPoints );
		}
	}
	else // samples >= 2
	{
		const unsigned samples = upperSample - lowerSample + 1;
		const double stepX = width/samples;

		for ( unsigned channel = 0; channel < m_audiolevels->channelsCount(); ++channel )
		{
			const unsigned midHeight = channel*channelHeight + channelHeight/2;

			m_points[channel].resize( samples );

			QPolygon regionPoints( samples + 3 );
			regionPoints.setPoint( 0, QPoint( 0, midHeight ) );

			for ( unsigned offset = 0; offset < samples; ++offset )
			{
				QPoint point(
					(int)((offset+0.5)*stepX),
					(int)(ABS(m_audiolevels->dataBySample( channel, offset + lowerSample )*heightScale)) + midHeight
				);

				m_points[channel].setPoint( offset, point );
				regionPoints.setPoint( offset + 1, point );
			}

			regionPoints.setPoint( samples + 1, QPoint( width, midHeight ) );
			regionPoints.setPoint( samples + 2, QPoint( 0, midHeight ) );

			m_regions[channel] = QRegion( regionPoints );
		}
	}
}


#include "audiolevelswidget.moc"
