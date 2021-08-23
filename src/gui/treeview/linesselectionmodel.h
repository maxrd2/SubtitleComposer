#ifndef LINESSELECTIONMODEL_H
#define LINESSELECTIONMODEL_H

/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2020 Mladen Milinkovic <max@smoothware.net>
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

#include <QItemSelectionModel>
#include <QSet>

namespace SubtitleComposer {
class LinesModel;
class SubtitleLine;

class LinesSelectionModel : public QItemSelectionModel
{
	Q_OBJECT

public:
	LinesSelectionModel(LinesModel *model = nullptr);

	inline SubtitleLine * currentLine() { return m_currentLine; }

public slots:
	void setCurrentIndex(const QModelIndex &index, QItemSelectionModel::SelectionFlags command) override;
	void select(const QModelIndex &index, QItemSelectionModel::SelectionFlags command) override;
	void select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command) override;
	void clear() override;
	void reset() override;

private:
	bool m_resetInProgress;
	SubtitleLine *m_currentLine;
	QSet<const SubtitleLine *> m_selection;
};
}

#endif // LINESSELECTIONMODEL_H
