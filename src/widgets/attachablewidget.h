#ifndef ATTACHABLEWIDGET_H
#define ATTACHABLEWIDGET_H

/***************************************************************************
 *   smplayer, GUI front-end for mplayer.                                  *
 *   Copyright (C) 2006-2008 Ricardo Villalba (rvm@escomposlinux.org)      *
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

#include <QtGui/QWidget>

class AttachableWidget : public QWidget
{
	Q_OBJECT

public:
	typedef enum { Top, Bottom } Place;

	explicit AttachableWidget(Place place = Bottom, unsigned animStepDuration = 4);
	virtual ~AttachableWidget();

	bool isAttached() const;

	bool isAnimated() const;
	int animStepDuration() const;

	virtual bool eventFilter(QObject *object, QEvent *event);

public slots:
	void attach(QWidget *target);
	void dettach();

	void setAnimStepDuration(int stepDuration);

	void toggleVisible(bool visible);

protected:
	virtual void timerEvent(QTimerEvent *event);

private:
	void toggleVisible(bool visible, bool force);

private:
	QWidget *m_targetWidget;
	Place m_place;
	int m_animStepDuration;

	typedef enum { Upward, Downward } Direction;

	int m_animTID;
	bool m_animHiding;
	int m_animFinalY;
	int m_animCurrentY;
	Direction m_animDirection;
};

#endif
