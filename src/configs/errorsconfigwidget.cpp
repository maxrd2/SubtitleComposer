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

#include "errorsconfigwidget.h"
#include "widgets/timeedit.h"

#include <QSignalBlocker>

using namespace SubtitleComposer;

ErrorsConfigWidget::ErrorsConfigWidget(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);

	connect(kcfg_MinDurationPerCharacter, QOverload<int>::of(&QSpinBox::valueChanged), [this](int val){
		QSignalBlocker b(alt_MinDurationPerCharacter);
		alt_MinDurationPerCharacter->setValue(1000 / val);
		kcfg_IdealDurationPerCharacter->setMinimum(val);
		alt_IdealDurationPerCharacter->setMaximum(1000 / val);
	});
	connect(alt_MinDurationPerCharacter, QOverload<int>::of(&QSpinBox::valueChanged), [this](int val){
		QSignalBlocker b(kcfg_MinDurationPerCharacter);
		kcfg_MinDurationPerCharacter->setValue(1000 / val);
		alt_IdealDurationPerCharacter->setMaximum(val);
		kcfg_IdealDurationPerCharacter->setMinimum(1000 / val);
	});

	connect(kcfg_IdealDurationPerCharacter, QOverload<int>::of(&QSpinBox::valueChanged), [this](int val){
		QSignalBlocker b(alt_IdealDurationPerCharacter);
		alt_IdealDurationPerCharacter->setValue(1000 / val);
		kcfg_MinDurationPerCharacter->setMaximum(val);
		kcfg_MaxDurationPerCharacter->setMinimum(val);
		alt_MinDurationPerCharacter->setMinimum(1000 / val);
		alt_MaxDurationPerCharacter->setMaximum(1000 / val);
	});
	connect(alt_IdealDurationPerCharacter, QOverload<int>::of(&QSpinBox::valueChanged), [this](int val){
		QSignalBlocker b(kcfg_IdealDurationPerCharacter);
		kcfg_IdealDurationPerCharacter->setValue(1000 / val);
		alt_MinDurationPerCharacter->setMinimum(val);
		alt_MaxDurationPerCharacter->setMaximum(val);
		kcfg_MinDurationPerCharacter->setMaximum(1000 / val);
		kcfg_MaxDurationPerCharacter->setMinimum(1000 / val);
	});

	connect(kcfg_MaxDurationPerCharacter, QOverload<int>::of(&QSpinBox::valueChanged), [this](int val){
		QSignalBlocker b(alt_MaxDurationPerCharacter);
		alt_MaxDurationPerCharacter->setValue(1000 / val);
		kcfg_IdealDurationPerCharacter->setMaximum(val);
		alt_IdealDurationPerCharacter->setMinimum(1000 / val);
	});
	connect(alt_MaxDurationPerCharacter, QOverload<int>::of(&QSpinBox::valueChanged), [this](int val){
		QSignalBlocker b(kcfg_MaxDurationPerCharacter);
		kcfg_MaxDurationPerCharacter->setValue(1000 / val);
		alt_IdealDurationPerCharacter->setMinimum(val);
		kcfg_IdealDurationPerCharacter->setMaximum(1000 / val);
	});
}

ErrorsConfigWidget::~ErrorsConfigWidget()
{

}
