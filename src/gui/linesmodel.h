#ifndef LINESMODEL_H
#define LINESMODEL_H

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

#include <QAbstractListModel>
#include <QList>

QT_FORWARD_DECLARE_CLASS(QTimer)

namespace SubtitleComposer {
class Subtitle;
class SubtitleLine;

class LinesModel : public QAbstractListModel
{
	Q_OBJECT

public:
	enum { Number = 0, ShowTime, HideTime, Text, Translation, ColumnCount };
	enum { PlayingLineRole = Qt::UserRole, MarkedRole, ErrorRole, AnchoredRole };

	explicit LinesModel(QObject *parent = nullptr);

	Subtitle * subtitle() const;
	void setSubtitle(Subtitle *subtitle);

	const QList<Subtitle *> & graftPoints() const;

	SubtitleLine * playingLine() const;
	void setPlayingLine(SubtitleLine *line);

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	Qt::ItemFlags flags(const QModelIndex &index) const override;

	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	QVariant data(const QModelIndex &index, int role) const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

private slots:
	void onLinesInserted(int firstIndex, int lastIndex);
	void onLinesRemoved(int firstIndex, int lastIndex);

	void onCompositeActionStart();
	void onCompositeActionEnd();

	void onLineChanged(const SubtitleLine *line);
	void emitDataChanged();

private:
	static QString buildToolTip(SubtitleLine *line, bool primary);

private:
	Subtitle *m_subtitle;
	SubtitleLine *m_playingLine;
	QTimer *m_dataChangedTimer;
	int m_minChangedLineIndex;
	int m_maxChangedLineIndex;
	QList<Subtitle *> m_graftPoints;
	bool m_allowModelReset;
	int m_resettingModel;
	QList<SubtitleLine *> m_selectionBackup;

	friend class LinesWidget;
};
}

#endif // LINESMODEL_H
