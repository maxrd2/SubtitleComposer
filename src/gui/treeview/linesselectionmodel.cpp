/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
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
	Subtitle *sub = static_cast<LinesModel *>(model())->subtitle();
	m_currentLine = sub ? sub->line(index.row()) : nullptr;
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
	if(!subtitle)
		return;

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

	const LinesModel *model = static_cast<LinesModel *>(this->model());
	Subtitle *subtitle = model->subtitle();
	if(!subtitle) {
		QItemSelectionModel::clear();
		return;
	}

	if(m_currentLine)
		QItemSelectionModel::setCurrentIndex(model->index(m_currentLine->index(), 0), QItemSelectionModel::Current);

	const int lastCol = model->columnCount() - 1;
	for(auto it = m_selection.cbegin(); it != m_selection.cend(); ++it) {
		const SubtitleLine *line = *it;
		if(line && line->subtitle() == subtitle) {
			const int i = line->index();
			QItemSelectionModel::select(QItemSelection(model->index(i), model->index(i, lastCol)), QItemSelectionModel::Select);
		}
	}
}
