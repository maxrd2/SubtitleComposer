/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LAYEREDWIDGET_H
#define LAYEREDWIDGET_H

#include <QList>
#include <QWidget>

/// a class used to show varios widgets simultaneously one in top
/// of the another (unlike QStackWidget which shows one at a time)

class LayeredWidget : public QWidget
{
	Q_OBJECT

public:
	typedef enum { HandleResize, IgnoreResize } Mode;

	explicit LayeredWidget(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());

	void setWidgetMode(QWidget *widget, Mode mode);

public slots:
	void setMouseTracking(bool enable);

protected:
	void resizeEvent(QResizeEvent *e) override;

private:
	QList<QObject *> m_ignoredWidgets;
};

#endif
