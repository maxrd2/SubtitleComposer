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
	});
	connect(alt_MinDurationPerCharacter, QOverload<int>::of(&QSpinBox::valueChanged), [this](int val){
		QSignalBlocker b(kcfg_MinDurationPerCharacter);
		kcfg_MinDurationPerCharacter->setValue(1000 / val);
	});
	connect(kcfg_MaxDurationPerCharacter, QOverload<int>::of(&QSpinBox::valueChanged), [this](int val){
		QSignalBlocker b(alt_MaxDurationPerCharacter);
		alt_MaxDurationPerCharacter->setValue(1000 / val);
	});
	connect(alt_MaxDurationPerCharacter, QOverload<int>::of(&QSpinBox::valueChanged), [this](int val){
		QSignalBlocker b(kcfg_MaxDurationPerCharacter);
		kcfg_MaxDurationPerCharacter->setValue(1000 / val);
	});
}

ErrorsConfigWidget::~ErrorsConfigWidget()
{

}
