#ifndef SCRIPTING_RANGELIST_H
#define SCRIPTING_RANGELIST_H

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

#include "core/rangelist.h"

#include <QObject>

namespace SubtitleComposer {
namespace Scripting {
class RangeList : public QObject
{
	Q_OBJECT

public slots:
	QObject * complement() const;

	bool isEmpty() const;

	int rangesCount() const;
	int indexesCount() const;

	int firstIndex() const;
	int lastIndex() const;

	QObject * range(int rangeIndex) const;

	bool contains(int index) const;

	void clear();

	void trimToIndex(int index);
	void trimToRange(const QObject *range);

	QObject * addIndex(int index);
	QObject * addRange(const QObject *range);

	void shiftIndexesForwards(int fromIndex, int delta, bool fillSplitGap);
	void shiftIndexesBackwards(int fromIndex, int delta);

private:
	friend class RangesModule;
	friend class Subtitle;

	RangeList(const SubtitleComposer::RangeList &backend, QObject *parent);

	SubtitleComposer::RangeList m_backend;
};
}
}
#endif
