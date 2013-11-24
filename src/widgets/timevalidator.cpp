/***************************************************************************
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

		timeMillis = hours.toInt() * 3600000 + mins.toInt() * 60000 + m_parserRegExp.cap(3).toInt() * 1000 + (int)pow(msecs.toInt(), 4 - msecs.length());

		return true;
	}
}
