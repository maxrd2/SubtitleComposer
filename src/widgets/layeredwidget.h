#ifndef LAYEREDWIDGET_H
#define LAYEREDWIDGET_H

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

#include <QList>
#include <QWidget>

/// a class used to show varios widgets simultaneously one in top
/// of the another (unlike QStackWidget which shows one at a time)

class LayeredWidget : public QWidget
{
	Q_OBJECT

public:
	typedef enum { HandleResize, IgnoreResize } Mode;

	explicit LayeredWidget(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());

	void setWidgetMode(QWidget *widget, Mode mode);

public slots:
	void setMouseTracking(bool enable);

protected:
	void resizeEvent(QResizeEvent *e) override;

private:
	QList<QObject *> m_ignoredWidgets;
};

#endif
