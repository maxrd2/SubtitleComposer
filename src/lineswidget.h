#ifndef LINESWIDGET_H
#define LINESWIDGET_H

/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2019 Mladen Milinkovic <max@smoothware.net>
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

#include "core/rangelist.h"
#include "core/subtitle.h"
#include "core/subtitleline.h"
#include "widgets/treeview.h"

#include <QAbstractItemModel>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QPen>
#include <QIcon>
#include <QPixmap>
#include <QList>

QT_FORWARD_DECLARE_CLASS(QTextDocument)
QT_FORWARD_DECLARE_CLASS(QTimer)

namespace SubtitleComposer {
class LinesModel : public QAbstractListModel
{
	Q_OBJECT

public:
	enum { Number = 0, ShowTime, HideTime, Text, Translation, ColumnCount };
	enum { PlayingLineRole = Qt::UserRole, MarkedRole, ErrorRole, AnchoredRole };

	explicit LinesModel(QObject *parent = 0);

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

class LinesWidget;

class LinesItemDelegate : public QStyledItemDelegate
{
public:
	typedef enum {
		NoHint = QAbstractItemDelegate::NoHint,
		EditNextItem = QAbstractItemDelegate::EditNextItem,
		EditPreviousItem = QAbstractItemDelegate::EditPreviousItem,
		SubmitModelCache = QAbstractItemDelegate::SubmitModelCache,
		RevertModelCache = QAbstractItemDelegate::RevertModelCache,
		EditUpperItem,
		EditLowerItem,
	} ExtendedEditHint;

	LinesItemDelegate(bool useStyle, bool singleLineMode, bool richTextMode, LinesWidget *parent);
	virtual ~LinesItemDelegate();

	inline LinesWidget * linesWidget() const { return qobject_cast<LinesWidget *>(parent()); }

	bool useStyle() const;
	void setUseStyle(bool useStyle);

	bool singleLineMode() const;
	void setSingleLineMode(bool singleLineMode);

	bool richTextMode() const;
	void setRichTextMode(bool richTextMode);

	void updateStyle();

	QString displayText(const QVariant &value, const QLocale &locale) const override;

	static const QIcon & markIcon();
	static const QIcon & errorIcon();
	static const QIcon & anchorIcon();

protected:
	bool eventFilter(QObject *object, QEvent *event) override;

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

	void drawBackgroundPrimitive(QPainter *painter, const QStyle *style, const QStyleOptionViewItem &option) const;

	void drawTextPrimitive(QPainter *painter, const QStyle *style, const QStyleOptionViewItem &option, const QRect &rect, QPalette::ColorGroup cg, const QModelIndex &index) const;

private:
	bool m_useStyle;
	bool m_singleLineMode;
	QTextDocument *m_textDocument;
};

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

	void loadConfig();
	void saveConfig();

	bool eventFilter(QObject *object, QEvent *event) override;

	/**
	 * @brief Disable calls to begin/endModelReset() during QUndoStack composite action processing.
	 * Disabling model reset will decrease performance but will keep LinesWidget interactive.
	 * @param disable - true to disable
	 */
	void disableModelReset(bool disable);

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

	void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end) override;
	void rowsInserted(const QModelIndex &parent, int start, int end) override;

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
};

class LinesWidgetScrollToModelDetacher
{
public:
	LinesWidgetScrollToModelDetacher(LinesWidget &linesWidget);
	~LinesWidgetScrollToModelDetacher();

private:
	LinesWidget &m_linesWidget;
};
}
#endif
