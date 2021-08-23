#ifndef ATTACHABLEWIDGET_H
#define ATTACHABLEWIDGET_H

/***************************************************************************
 *   smplayer, GUI front-end for mplayer.                                  *
 *   SPDX-FileCopyrightText: 2006-2008 Ricardo Villalba (rvm@escomposlinux.org)      *
 *                                                                         *
 *   modified for inclusion in Subtitle Composer                           *
 *   SPDX-FileCopyrightText: 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)  *
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 ***************************************************************************/

#include <QWidget>

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

	bool eventFilter(QObject *object, QEvent *event) override;

public slots:
	void attach(QWidget *target);
	void dettach();

	void setAnimStepDuration(int stepDuration);

	void toggleVisible(bool visible);

protected:
	void timerEvent(QTimerEvent *event) override;

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
