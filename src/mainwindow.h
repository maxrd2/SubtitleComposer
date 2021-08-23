#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>
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

#include <kxmlguiwindow.h>

namespace SubtitleComposer {
class PlayerWidget;
class LinesWidget;
class CurrentLineWidget;
class WaveformWidget;

class MainWindow : public KXmlGuiWindow
{
	Q_OBJECT

	friend class Application;

public:
	MainWindow();
	virtual ~MainWindow();

	void loadConfig();
	void saveConfig();

	QMenu *createPopupMenu() override;

public slots:
	void setSubtitle(Subtitle *subtitle = 0);

protected:
	bool queryClose() override;

protected:
	PlayerWidget *m_playerWidget;
	LinesWidget *m_linesWidget;
	CurrentLineWidget *m_curLineWidget;
	WaveformWidget *m_waveformWidget;
};
}
#endif
