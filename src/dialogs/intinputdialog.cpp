/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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


