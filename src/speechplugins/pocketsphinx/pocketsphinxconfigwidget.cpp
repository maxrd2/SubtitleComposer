/*
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "pocketsphinxconfigwidget.h"

#include <KLocalizedString>

using namespace SubtitleComposer;

PocketSphinxConfigWidget::PocketSphinxConfigWidget(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);
	kcfg_lexiconFile->setFilter(QLatin1String("*.dict *.dic|") + i18n("Sphinx Dictionaries") + QLatin1String("\n*|") + i18n("All Files"));
	kcfg_trigramModelFile->setFilter(QStringLiteral("*.lm.bin *.lm|") + i18n("Trigram Models") + QLatin1String("\n*|") + i18n("All Files"));
}

PocketSphinxConfigWidget::~PocketSphinxConfigWidget()
{

}
