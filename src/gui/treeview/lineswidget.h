#ifndef LINESWIDGET_H
#define LINESWIDGET_H

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

#include "core/subtitle.h"
#include "gui/treeview/linesmodel.h"
#include "gui/treeview/treeview.h"

#include <QPen>

namespace SubtitleComposer {
class LinesItemDelegate;

class LinesWidget : public TreeView
{
	Q_OBJECT

public:
	explicit LinesWidget(QWidget *parent);
	virtual ~LinesWidget();

	bool showingContextMenu();

	SubtitleLine * currentLine() const;
	int currentLineIndex() const;

	int firstSelectedIndex() const;
	int lastSelectedIndex() const;
	bool selectionHasMultipleRanges() const;
	RangeList selectionRanges() const;
	RangeList targetRanges(int target) const;

	inline LinesModel * model() const { return static_cast<LinesModel *>(TreeView::model()); }
	inline bool scrollFollowsModel() const { return m_scrollFollowsModel; }

	void loadConfig();
	void saveConfig();

	bool eventFilter(QObject *object, QEvent *event) override;

public slots:
	void setSubtitle(Subtitle *subtitle = 0);
	void setTranslationMode(bool enabled);

	void setCurrentLine(SubtitleLine *line, bool clearSelection = true);
	void setPlayingLine(SubtitleLine *line);

	void editCurrentLineInPlace(bool primaryText = true);

signals:
	void currentLineChanged(SubtitleLine *line);
	void lineDoubleClicked(SubtitleLine *line);

protected slots:
	void closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint) override;

protected:
	void changeEvent(QEvent *event) override;

private:
	void contextMenuEvent(QContextMenuEvent *e) override;
	void mouseDoubleClickEvent(QMouseEvent *e) override;

	static void drawHorizontalDotLine(QPainter *painter, int x1, int x2, int y);
	static void drawVerticalDotLine(QPainter *painter, int x, int y1, int y2);

	void drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private slots:
	void onCurrentRowChanged();

private:
	bool m_scrollFollowsModel;

	bool m_translationMode;

	bool m_showingContextMenu;

	QPen m_gridPen;

	LinesItemDelegate *m_plainTextDelegate;
	LinesItemDelegate *m_richTextDelegate;

	friend class LinesWidgetScrollToModelDetacher;
	friend class LinesModel;
};

class LinesWidgetScrollToModelDetacher
{
public:
	inline LinesWidgetScrollToModelDetacher(LinesWidget &w) : m_linesWidget(w) { m_linesWidget.m_scrollFollowsModel = false; }
	inline ~LinesWidgetScrollToModelDetacher() { m_linesWidget.m_scrollFollowsModel = true; }

private:
	LinesWidget &m_linesWidget;
};
}
#endif
