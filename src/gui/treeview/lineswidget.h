/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LINESWIDGET_H
#define LINESWIDGET_H

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

	inline bool isEditing() { return m_inlineEditor != nullptr; }

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

private:
	void contextMenuEvent(QContextMenuEvent *e) override;
	void mouseDoubleClickEvent(QMouseEvent *e) override;

	static void drawHorizontalDotLine(QPainter *painter, int x1, int x2, int y);
	static void drawVerticalDotLine(QPainter *painter, int x, int y1, int y2);
	void updateHeader();

	void drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private slots:
	void onCurrentRowChanged();

private:
	bool m_scrollFollowsModel;
	bool m_translationMode;
	bool m_showingContextMenu;
	QPen m_gridPen;

	LinesItemDelegate *m_itemsDelegate;
	QWidget *m_inlineEditor;

	friend class LinesWidgetScrollToModelDetacher;
	friend class LinesModel;
	friend class LinesItemDelegate;
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
