/*
 * Copyright (C) 2010-2019 Mladen Milinkovic <max@smoothware.net>
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
