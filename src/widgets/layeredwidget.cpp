/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "layeredwidget.h"

#include <typeinfo>

#include <QResizeEvent>

LayeredWidget::LayeredWidget(QWidget *parent, Qt::WindowFlags f) :
	QWidget(parent, f)
{}

void
LayeredWidget::setWidgetMode(QWidget *widget, LayeredWidget::Mode mode)
{
	m_ignoredWidgets.removeAll(widget);
	if(mode == IgnoreResize)
		m_ignoredWidgets.append(widget);
}

void
LayeredWidget::setMouseTracking(bool enable)
{
	// propagates to our children and our children children's
	QWidget::setMouseTracking(enable);
	QList<QWidget *> children = findChildren<QWidget *>();
	for(QList<QWidget *>::ConstIterator it = children.constBegin(), end = children.constEnd(); it != end; ++it)
		(*it)->setMouseTracking(enable);
}

void
LayeredWidget::resizeEvent(QResizeEvent *)
{
	// propagated to our children but not our children children's
	QSize size = this->size();
	const QObjectList &children = this->children();
	for(QObjectList::ConstIterator it = children.begin(), end = children.end(); it != end; ++it) {
		if(!(*it)->isWidgetType() || m_ignoredWidgets.contains(*it))
			continue;
		(static_cast<QWidget *>(*it))->resize(size);
	}
}


