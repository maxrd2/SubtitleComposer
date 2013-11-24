/***************************************************************************
 *   smplayer, GUI front-end for mplayer.                                  *
 *   Copyright (C) 2007 Ricardo Villalba (rvm@escomposlinux.org)           *
 *                                                                         *
 *   modified for inclusion in Subtitle Composer                           *
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

#include "pointingslider.h"

#include <QtGui/QApplication>
#include <QtGui/QStyle>
#include <QtGui/QMouseEvent>

PointingSlider::PointingSlider(QWidget *parent) :
	QSlider(parent)
{}

PointingSlider::PointingSlider(Qt::Orientation orientation, QWidget *parent) :
	QSlider(orientation, parent)
{}

PointingSlider::~PointingSlider()
{}

// The code from the following function is from Javier Díaz,
// taken from a post in the Qt-interest mailing list.
void
PointingSlider::mousePressEvent(QMouseEvent *e)
{
	int range = maximum() - minimum();

	int clickedValue;                       // this will contain the value corresponding to the clicked (x,y) screen position
	double pixelsPerUnit;           // the will contain the amount of pixels corresponding to each slider range unit

	if(orientation() == Qt::Horizontal) {
		int width = this->width();
		pixelsPerUnit = (double)width / range;
		clickedValue = (e->x() * range) / width;

		if((qApp->isRightToLeft() && !invertedAppearance()) || (!qApp->isRightToLeft() && invertedAppearance()))
			clickedValue = maximum() - clickedValue;
	} else {
		int height = this->height();
		pixelsPerUnit = (double)height / range;
		clickedValue = (e->y() * range) / height;

		if(!invertedAppearance())
			clickedValue = maximum() - clickedValue;
	}

	// calculate how many range units take the slider handle
	int sliderHandleUnits = (int)(qApp->style()->pixelMetric(QStyle::PM_SliderLength) / pixelsPerUnit);

	// if the condition is is not true, the user has clicked inside the slider
	// (or a screen position with a corresponding range value inside the slider tolerance)
	if(qAbs(clickedValue - value()) > sliderHandleUnits) {
		setValue(clickedValue);
		emit sliderMoved(clickedValue);
	} else {
		QSlider::mousePressEvent(e);
	}
}

#include "pointingslider.moc"
