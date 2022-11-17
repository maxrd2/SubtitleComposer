/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "intinputdialog.h"

#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QSlider>
#include <QDialogButtonBox>
#include <QPushButton>

using namespace SubtitleComposer;

IntInputDialog::IntInputDialog(const QString &caption, const QString &label, QWidget *parent)
	: QDialog(parent)
{
	init(caption, label, 0, 100, 0);
}

IntInputDialog::IntInputDialog(const QString &caption, const QString &label, int min, int max, QWidget *parent)
	: QDialog(parent)
{
	init(caption, label, min, max, min);
}

IntInputDialog::IntInputDialog(const QString &caption, const QString &label, int min, int max, int value, QWidget *parent)
	: QDialog(parent)
{
	init(caption, label, min, max, value);
}

void
IntInputDialog::init(const QString &caption, const QString &labelText, int min, int max, int value)
{
	setupUi(this);

	setWindowTitle(caption);

	slider->setRange(min, max);
	slider->setValue(value);

	spinBox->setRange(min, max);
	spinBox->setValue(value);
	spinBox->setFocus();

	int step = (max - min) / 10;
	slider->setTickInterval(step);
	slider->setPageStep(step);

	label->setText(labelText);

	resize(300, 10);
}

int
IntInputDialog::minimum() const
{
	return slider->minimum();
}

void
IntInputDialog::setMinimum(int minimum)
{
	slider->setMinimum(minimum);
	spinBox->setMinimum(minimum);

	int step = (maximum() - minimum) / 10;
	slider->setTickInterval(step);
	slider->setPageStep(step);
}

int
IntInputDialog::maximum() const
{
	return slider->maximum();
}

void
IntInputDialog::setMaximum(int maximum)
{
	slider->setMaximum(maximum);
	spinBox->setMaximum(maximum);

	int step = (maximum - minimum()) / 10;
	slider->setTickInterval(step);
	slider->setPageStep(step);
}

int
IntInputDialog::value() const
{
	return slider->value();
}

void
IntInputDialog::setValue(int value)
{
	slider->setValue(value);
	spinBox->setValue(value);
}


