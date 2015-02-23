/***************************************************************************
 *   Copyright (C) 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)  *
 *   Copyright (C) 2013-2015 Mladen Milinković                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#include "playerconfigwidget.h"

#include "../../services/player.h"
#include "../../services/decoder.h"

#include "../../widgets/layeredwidget.h"
#include "../../widgets/textoverlaywidget.h"

#include <KLocale>

using namespace SubtitleComposer;

PlayerConfigWidget::PlayerConfigWidget(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);

	m_textOverlayWidget->setText(i18nc("Text for previewing the subtitles font settings", "<p>The Quick Brown Fox<br/>Jumps Over The Lazy Dog</p>"));
	m_textOverlayWidget->setOutlineWidth(1);
	m_textOverlayWidget->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);

	kcfg_PlayerBackend->addItems(Player::instance()->backendNames());
	kcfg_PlayerBackend->setProperty("kcfg_property", QByteArray("currentText"));
	if(kcfg_PlayerBackend->count() > 1) {
		int dummyBackendIndex = kcfg_PlayerBackend->findText(Player::instance()->dummyBackendName());
		if(dummyBackendIndex >= 0)
			kcfg_PlayerBackend->removeItem(dummyBackendIndex);
	}

	kcfg_DecoderBackend->addItems(Decoder::instance()->backendNames());
	kcfg_DecoderBackend->setProperty("kcfg_property", QByteArray("currentText"));
	if(kcfg_DecoderBackend->count() > 1) {
		int dummyBackendIndex = kcfg_DecoderBackend->findText(Decoder::instance()->dummyBackendName());
		if(dummyBackendIndex >= 0)
			kcfg_DecoderBackend->removeItem(dummyBackendIndex);
	}

	kcfg_FontFamily->setProperty("kcfg_property", QByteArray("currentText"));
}

PlayerConfigWidget::~PlayerConfigWidget()
{}
