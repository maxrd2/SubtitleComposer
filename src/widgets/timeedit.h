/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TIMEEDIT_H
#define TIMEEDIT_H

#include <QTimeEdit>

QT_FORWARD_DECLARE_CLASS(QEvent)

class TimeEdit : public QTimeEdit
{
	Q_OBJECT

public:
	TimeEdit(QWidget *parent = 0);

	int msecsStep() const;

	int value() const;

	void stepBy(int steps) override;

public slots:
	void setMSecsStep(int msecs);
	void setValue(int value);

signals:
	void valueChanged(int value);
	void valueEntered(int value);

protected slots:
	void onTimeChanged(const QTime &time);

protected:
	StepEnabled stepEnabled() const override;
	void keyPressEvent(QKeyEvent *event) override;

private:
	int m_secsStep;
};

#endif
