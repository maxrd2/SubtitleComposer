/*
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "pocketsphinxconfigwidget.h"

#include <KLocalizedString>
#include <kio_version.h>

using namespace SubtitleComposer;

PocketSphinxConfigWidget::PocketSphinxConfigWidget(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);
#if KIO_VERSION >= QT_VERSION_CHECK(5, 108, 0)
	kcfg_lexiconFile->setNameFilters({
		i18n("Sphinx Dictionaries") + QLatin1String(" (*.dict *.dic)"),
		i18n("All Files") + QLatin1String(" (*)"),
	});
	kcfg_trigramModelFile->setNameFilters({
		i18n("Trigram Models") + QStringLiteral(" (*.lm.bin *.lm)"),
		i18n("All Files") + QLatin1String(" (*)"),
	});
#else
	kcfg_lexiconFile->setFilter(QLatin1String("*.dict *.dic|") + i18n("Sphinx Dictionaries") + QLatin1String("\n*|") + i18n("All Files"));
	kcfg_trigramModelFile->setFilter(QStringLiteral("*.lm.bin *.lm|") + i18n("Trigram Models") + QLatin1String("\n*|") + i18n("All Files"));
#endif
}

PocketSphinxConfigWidget::~PocketSphinxConfigWidget()
{

}
