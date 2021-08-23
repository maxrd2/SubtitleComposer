#ifndef ERRORTRACKER_H
#define ERRORTRACKER_H

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

#include "core/subtitle.h"

namespace SubtitleComposer {
class SubtitleLine;

class ErrorTracker : public QObject
{
	Q_OBJECT

public:
	explicit ErrorTracker(QObject *parent = 0);
	virtual ~ErrorTracker();

	bool isTracking() const;

public slots:
	void setSubtitle(Subtitle *subtitle = 0);

private:
	void connectSlots();
	void disconnectSlots();

	void updateLineErrors(SubtitleLine *line, int errorFlags) const;

private slots:
	void onLinePrimaryTextChanged(SubtitleLine *line);
	void onLineSecondaryTextChanged(SubtitleLine *line);
	void onLineTimesChanged(SubtitleLine *line);

	void onConfigChanged();

private:
	QExplicitlySharedDataPointer<const Subtitle> m_subtitle;
	bool m_autoClearFixed;
};
}
#endif
