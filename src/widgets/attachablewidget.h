#ifndef ATTACHABLEWIDGET_H
#define ATTACHABLEWIDGET_H

/***************************************************************************
 *   smplayer, GUI front-end for mplayer.                                  *
 *   SPDX-FileCopyrightText: 2006-2008 Ricardo Villalba (rvm@escomposlinux.org)      *
 *                                                                         *
 *   modified for inclusion in Subtitle Composer                           *
 *   SPDX-FileCopyrightText: 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)  *
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
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

#include <QWidget>

class AttachableWidget : public QWidget
{
	Q_OBJECT

public:
	typedef enum { Top, Bottom } Place;

	explicit AttachableWidget(Place place = Bottom, unsigned animStepDuration = 4);
	virtual ~AttachableWidget();

	bool isAttached() const;

	bool isAnimated() const;
	int animStepDuration() const;

	bool eventFilter(QObject *object, QEvent *event) override;

public slots:
	void attach(QWidget *target);
	void dettach();

	void setAnimStepDuration(int stepDuration);

	void toggleVisible(bool visible);

protected:
	void timerEvent(QTimerEvent *event) override;

private:
	void toggleVisible(bool visible, bool force);

private:
	QWidget *m_targetWidget;
	Place m_place;
	int m_animStepDuration;

	typedef enum { Upward, Downward } Direction;

	int m_animTID;
	bool m_animHiding;
	int m_animFinalY;
	int m_animCurrentY;
	Direction m_animDirection;
};

#endif
