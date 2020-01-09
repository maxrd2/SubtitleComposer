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

#include "linesmodel.h"
#include "core/subtitle.h"
#include "gui/lineswidget.h"

#include <QFont>
#include <QFontMetrics>
#include <QItemSelectionModel>
#include <QTimer>

#include <KLocalizedString>

#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
#define horizontalAdvance width
#endif

using namespace SubtitleComposer;

LinesModel::LinesModel(QObject *parent)
	: QAbstractListModel(parent),
	  m_subtitle(nullptr),
	  m_playingLine(nullptr),
	  m_dataChangedTimer(new QTimer(this)),
	  m_minChangedLineIndex(-1),
	  m_maxChangedLineIndex(-1),
	  m_graftPoints(QList<Subtitle *>()),
	  m_allowModelReset(true),
	  m_resettingModel(0)
{
	m_dataChangedTimer->setInterval(0);
	m_dataChangedTimer->setSingleShot(true);

	connect(m_dataChangedTimer, &QTimer::timeout, this, &LinesModel::emitDataChanged);
}

Subtitle *
LinesModel::subtitle() const
{
	return m_subtitle;
}

void
LinesModel::setSubtitle(Subtitle *subtitle)
{
	if(m_subtitle != subtitle) {
		m_playingLine = nullptr;

		if(m_subtitle) {
			disconnect(m_subtitle, &Subtitle::linesInserted, this, &LinesModel::onLinesInserted);
			disconnect(m_subtitle, &Subtitle::linesRemoved, this, &LinesModel::onLinesRemoved);
			disconnect(m_subtitle, &Subtitle::compositeActionStart, this, &LinesModel::onCompositeActionStart);
			disconnect(m_subtitle, &Subtitle::compositeActionEnd, this, &LinesModel::onCompositeActionEnd);

			disconnect(m_subtitle, &Subtitle::lineAnchorChanged, this, &LinesModel::onLineChanged);
			disconnect(m_subtitle, &Subtitle::lineErrorFlagsChanged, this, &LinesModel::onLineChanged);
			disconnect(m_subtitle, &Subtitle::linePrimaryTextChanged, this, &LinesModel::onLineChanged);
			disconnect(m_subtitle, &Subtitle::lineSecondaryTextChanged, this, &LinesModel::onLineChanged);
			disconnect(m_subtitle, &Subtitle::lineShowTimeChanged, this, &LinesModel::onLineChanged);
			disconnect(m_subtitle, &Subtitle::lineHideTimeChanged, this, &LinesModel::onLineChanged);

			if(m_subtitle->linesCount()) {
				onLinesRemoved(0, m_subtitle->linesCount() - 1);
			}
		}

		m_subtitle = subtitle;

		if(m_subtitle) {
			if(m_subtitle->linesCount()) {
				onLinesInserted(0, m_subtitle->linesCount() - 1);
			}

			connect(m_subtitle, &Subtitle::linesInserted, this, &LinesModel::onLinesInserted);
			connect(m_subtitle, &Subtitle::linesRemoved, this, &LinesModel::onLinesRemoved);
			connect(m_subtitle, &Subtitle::compositeActionStart, this, &LinesModel::onCompositeActionStart);
			connect(m_subtitle, &Subtitle::compositeActionEnd, this, &LinesModel::onCompositeActionEnd);

			connect(m_subtitle, &Subtitle::lineAnchorChanged, this, &LinesModel::onLineChanged);
			connect(m_subtitle, &Subtitle::lineErrorFlagsChanged, this, &LinesModel::onLineChanged);
			connect(m_subtitle, &Subtitle::linePrimaryTextChanged, this, &LinesModel::onLineChanged);
			connect(m_subtitle, &Subtitle::lineSecondaryTextChanged, this, &LinesModel::onLineChanged);
			connect(m_subtitle, &Subtitle::lineShowTimeChanged, this, &LinesModel::onLineChanged);
			connect(m_subtitle, &Subtitle::lineHideTimeChanged, this, &LinesModel::onLineChanged);
		}
	}
}

const QList<Subtitle *> &
LinesModel::graftPoints() const
{
	return m_graftPoints;
}

SubtitleLine *
LinesModel::playingLine() const
{
	return m_playingLine;
}

void
LinesModel::setPlayingLine(SubtitleLine *line)
{
	if(m_playingLine != line) {
		if(m_playingLine) {
			int row = m_playingLine->index();
			m_playingLine = nullptr;
			emit dataChanged(index(row, 0), index(row, ColumnCount));
		}

		m_playingLine = line;

		if(line) {
			int row = m_playingLine->index();
			emit dataChanged(index(row, 0), index(row, ColumnCount));
		}
	}
}

int
LinesModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return m_subtitle ? m_subtitle->linesCount() : 0;
}

int
LinesModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return ColumnCount;
}

Qt::ItemFlags
LinesModel::flags(const QModelIndex &index) const
{
	if(!index.isValid() || index.column() < Text)
		return QAbstractItemModel::flags(index);

	return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

QString
LinesModel::buildToolTip(SubtitleLine *line, bool primary)
{
	int errorFlags = line->errorFlags();
	if(primary)
		errorFlags &= ~SubtitleLine::SecondaryOnlyErrors;
	else
		errorFlags &= ~SubtitleLine::PrimaryOnlyErrors;

	const SString &text = primary ? line->primaryText() : line->secondaryText();

	if(errorFlags) {
		QString toolTip = "<p style='white-space:pre;margin-bottom:6px;'>" + text.richString() + "</p><p style='white-space:pre;margin-top:0px;'>";

		if(errorFlags) {
			toolTip += i18n("<b>Observations:</b>");

			for(int id = 0; id < SubtitleLine::ErrorSIZE; ++id) {
				if(!((0x1 << id) & errorFlags))
					continue;

				QString errorText = line->fullErrorText((SubtitleLine::ErrorID)id);
				if(!errorText.isEmpty())
					toolTip += "\n  - " + errorText;
			}
		}

		toolTip += "</p>";

		return toolTip;
	}

	return text.richString();
}

QVariant
LinesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(role != Qt::DisplayRole || orientation == Qt::Vertical)
		return QVariant();

	switch(section) {
	case Number: return i18nc("@title:column Subtitle line number", "Line");
	case ShowTime: return i18nc("@title:column", "Show Time");
	case HideTime: return i18nc("@title:column", "Hide Time");
	case Text: return i18nc("@title:column Subtitle line (primary) text", "Text");
	case Translation: return i18nc("@title:column Subtitle line translation text", "Translation");
	default: return QVariant();
	}
}

QVariant
LinesModel::data(const QModelIndex &index, int role) const
{
	if(!m_subtitle)
		return QVariant();

	SubtitleLine *line = m_subtitle->at(index.row());

	if(role == PlayingLineRole)
		return line == m_playingLine;

	if(role == AnchoredRole)
		return !m_subtitle->hasAnchors() ? 0 : (m_subtitle->isLineAnchored(line) ? 1 : -1);

	switch(index.column()) {
	case Number:
		if(role == Qt::DisplayRole)
			return index.row() + 1;
		if(role == Qt::SizeHintRole)
			return QSize(QFontMetrics(QFont()).horizontalAdvance(QString::number(index.row() + 1)) + 28, 0);
		break;

	case ShowTime:
		if(role == Qt::DisplayRole)
			return line->showTime().toString();
		if(role == Qt::TextAlignmentRole)
			return Qt::AlignCenter;
		break;

	case HideTime:
		if(role == Qt::DisplayRole)
			return line->hideTime().toString();
		if(role == Qt::TextAlignmentRole)
			return Qt::AlignCenter;
		break;

	case Text:
		if(role == Qt::DisplayRole)
			return line->primaryText().richString();
		if(role == MarkedRole)
			return line->errorFlags() & SubtitleLine::UserMark;
		if(role == ErrorRole)
			return line->errorFlags() & ((SubtitleLine::SharedErrors | SubtitleLine::PrimaryOnlyErrors) & ~SubtitleLine::UserMark);
		if(role == Qt::ToolTipRole)
			return buildToolTip(m_subtitle->line(index.row()), true);
		if(role == Qt::EditRole)
			return m_subtitle->line(index.row())->primaryText().richString().replace('\n', '|');
		break;

	case Translation:
		if(role == Qt::DisplayRole)
			return m_subtitle->line(index.row())->secondaryText().richString();
		if(role == MarkedRole)
			return line->errorFlags() & SubtitleLine::UserMark;
		if(role == ErrorRole)
			return line->errorFlags() & ((SubtitleLine::SharedErrors | SubtitleLine::SecondaryOnlyErrors) & ~SubtitleLine::UserMark);
		if(role == Qt::ToolTipRole)
			return buildToolTip(m_subtitle->line(index.row()), false);
		if(role == Qt::EditRole)
			return m_subtitle->line(index.row())->secondaryText().richString().replace('\n', '|');
		break;

	default:
		break;
	}
	return QVariant();
}

bool
LinesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if(!m_subtitle || !index.isValid())
		return false;

	switch(index.column()) {
	case Text:
		if(role == Qt::EditRole) {
			SString sstring;
			sstring.setRichString(value.toString().replace('|', '\n'));
			m_subtitle->line(index.row())->setPrimaryText(sstring);
			emit dataChanged(index, index);
			return true;
		}
		break;
	case Translation:
		if(role == Qt::EditRole) {
			SString sstring;
			sstring.setRichString(value.toString().replace('|', '\n'));
			m_subtitle->line(index.row())->setSecondaryText(sstring);
			emit dataChanged(index, index);
			return true;
		}
		break;
	default:
		break;
	}

	return false;
}

void
LinesModel::onLinesInserted(int firstIndex, int lastIndex)
{
	if(m_resettingModel)
		return;

	static const QModelIndex rootIndex;

	beginInsertRows(rootIndex, firstIndex, lastIndex);
	endInsertRows();                        // ridiculously costly operation
}

void
LinesModel::onLinesRemoved(int firstIndex, int lastIndex)
{
	if(m_resettingModel)
		return;

	static const QModelIndex rootIndex;

	beginRemoveRows(rootIndex, firstIndex, lastIndex);
	endRemoveRows();
}

void
LinesModel::onCompositeActionStart()
{
	if(!m_allowModelReset)
		return;

	if(m_resettingModel++ == 0) {
		// preserve selection as beginResetModel() will invalidate all current/selected item(s)
		QItemSelectionModel *sm = static_cast<LinesWidget *>(parent())->selectionModel();
		QModelIndexList sel = sm->selectedIndexes();
		while(!sel.empty())
			m_selectionBackup.push_back(m_subtitle->at(sel.takeLast().row()));

		beginResetModel();
	}
}

void
LinesModel::onCompositeActionEnd()
{
	if(!m_allowModelReset) {
		Q_ASSERT(m_resettingModel == 0);
		return;
	}

	Q_ASSERT(m_resettingModel > 0);
	if(--m_resettingModel == 0) {
		endResetModel();

		// restore selection
		const LinesWidget *widget = static_cast<LinesWidget *>(parent());
		const LinesModel *model = widget->model();
		QItemSelectionModel *sm = widget->selectionModel();
		const int lastCol = model->columnCount() - 1;
		while(!m_selectionBackup.empty()) {
			const SubtitleLine *line = m_selectionBackup.takeFirst();
			if(line && line->subtitle() == m_subtitle) {
				const int i = line->index();
				sm->select(QItemSelection(model->index(i), model->index(i, lastCol)), QItemSelectionModel::Select);
			}
		}
	}
}

void
LinesModel::onLineChanged(const SubtitleLine *line)
{
	const int lineIndex = line->index();

	if(m_minChangedLineIndex < 0) {
		m_minChangedLineIndex = lineIndex;
		m_maxChangedLineIndex = lineIndex;
		m_dataChangedTimer->start();
	} else if(lineIndex < m_minChangedLineIndex) {
		m_minChangedLineIndex = lineIndex;
	} else if(lineIndex > m_maxChangedLineIndex) {
		m_maxChangedLineIndex = lineIndex;
	}

	LinesWidget *w = static_cast<LinesWidget *>(parent());
	const QModelIndex idx = w->model()->index(lineIndex);
	w->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::Current);
	if(w->m_scrollFollowsModel)
		w->scrollTo(idx, QAbstractItemView::EnsureVisible);
}

void
LinesModel::emitDataChanged()
{
	if(m_minChangedLineIndex < 0)
		m_minChangedLineIndex = m_maxChangedLineIndex;
	else if(m_maxChangedLineIndex < 0)
		m_maxChangedLineIndex = m_minChangedLineIndex;

	emit dataChanged(index(m_minChangedLineIndex, 0), index(m_maxChangedLineIndex, ColumnCount - 1));

	m_minChangedLineIndex = -1;
	m_maxChangedLineIndex = -1;
}
