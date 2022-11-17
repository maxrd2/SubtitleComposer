/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LINESMODEL_H
#define LINESMODEL_H

#include <QAbstractListModel>
#include <QExplicitlySharedDataPointer>
#include <QList>
#include <QTimer>

namespace SubtitleComposer {
class Subtitle;
class SubtitleLine;

class LinesModel : public QAbstractListModel
{
	Q_OBJECT

public:
	enum { Number = 0, PauseTime, ShowTime, HideTime, Duration, Text, Translation, ColumnCount };
	enum { PlayingLineRole = Qt::UserRole, MarkedRole, ErrorRole, AnchoredRole };

	explicit LinesModel(QObject *parent = nullptr);

	inline Subtitle * subtitle() const { return m_subtitle.data(); }
	void setSubtitle(Subtitle *subtitle);

	inline SubtitleLine * playingLine() const { return m_playingLine; }
	void setPlayingLine(SubtitleLine *line);

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	Qt::ItemFlags flags(const QModelIndex &index) const override;

	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	QVariant data(const QModelIndex &index, int role) const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

	inline void processSelectionUpdate() {
		if(!m_resetModelTimer->isActive())
			return;
		m_resetModelTimer->stop();
		onModelReset();
	}

private slots:
	void onLinesInserted(int firstIndex, int lastIndex);
	void onLinesAboutToRemove(int firstIndex, int lastIndex);
	void onLinesRemoved(int firstIndex, int lastIndex);
	void onModelReset();

	void onLineChanged(const SubtitleLine *line);
	void onLinesChanged();
	void emitDataChanged();

private:
	static QString buildToolTip(SubtitleLine *line, bool primary);

private:
	QExplicitlySharedDataPointer<Subtitle> m_subtitle;
	SubtitleLine *m_playingLine;
	QTimer *m_dataChangedTimer;
	int m_minChangedLineIndex;
	int m_maxChangedLineIndex;
	QTimer *m_resetModelTimer;
	std::pair<const SubtitleLine *, const SubtitleLine *> m_resetModelSelection;
	bool m_resetModelResumeEditing;

	friend class LinesWidget;
};
}

#endif // LINESMODEL_H
