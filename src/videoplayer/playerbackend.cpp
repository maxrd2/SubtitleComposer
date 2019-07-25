/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
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

#include "playerbackend.h"
#include "videowidget.h"

#include <QDebug>

using namespace SubtitleComposer;

PlayerBackend::PlayerBackend()
	: QObject(NULL),
	m_player(NULL)
{}

PlayerBackend::~PlayerBackend()
{}

bool
PlayerBackend::isInitialized() const
{
	return m_player->activeBackend() == this;
}

bool
PlayerBackend::isActiveBackend() const
{
	return m_player->activeBackend() == this;
}

void
PlayerBackend::setConfig()
{
	if(isActiveBackend())
		m_player->reinitialize();
}

bool
PlayerBackend::isDummy() const
{
	return m_name == m_player->dummyBackendName();
}

bool
PlayerBackend::doesVolumeCorrection() const
{
	return false;
}

bool
PlayerBackend::supportsChangingAudioStream(bool *onTheFly) const
{
	if(onTheFly)
		*onTheFly = true;

	return true;
}

void
PlayerBackend::playbackRateNotify(double newRate)
{
	if(m_player->m_playbackRate != newRate) {
		m_player->m_playbackRate = newRate;
		emit m_player->playbackRateChanged(newRate);
	}
}

