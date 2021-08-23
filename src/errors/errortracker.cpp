
/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
#include "core/richdocument.h"
#include "core/subtitleline.h"

using namespace SubtitleComposer;

ErrorTracker::ErrorTracker(QObject *parent)
	: QObject(parent),
	  m_subtitle(nullptr),
	  m_autoClearFixed(SCConfig::autoClearFixed())
{
	connect(SCConfig::self(), &SCConfig::configChanged, this, &ErrorTracker::onConfigChanged);
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
	connect(m_subtitle.constData(), &Subtitle::linePrimaryTextChanged, this, &ErrorTracker::onLinePrimaryTextChanged);
	connect(m_subtitle.constData(), &Subtitle::lineSecondaryTextChanged, this, &ErrorTracker::onLineSecondaryTextChanged);
	connect(m_subtitle.constData(), &Subtitle::lineShowTimeChanged, this, &ErrorTracker::onLineTimesChanged);
	connect(m_subtitle.constData(), &Subtitle::lineHideTimeChanged, this, &ErrorTracker::onLineTimesChanged);
}

void
ErrorTracker::disconnectSlots()
{
	disconnect(m_subtitle.constData(), nullptr, this, nullptr);
}

void
ErrorTracker::updateLineErrors(SubtitleLine *line, int errorFlags) const
{
	line->check(errorFlags);
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
	if(m_autoClearFixed == SCConfig::autoClearFixed())
		return;

	if(isTracking())
		disconnectSlots();
	m_autoClearFixed = SCConfig::autoClearFixed();
	if(isTracking())
		connectSlots();
}


