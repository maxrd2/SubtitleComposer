/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "errorsconfigwidget.h"
#include "widgets/timeedit.h"

#include <QSignalBlocker>

using namespace SubtitleComposer;

ErrorsConfigWidget::ErrorsConfigWidget(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);

	connect(kcfg_MinDurationPerCharacter, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int val){
		QSignalBlocker b(alt_MinDurationPerCharacter);
		alt_MinDurationPerCharacter->setValue(1000 / val);
		kcfg_IdealDurationPerCharacter->setMinimum(val);
		alt_IdealDurationPerCharacter->setMaximum(1000 / val);
	});
	connect(alt_MinDurationPerCharacter, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int val){
		QSignalBlocker b(kcfg_MinDurationPerCharacter);
		kcfg_MinDurationPerCharacter->setValue(1000 / val);
		alt_IdealDurationPerCharacter->setMaximum(val);
		kcfg_IdealDurationPerCharacter->setMinimum(1000 / val);
	});

	connect(kcfg_IdealDurationPerCharacter, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int val){
		QSignalBlocker b(alt_IdealDurationPerCharacter);
		alt_IdealDurationPerCharacter->setValue(1000 / val);
		kcfg_MinDurationPerCharacter->setMaximum(val);
		kcfg_MaxDurationPerCharacter->setMinimum(val);
		alt_MinDurationPerCharacter->setMinimum(1000 / val);
		alt_MaxDurationPerCharacter->setMaximum(1000 / val);
	});
	connect(alt_IdealDurationPerCharacter, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int val){
		QSignalBlocker b(kcfg_IdealDurationPerCharacter);
		kcfg_IdealDurationPerCharacter->setValue(1000 / val);
		alt_MinDurationPerCharacter->setMinimum(val);
		alt_MaxDurationPerCharacter->setMaximum(val);
		kcfg_MinDurationPerCharacter->setMaximum(1000 / val);
		kcfg_MaxDurationPerCharacter->setMinimum(1000 / val);
	});

	connect(kcfg_MaxDurationPerCharacter, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int val){
		QSignalBlocker b(alt_MaxDurationPerCharacter);
		alt_MaxDurationPerCharacter->setValue(1000 / val);
		kcfg_IdealDurationPerCharacter->setMaximum(val);
		alt_IdealDurationPerCharacter->setMinimum(1000 / val);
	});
	connect(alt_MaxDurationPerCharacter, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int val){
		QSignalBlocker b(kcfg_MaxDurationPerCharacter);
		kcfg_MaxDurationPerCharacter->setValue(1000 / val);
		alt_IdealDurationPerCharacter->setMinimum(val);
		kcfg_IdealDurationPerCharacter->setMaximum(1000 / val);
	});
}

ErrorsConfigWidget::~ErrorsConfigWidget()
{

}
