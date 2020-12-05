#ifndef LINESITEMDELEGATE_H
#define LINESITEMDELEGATE_H

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

#include <QStyledItemDelegate>

QT_FORWARD_DECLARE_CLASS(QTextDocument)

namespace SubtitleComposer {
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
}

#endif // LINESITEMDELEGATE_H
