
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

#include "errortracker.h"
#include "application.h"
#include "core/subtitleline.h"

using namespace SubtitleComposer;

ErrorTracker::ErrorTracker(QObject *parent) :
	QObject(parent),
	m_subtitle(0),
	m_autoClearFixed(SCConfig::autoClearFixed()),
	m_minDuration(SCConfig::minDuration()),
	m_maxDuration(SCConfig::maxDuration()),
	m_minDurationPerChar(SCConfig::minDurationPerCharacter()),
	m_maxDurationPerChar(SCConfig::maxDurationPerCharacter()),
	m_maxCharacters(SCConfig::maxCharacters()),
	m_maxLines(SCConfig::maxLines())
{
	connect(SCConfig::self(), SIGNAL(configChanged()), this, SLOT(onConfigChanged()));
}

ErrorTracker::~ErrorTracker()
{}

bool
ErrorTracker::isTracking() const
{
	return m_autoClearFixed && m_subtitle;
}

void
ErrorTracker::setSubtitle(Subtitle *subtitle)
{
	if(isTracking())
		disconnectSlots();
	m_subtitle = subtitle;
	if(isTracking())
		connectSlots();
}

void
ErrorTracker::connectSlots()
{
	connect(m_subtitle, SIGNAL(linePrimaryTextChanged(SubtitleLine *, const SString &)), this, SLOT(onLinePrimaryTextChanged(SubtitleLine *)));
	connect(m_subtitle, SIGNAL(lineSecondaryTextChanged(SubtitleLine *, const SString &)), this, SLOT(onLineSecondaryTextChanged(SubtitleLine *)));
	connect(m_subtitle, SIGNAL(lineShowTimeChanged(SubtitleLine *, const Time &)), this, SLOT(onLineTimesChanged(SubtitleLine *)));
	connect(m_subtitle, SIGNAL(lineHideTimeChanged(SubtitleLine *, const Time &)), this, SLOT(onLineTimesChanged(SubtitleLine *)));
}

void
ErrorTracker::disconnectSlots()
{
	disconnect(m_subtitle, SIGNAL(linePrimaryTextChanged(SubtitleLine *, const SString &)), this, SLOT(onLinePrimaryTextChanged(SubtitleLine *)));
	disconnect(m_subtitle, SIGNAL(lineSecondaryTextChanged(SubtitleLine *, const SString &)), this, SLOT(onLineSecondaryTextChanged(SubtitleLine *)));
	disconnect(m_subtitle, SIGNAL(lineShowTimeChanged(SubtitleLine *, const Time &)), this, SLOT(onLineTimesChanged(SubtitleLine *)));
	disconnect(m_subtitle, SIGNAL(lineHideTimeChanged(SubtitleLine *, const Time &)), this, SLOT(onLineTimesChanged(SubtitleLine *)));
}

void
ErrorTracker::updateLineErrors(SubtitleLine *line, int errorFlags) const
{
	line->check(errorFlags, m_minDuration, m_maxDuration, m_minDurationPerChar, m_maxDurationPerChar, m_maxCharacters, m_maxLines);
}

void
ErrorTracker::onLinePrimaryTextChanged(SubtitleLine *line)
{
	updateLineErrors(line, line->errorFlags() & SubtitleLine::PrimaryOnlyErrors);
}

void
ErrorTracker::onLineSecondaryTextChanged(SubtitleLine *line)
{
	updateLineErrors(line, line->errorFlags() & SubtitleLine::SecondaryOnlyErrors);
}

void
ErrorTracker::onLineTimesChanged(SubtitleLine *line)
{
	updateLineErrors(line, line->errorFlags() & SubtitleLine::TimesErrors);

	SubtitleLine *prevLine = line->prevLine();
	if(prevLine)
		updateLineErrors(prevLine, prevLine->errorFlags() & SubtitleLine::OverlapsWithNext);
}

void
ErrorTracker::onConfigChanged()
{
	if(m_autoClearFixed != SCConfig::autoClearFixed()) {
		if(isTracking())
			disconnectSlots();
		m_autoClearFixed = SCConfig::autoClearFixed();
		if(isTracking())
			connectSlots();
	}

	m_minDuration = SCConfig::minDuration();
	m_maxDuration = SCConfig::maxDuration();
	m_minDurationPerChar = SCConfig::minDurationPerCharacter();
	m_maxDurationPerChar = SCConfig::maxDurationPerCharacter();
	m_maxCharacters = SCConfig::maxCharacters();
	m_maxLines = SCConfig::maxLines();
}


