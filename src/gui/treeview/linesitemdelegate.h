/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2020 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef LINESITEMDELEGATE_H
#define LINESITEMDELEGATE_H

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

	LinesItemDelegate(LinesWidget *parent);
	virtual ~LinesItemDelegate();

	inline LinesWidget * linesWidget() const { return qobject_cast<LinesWidget *>(parent()); }

	QString displayText(const QVariant &value, const QLocale &locale) const override;
	QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
	void setEditorData(QWidget *editor, const QModelIndex &index) const override;

protected:
	bool eventFilter(QObject *object, QEvent *event) override;

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};
}

#endif // LINESITEMDELEGATE_H
