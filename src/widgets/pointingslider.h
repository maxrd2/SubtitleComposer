/*
    smplayer, GUI front-end for mplayer.
    SPDX-FileCopyrightText: 2007 Ricardo Villalba <rvm@escomposlinux.org>

    modified for inclusion in Subtitle Composer
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef POINTINGSLIDER_H
#define POINTINGSLIDER_H

#include <QSlider>

class PointingSlider : public QSlider
{
	Q_OBJECT

public:
	explicit PointingSlider(QWidget *parent = 0);
	explicit PointingSlider(Qt::Orientation orientation, QWidget *parent = 0);

	virtual ~PointingSlider();

protected:
	void mousePressEvent(QMouseEvent *e) override;
};

#endif
