/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "timevalidator.h"

#include <math.h>

//TimeValidator::TimeValidator(QObject *parent) :
//	QRegExpValidator(
//		QRegExp(
//			"^([0-2]?[0-9]?:[0-5]?[0-9]?:[0-5]?[0-9]?\\.?[0-9]{0,3}|"
//			"[0-5]?[0-9]?:[0-5]?[0-9]?\\.?[0-9]{0,3}|"
//			"[0-5]?[0-9]?\\.?[0-9]{0,3})$"
//			),
//		parent
//		),
//	m_parserRegExp( "^([0-2]?[0-9]:|)([0-5]?[0-9]:|)([0-5]?[0-9]?|)(\\.[0-9]{1,3}|)$" )
//{}

TimeValidator::TimeValidator(QObject *parent) :
	QValidator(parent),
	m_parserRegExp("^([0-2]?[0-9]:|)([0-5]?[0-9]:|)([0-5]?[0-9]?|)(\\.[0-9]{1,3}|)$")
{}

//QValidator::State TimeValidator::validate(QString &input, int &pos) const
//{
//	return input.isEmpty() ? QValidator::Intermediate : QRegExpValidator::validate(input, pos);
//}

QValidator::State
TimeValidator::validate(QString &input, int &/*pos*/) const
{
	if(input.isEmpty())
		return QValidator::Intermediate;

	if(input.contains(QRegExp("[^0-9:\\.]")))
		return QValidator::Invalid;

	return m_parserRegExp.indexIn(input) == -1 ? QValidator::Intermediate : QValidator::Acceptable;
}

bool
TimeValidator::parse(const QString &input, int &timeMillis)
{
	if(input.isEmpty() || m_parserRegExp.indexIn(input) == -1)
		return false;
	else {
		QString hours = m_parserRegExp.cap(1);
		if(hours.length())
			hours = hours.left(hours.length() - 1);

		QString mins = m_parserRegExp.cap(2);
		if(mins.length())
			mins = mins.left(mins.length() - 1);
		else {
			mins = hours;
			hours.clear();
		}

		QString msecs = m_parserRegExp.cap(4).mid(1);

		timeMillis = hours.toInt() * 3600000 + mins.toInt() * 60000 + m_parserRegExp.cap(3).toInt() * 1000 + static_cast<int>(pow(msecs.toInt(), 4 - msecs.length()));

		return true;
	}
}
