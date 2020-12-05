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

#include "linesselectionmodel.h"
#include "core/subtitle.h"
#include "core/subtitleline.h"
#include "gui/treeview/linesmodel.h"

#include <QDebug>
#include <QSignalBlocker>

using namespace SubtitleComposer;

/**
 * @brief QItemSelectionModel that doesn't lose selection during model reset
 */
LinesSelectionModel::LinesSelectionModel(LinesModel *model)
	: QItemSelectionModel(model),
	  m_resetInProgress(false),
	  m_currentLine(nullptr)
{
}

void
LinesSelectionModel::setCurrentIndex(const QModelIndex &index, QItemSelectionModel::SelectionFlags command)
{
	const Subtitle *subtitle = static_cast<LinesModel *>(model())->subtitle();
	m_currentLine = subtitle->line(index.row());

	QItemSelectionModel::setCurrentIndex(index, command);
}

void
LinesSelectionModel::select(const QModelIndex &index, QItemSelectionModel::SelectionFlags command)
{
	select(QItemSelection(index, index), command);
}

void
LinesSelectionModel::select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command)
{
	QItemSelectionModel::select(selection, command);

	if(command == NoUpdate)
		return;

	if(!m_resetInProgress && (command & Clear))
		m_selection.clear();

	const Subtitle *subtitle = static_cast<LinesModel *>(model())->subtitle();
	QModelIndexList sel = selection.indexes();
	while(!sel.empty()) {
		const SubtitleLine *line = subtitle->line(sel.takeFirst().row());
		if(command & Select) {
			m_selection.insert(line);
		} else if(command & Deselect) {
			m_selection.remove(line);
		} else if(command & Toggle) {
			if(m_selection.contains(line))
				m_selection.remove(line);
			else
				m_selection.insert(line);
		}
	}
}

void
LinesSelectionModel::clear()
{
	QItemSelectionModel::clear();
	if(!m_resetInProgress) {
		m_currentLine = nullptr;
		m_selection.clear();
	}
}

void
LinesSelectionModel::reset()
{
	m_resetInProgress = true;
	QItemSelectionModel::reset();
	m_resetInProgress = false;

	if(m_currentLine)
		QItemSelectionModel::setCurrentIndex(model()->index(m_currentLine->index(), 0), QItemSelectionModel::Current);

	const LinesModel *model = static_cast<LinesModel *>(this->model());
	Subtitle *subtitle = model->subtitle();
	const int lastCol = model->columnCount() - 1;
	for(auto it = m_selection.cbegin(); it != m_selection.cend(); ++it) {
		const SubtitleLine *line = *it;
		if(line && line->subtitle() == subtitle) {
			const int i = line->index();
			QItemSelectionModel::select(QItemSelection(model->index(i), model->index(i, lastCol)), QItemSelectionModel::Select);
		}
	}
}
