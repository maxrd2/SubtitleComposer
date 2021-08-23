/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TIMEVALIDATOR_H
#define TIMEVALIDATOR_H

#include <QString>
#include <QValidator>

// class TimeValidator : public QRegExpValidator
class TimeValidator : public QValidator
{
public:
	TimeValidator(QObject *parent = 0);

	bool parse(const QString &input, int &timeMillis);

	QValidator::State validate(QString &input, int &pos) const override;

private:
	mutable QRegExp m_parserRegExp;
};

#endif
