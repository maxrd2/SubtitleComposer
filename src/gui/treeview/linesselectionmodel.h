#ifndef LINESSELECTIONMODEL_H
#define LINESSELECTIONMODEL_H

/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2020 Mladen Milinkovic <max@smoothware.net>
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

public slots:
	void setCurrentIndex(const QModelIndex &index, QItemSelectionModel::SelectionFlags command) override;
	void select(const QModelIndex &index, QItemSelectionModel::SelectionFlags command) override;
	void select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command) override;
	void clear() override;
	void reset() override;

private:
	bool m_resetInProgress;
	const SubtitleLine *m_currentLine;
	QSet<const SubtitleLine *> m_selection;
};
}

#endif // LINESSELECTIONMODEL_H
