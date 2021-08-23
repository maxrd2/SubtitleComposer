/*
    smplayer, GUI front-end for mplayer.
    SPDX-FileCopyrightText: 2007 Ricardo Villalba <rvm@escomposlinux.org>

    modified for inclusion in Subtitle Composer
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "pointingslider.h"

#include <QApplication>
#include <QStyle>
#include <QMouseEvent>

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


