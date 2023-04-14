/*
    SPDX-FileCopyrightText: 2023 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef TRANSLATEENGINE_H
#define TRANSLATEENGINE_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QWidget>

namespace SubtitleComposer {

class TranslateEngine : public QObject
{
	Q_OBJECT

public:
	explicit TranslateEngine(QObject *parent=nullptr);

	virtual QString name() const = 0;

	virtual void settings(QWidget *widget) = 0;
	virtual void translate(QVector<QString> &textLines) = 0;

signals:
	void engineReady(bool status);
	void translated();
};
} // namespace SubtitleComposer

#endif // TRANSLATEENGINE_H
