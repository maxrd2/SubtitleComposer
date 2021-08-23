/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "timeedit.h"
#include "timevalidator.h"

#include <QTime>
#include <QEvent>
#include <QKeyEvent>

#include <QDebug>

TimeEdit::TimeEdit(QWidget *parent) :
	QTimeEdit(parent),
	m_secsStep(100)
{
	setDisplayFormat("hh:mm:ss.zzz");
	setTimeRange(QTime(0, 0, 0, 0), QTime(23, 59, 59, 999));
	setWrapping(false);
	setAlignment(Qt::AlignHCenter);
	setCorrectionMode(QAbstractSpinBox::CorrectToNearestValue);
	setCurrentSection(QDateTimeEdit::MSecSection);

	connect(this, &QDateTimeEdit::timeChanged, this, &TimeEdit::onTimeChanged);
}

int
TimeEdit::msecsStep() const
{
	return m_secsStep;
}

void
TimeEdit::setMSecsStep(int msecs)
{
	m_secsStep = msecs;
}

int
TimeEdit::value() const
{
	return QTime(0, 0, 0, 0).msecsTo(time());
}

void
TimeEdit::setValue(int value)
{
	if(wrapping()) {
		setTime(QTime(0, 0, 0, 0).addMSecs(value));
	} else {
		if(value < QTime(0, 0, 0, 0).msecsTo(minimumTime()))
			setTime(minimumTime());
		else if(value >= QTime(0, 0, 0, 0).msecsTo(maximumTime()))
			setTime(maximumTime());
		else
			setTime(QTime(0, 0, 0, 0).addMSecs(value));
	}
}

void
TimeEdit::onTimeChanged(const QTime &time)
{
	emit valueChanged(QTime(0, 0, 0, 0).msecsTo(time));
}

void
TimeEdit::stepBy(int steps)
{
	int oldValue = value();
	int newValue;

	setSelectedSection(currentSection());

	switch(currentSection()) {
	case QDateTimeEdit::MSecSection:
		newValue = oldValue + m_secsStep * steps;
		break;
	case QDateTimeEdit::SecondSection:
		newValue = oldValue + 1000 * steps;
		break;
	case QDateTimeEdit::MinuteSection:
		newValue = oldValue + 60000 * steps;
		break;
	case QDateTimeEdit::HourSection:
		newValue = oldValue + 3600000 * steps;
		break;
	default:
		return;
	}

	if(oldValue != newValue)
		setValue(newValue);
}

TimeEdit::StepEnabled
TimeEdit::stepEnabled() const
{
	if(wrapping()) {
		return QAbstractSpinBox::StepUpEnabled | QAbstractSpinBox::StepDownEnabled;
	} else {
		QTime time = this->time();
		if(time == minimumTime())
			return QAbstractSpinBox::StepUpEnabled;
		else if(time == maximumTime())
			return QAbstractSpinBox::StepDownEnabled;
		else
			return QAbstractSpinBox::StepUpEnabled | QAbstractSpinBox::StepDownEnabled;
	}
}

void
TimeEdit::keyPressEvent(QKeyEvent *event)
{
	int key = event->key();
	if(key == Qt::Key_Return) {
		emit valueEntered(QTime(0, 0, 0, 0).msecsTo(time()));
	} else if(key == Qt::Key_Up) {
		stepUp();
	} else if(key == Qt::Key_Down) {
		stepDown();
	} else if(key == Qt::Key_PageUp) {
		setCurrentSection(QDateTimeEdit::MSecSection);
		stepUp();
	} else if(key == Qt::Key_PageDown) {
		setCurrentSection(QDateTimeEdit::MSecSection);
		stepDown();
	} else {
		QTimeEdit::keyPressEvent(event);
	}
}


