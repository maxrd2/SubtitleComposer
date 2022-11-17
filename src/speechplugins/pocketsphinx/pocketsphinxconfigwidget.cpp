/*
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "pocketsphinxconfigwidget.h"

using namespace SubtitleComposer;

PocketSphinxConfigWidget::PocketSphinxConfigWidget(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);
	kcfg_lexiconFile->setFilter(QStringLiteral("*.dict *.dic|Sphinx Dictionary (*.dict *.dic)\n*|All Files"));
	kcfg_trigramModelFile->setFilter(QStringLiteral("*.lm.bin *.lm|Trigram Models (*.lm.bin *.lm)\n*|All Files"));
}

PocketSphinxConfigWidget::~PocketSphinxConfigWidget()
{

}
