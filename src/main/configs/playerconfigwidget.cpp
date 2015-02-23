/***************************************************************************
 *   Copyright (C) 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)  *
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

PlayerConfigWidget::PlayerConfigWidget(QWidget *parent) :
	AppConfigGroupWidget(new PlayerConfig(), parent, false)
{
	setupUi(this);

	m_textOverlayWidget->setText(i18nc("Text for previewing the subtitles font settings", "<p>The Quick Brown Fox<br/>Jumps Over The Lazy Dog</p>"));
	m_textOverlayWidget->setOutlineWidth(1);
	m_textOverlayWidget->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);

	connect(m_fontFamily, SIGNAL(activated(const QString &)), this, SLOT(onFamilyChanged(const QString &)));
	connect(m_fontFamily, SIGNAL(textChanged(const QString &)), this, SLOT(onFamilyChanged(const QString &)));
	connect(m_fontSize, SIGNAL(valueChanged(int)), this, SLOT(onSizeChanged(int)));
	connect(m_textColor, SIGNAL(activated(const QColor &)), this, SLOT(onPrimaryColorChanged(const QColor &)));
	connect(m_outlineColor, SIGNAL(activated(const QColor &)), this, SLOT(onOutlineColorChanged(const QColor &)));
	connect(m_outlineWidth, SIGNAL(valueChanged(int)), this, SLOT(onOutlineWidthChanged(int)));
	connect(m_textAntialias, SIGNAL(toggled(bool)), this, SLOT(onAntialiasChanged(bool)));

	connect(m_playerBackend, SIGNAL(activated(int)), this, SIGNAL(settingsChanged()));
	connect(m_decoderBackend, SIGNAL(activated(int)), this, SIGNAL(settingsChanged()));
	connect(m_seekJumpSecs, SIGNAL(valueChanged(int)), this, SIGNAL(settingsChanged()));
	connect(m_editabelPositionCtrl, SIGNAL(toggled(bool)), this, SIGNAL(settingsChanged()));

	connect(m_fontFamily, SIGNAL(activated(int)), this, SIGNAL(settingsChanged()));
	connect(m_fontSize, SIGNAL(valueChanged(int)), this, SIGNAL(settingsChanged()));
	connect(m_textColor, SIGNAL(activated(const QColor &)), this, SIGNAL(settingsChanged()));
	connect(m_outlineColor, SIGNAL(activated(const QColor &)), this, SIGNAL(settingsChanged()));
	connect(m_outlineWidth, SIGNAL(valueChanged(int)), this, SIGNAL(settingsChanged()));
	connect(m_textAntialias, SIGNAL(toggled(bool)), this, SIGNAL(settingsChanged()));

	setControlsFromConfig();
}

PlayerConfigWidget::~PlayerConfigWidget()
{}

void
PlayerConfigWidget::setControlsFromConfig()
{
	m_playerBackend->setEditText(config()->playerBackend());
	m_decoderBackend->setEditText(config()->decoderBackend());
	m_seekJumpSecs->setValue(config()->seekJumpLength());
	m_editabelPositionCtrl->setChecked(config()->showPositionTimeEdit());

	m_fontFamily->setCurrentFont(config()->fontFamily());
	onFamilyChanged(config()->fontFamily());

	m_fontSize->setValue(config()->fontPointSize());
	onSizeChanged(config()->fontPointSize());

	m_textColor->setColor(config()->fontColor());
	onPrimaryColorChanged(config()->fontColor());

	m_outlineColor->setColor(config()->outlineColor());
	onOutlineColorChanged(config()->outlineColor());

	m_outlineWidth->setValue(config()->outlineWidth());
	onOutlineWidthChanged(config()->outlineWidth());

	m_textAntialias->setChecked(config()->antialiasEnabled());
	onAntialiasChanged(config()->antialiasEnabled());
}

void
PlayerConfigWidget::setConfigFromControls()
{
	config()->setPlayerBackend(m_playerBackend->currentText());
	config()->setDecoderBackend(m_decoderBackend->currentText());
	config()->setSeekJumpLength(m_seekJumpSecs->value());
	config()->setShowPositionTimeEdit(m_editabelPositionCtrl->isChecked());

	config()->setFontFamily(m_textOverlayWidget->family());
	config()->setFontPointSize(m_textOverlayWidget->pointSize());
	config()->setFontColor(m_textOverlayWidget->primaryColor());
	config()->setOutlineColor(m_textOverlayWidget->outlineColor());
	config()->setOutlineWidth((int)m_textOverlayWidget->outlineWidth());
	config()->setAntialiasEnabled(m_textAntialias->isChecked());
}

void
PlayerConfigWidget::onFamilyChanged(const QString &family)
{
	if(m_fontFamily->findText(family) != -1)
		m_textOverlayWidget->setFamily(family);
}

void
PlayerConfigWidget::onSizeChanged(int size)
{
	m_textOverlayWidget->setPointSize(size);
}

void
PlayerConfigWidget::onPrimaryColorChanged(const QColor &color)
{
	m_textOverlayWidget->setPrimaryColor(color);
}

void
PlayerConfigWidget::onOutlineColorChanged(const QColor &color)
{
	m_textOverlayWidget->setOutlineColor(color);
}

void
PlayerConfigWidget::onOutlineWidthChanged(int width)
{
	m_textOverlayWidget->setOutlineWidth(width);
}

void
PlayerConfigWidget::onAntialiasChanged(bool antialias)
{
	m_textOverlayWidget->setAntialias(antialias);
}


