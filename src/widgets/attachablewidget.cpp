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

#include "attachablewidget.h"

#include <QtCore/QEvent>
#include <QtCore/QTimerEvent>
#include <QtGui/QMouseEvent>

#include <KDebug>

AttachableWidget::AttachableWidget(AttachableWidget::Place place, unsigned animStepDuration) :
	// QWidget(0, Qt::Window | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint),
	QWidget(0),
	m_targetWidget(0),
	m_place(place),
	m_animStepDuration(animStepDuration),
	m_animHiding(true)
{
//	m_animStepDuration = 0;

	hide();

//	QPalette palette;
//	palette.setColor(backgroundRole(), Qt::red);
//	setPalette(palette);

//	setAttribute(Qt::WA_TranslucentBackground, false);
}

AttachableWidget::~AttachableWidget()
{}

bool
AttachableWidget::isAttached() const
{
	return m_targetWidget != 0;
}

bool
AttachableWidget::isAnimated() const
{
	return m_animStepDuration > 0;
}

int
AttachableWidget::animStepDuration() const
{
	return m_animStepDuration;
}

void
AttachableWidget::setAnimStepDuration(int duration)
{
	m_animStepDuration = duration;
}

void
AttachableWidget::attach(QWidget *targetWidget)
{
	if(m_targetWidget != targetWidget) {
		if(m_targetWidget) {
			kWarning() << "attach attempted but already attached to another widget";
			return;
		}

		setParent(targetWidget);

		m_targetWidget = targetWidget;

		m_targetWidget->installEventFilter(this);

		show();
	}
}

void
AttachableWidget::dettach()
{
	if(m_targetWidget) {
		m_targetWidget->removeEventFilter(this);
		m_targetWidget = 0;

		m_animHiding = true;    // reset the flag

		hide();

		setParent(0);
	} else
		kWarning() << "dettach attempted but not attached to any widget";
}

void
AttachableWidget::timerEvent(QTimerEvent *event)
{
	if(event->timerId() == m_animTID) {     // Advance animation
		if(m_animCurrentY == m_animFinalY) {
			killTimer(m_animTID);
			m_animTID = 0;
			if(m_animHiding)
				hide();
		} else {
			if(m_animDirection == Upward)
				m_animCurrentY--;
			else
				m_animCurrentY++;
			move(x(), m_animCurrentY);
		}
	}
}

void
AttachableWidget::toggleVisible(bool visible)
{
	toggleVisible(visible, false);
}

void
AttachableWidget::toggleVisible(bool visible, bool force)
{
	if(!force && visible != m_animHiding)
		return;

	if(m_animTID) {
		killTimer(m_animTID);
		m_animTID = 0;
	}

	m_animHiding = !visible;

	resize(m_targetWidget->width(), height());

	QPoint targetPos = m_targetWidget->mapToGlobal(QPoint(0, 0));

	if(visible) {                                   // we have to show the widget
		m_animFinalY = m_place == Top ? targetPos.y() : targetPos.y() + m_targetWidget->height() - height();
		m_animCurrentY = m_place == Top ? m_animFinalY - height() : m_animFinalY + height();
		m_animDirection = m_place == Top ? Downward : Upward;
	} else {                                        // we have to hide the widget
		m_animFinalY = m_place == Top ? targetPos.y() - height() : targetPos.y() + m_targetWidget->height();
		m_animCurrentY = m_place == Top ? m_animFinalY + height() : m_animFinalY - height();
		m_animDirection = m_place == Top ? Upward : Downward;
	}

	if(isAnimated()) {
		move(targetPos.x(), m_animCurrentY);
		show();
		raise();

		m_animTID = startTimer(m_animStepDuration);     // start the animation
	} else {
		move(targetPos.x(), m_animFinalY);
		show();
		raise();
	}
}

bool
AttachableWidget::eventFilter(QObject *object, QEvent *event)
{
	if(object == m_targetWidget) {
		if(event->type() == QEvent::Resize || event->type() == QEvent::Move) {
			if(isVisible()) {
				// hide, and show with recalculated sizes and positions
				hide();
				toggleVisible(true, true);
			}
		}
	}

	return QWidget::eventFilter(object, event);
}

#include "attachablewidget.moc"
