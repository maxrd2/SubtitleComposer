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

#include "linesitemdelegate.h"
#include "gui/treeview/lineswidget.h"
#include "gui/treeview/richdocumentptr.h"
#include "gui/treeview/richlineedit.h"

#include <QApplication>
#include <QKeyEvent>
#include <QPainter>
#include <QTextOption>
#include <QTextBlock>

using namespace SubtitleComposer;

LinesItemDelegate::LinesItemDelegate(LinesWidget *parent)
	: QStyledItemDelegate(parent)
{
}

LinesItemDelegate::~LinesItemDelegate()
{
}

bool
LinesItemDelegate::eventFilter(QObject *object, QEvent *event)
{
	QWidget *editor = qobject_cast<QWidget *>(object);
	if(!editor)
		return false;

	if(event->type() == QEvent::KeyPress) {
		switch(static_cast<QKeyEvent *>(event)->key()) {
		case Qt::Key_Tab:
			emit commitData(editor);
			emit closeEditor(editor, QAbstractItemDelegate::EditNextItem);
			return true;

		case Qt::Key_Backtab:
			emit commitData(editor);
			emit closeEditor(editor, QAbstractItemDelegate::EditPreviousItem);
			return true;

		case Qt::Key_Up:
			emit commitData(editor);
			emit closeEditor(editor, QAbstractItemDelegate::EndEditHint(EditUpperItem));
			return true;

		case Qt::Key_Down:
			emit commitData(editor);
			emit closeEditor(editor, QAbstractItemDelegate::EndEditHint(EditLowerItem));
			return true;

		case Qt::Key_Enter:
		case Qt::Key_Return:
			if(static_cast<QKeyEvent *>(event)->modifiers()) {
				emit commitData(editor);
				emit closeEditor(editor, QAbstractItemDelegate::EndEditHint(EditLowerItem));
				return true;
			}
			if(qobject_cast<RichLineEdit *>(object))
				return false;
			break;

		default:
			break;
		}
	}

	return QStyledItemDelegate::eventFilter(object, event);
}

static const QIcon & markIcon() { static QIcon icon = QIcon::fromTheme(QStringLiteral("dialog-warning")); return icon; }
static const QIcon & errorIcon() { static QIcon icon = QIcon::fromTheme(QStringLiteral("dialog-error")); return icon; }
static const QIcon & anchorIcon() { static QIcon icon = QIcon::fromTheme(QStringLiteral("anchor")); return icon; }

inline static bool isRichDoc(const QModelIndex &index) { return index.column() >= LinesModel::Text; }

static void
drawTextPrimitive(QPainter *painter, const QStyle *style, const QStyleOptionViewItem &option, const QRect &rect, QPalette::ColorGroup cg, const QModelIndex &index)
{
	const int textMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, nullptr, option.widget) + 1;
	Qt::Alignment alignment = QStyle::visualAlignment(option.direction, option.displayAlignment);

	QRect textRect = rect.adjusted(textMargin, 0, -textMargin, 0);  // remove width padding

	if(index.column() == LinesModel::ShowTime && index.data(LinesModel::AnchoredRole).toInt() == -1)
		cg = QPalette::Disabled;

	QColor textColor;
	if(option.state & QStyle::State_Selected)
		textColor = option.palette.color(cg, QPalette::HighlightedText);
	else
		textColor = option.palette.color(cg, QPalette::Text);

	painter->setPen(textColor);

	QString text = option.fontMetrics.elidedText(option.text, option.textElideMode, textRect.width());

	if(index.column() == LinesModel::Number && index.data(LinesModel::AnchoredRole).toInt() == 1) {
		int iconSize = qMin(textRect.width(), textRect.height()) - 3;
		QRect iconRect = QRect(textRect.right() - iconSize - 2, textRect.y() + 1, iconSize, iconSize);

		QIcon::Mode mode = QIcon::Normal;
		if(!(option.state & QStyle::State_Enabled))
			mode = QIcon::Disabled;
		else if(option.state & QStyle::State_Selected)
			mode = QIcon::Selected;
		QIcon::State state = option.state & QStyle::State_Open ? QIcon::On : QIcon::Off;

		textRect.setRight(textRect.right() - iconSize);

		anchorIcon().paint(painter, iconRect, option.decorationAlignment, mode, state);
	}

	painter->drawText(textRect, alignment, text);
}

static void
drawRichText(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect)
{
	const RichDocument *doc = option.index.data(Qt::DisplayRole).value<RichDocumentPtr>();

	QPalette::ColorGroup cg = option.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
	if(cg == QPalette::Normal && !(option.state & QStyle::State_Active))
		cg = QPalette::Inactive;
	painter->setPen(option.palette.color(cg, (option.state & QStyle::State_Selected) ? QPalette::HighlightedText : QPalette::Text));

	const QStyle *style = option.widget ? option.widget->style() : QApplication::style();
	int textMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, nullptr, option.widget) + 1;

	QTextLayout layout;
	layout.setFont(option.font);
	layout.setCacheEnabled(true);

	QTextOption textOption;
	textOption.setAlignment(QStyle::visualAlignment(option.direction, option.displayAlignment));
	textOption.setWrapMode(QTextOption::NoWrap);
	textOption.setTextDirection(option.direction);
	layout.setTextOption(textOption);

	QString text;
	QVector<QTextLayout::FormatRange> ranges;
	for(QTextBlock bi = doc->begin(); bi != doc->end(); bi = bi.next()) {
		if(bi != doc->begin()) {
			QTextCharFormat fmt;
			fmt.setTextOutline(QPen(option.palette.color(cg, QPalette::Link), .75));
			ranges.push_back(QTextLayout::FormatRange{text.length(), 1, fmt});
			text.append(QChar(0x299a));
		}
		for(QTextBlock::iterator it = bi.begin(); !it.atEnd(); ++it) {
			if(!it.fragment().isValid())
				continue;
			const QString &t = it.fragment().text();
			ranges.push_back(QTextLayout::FormatRange{text.length(), t.length(), it.fragment().charFormat()});
			text.append(t);
		}
	}

	layout.setText(text);
	layout.setFormats(ranges);

	const QRect textRect = rect.adjusted(textMargin, 0, -textMargin, 0);
	QPointF textPos(textRect.topLeft());

	layout.beginLayout();
	QTextLine line = layout.createLine();
	line.setPosition(QPointF(0.0, option.fontMetrics.leading()));
	line.setLineWidth(textRect.width());
	textPos.ry() += (qreal(textRect.height()) - line.height()) / 2.;
	layout.endLayout();

	layout.draw(painter, textPos);
}

void
LinesItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
	QStyleOptionViewItem option = opt;
	initStyleOption(&option, index);

	const QWidget *widget = option.widget;
	const QStyle *style = widget ? widget->style() : QApplication::style();

	painter->save();
	painter->setClipRect(option.rect);

	QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &option, widget);

	QPalette::ColorGroup cg = option.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
	if(cg == QPalette::Normal && !(option.state & QStyle::State_Active))
		cg = QPalette::Inactive;

	// draw the background
	if(index.data(LinesModel::PlayingLineRole).toBool()) {
		if(option.state & QStyle::State_Selected) {
			option.backgroundBrush = option.palette.highlight().color().lighter(125);
			option.palette.setBrush(cg, QPalette::Highlight, option.backgroundBrush);
		} else {
			option.backgroundBrush = option.palette.highlight().color().lighter(165);
		}
	}
	style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, option.widget);

	// draw the icon(s)
	const bool showMarkedIcon = index.data(LinesModel::MarkedRole).toBool();
	const bool showErrorIcon = index.data(LinesModel::ErrorRole).toBool();

	if(showMarkedIcon || showErrorIcon) {
		int iconSize = qMin(textRect.width(), textRect.height()) - 3;
		QRect iconRect = QRect(textRect.x() + 2, textRect.y() + 1, iconSize, iconSize);

		QIcon::Mode mode = QIcon::Normal;
		if(!(option.state & QStyle::State_Enabled))
			mode = QIcon::Disabled;
		else if(option.state & QStyle::State_Selected)
			mode = QIcon::Selected;
		QIcon::State state = option.state & QStyle::State_Open ? QIcon::On : QIcon::Off;

		if(showMarkedIcon) {
			markIcon().paint(painter, iconRect, option.decorationAlignment, mode, state);
			textRect.setX(textRect.x() + iconSize + 2);
			if(showErrorIcon)
				iconRect.translate(iconSize + 2, 0);
		}

		if(showErrorIcon) {
			errorIcon().paint(painter, iconRect, option.decorationAlignment, mode, state);
			textRect.setX(textRect.x() + iconSize + 2);
		}
	}
	// draw the text
	if(isRichDoc(index) || !option.text.isEmpty()) {
		if(option.state & QStyle::State_Editing) {
			painter->setPen(option.palette.color(cg, QPalette::Text));
			painter->drawRect(textRect.adjusted(0, 0, -1, -1));
		} else if(isRichDoc(index)) {
			drawRichText(painter, option, textRect);
		} else {
			drawTextPrimitive(painter, style, option, textRect, cg, index);
		}
	}
	// draw the focus rect
	if(option.state & QStyle::State_HasFocus) {
		QStyleOptionFocusRect frOption;
		frOption.QStyleOption::operator=(option);
		frOption.rect = style->subElementRect(QStyle::SE_ItemViewItemFocusRect, &option, widget);
		frOption.state |= QStyle::State_KeyboardFocusChange;
		frOption.state |= QStyle::State_Item;
		frOption.backgroundColor = option.palette.color((option.state & QStyle::State_Enabled) ? QPalette::Normal : QPalette::Disabled, (option.state & QStyle::State_Selected) ? QPalette::Highlight : QPalette::Window);

		style->drawPrimitive(QStyle::PE_FrameFocusRect, &frOption, painter, widget);
	}

	painter->restore();
}

QString
LinesItemDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
	if(value.userType() == qMetaTypeId<RichDocumentPtr>())
		return QString();
	return QStyledItemDelegate::displayText(value, locale);
}

QWidget *
LinesItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if(isRichDoc(index)) {
		RichLineEdit *ed = new RichLineEdit(parent);
		ed->setLineEditStyle(option);
		return ed;
	}
	return QStyledItemDelegate::createEditor(parent, option, index);
}

void
LinesItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	if(isRichDoc(index)) {
		RichDocument *doc = index.data(Qt::EditRole).value<RichDocumentPtr>();
		RichLineEdit *edit = static_cast<RichLineEdit *>(editor);
		if(edit->document() != doc)
			edit->setDocument(doc);
	} else {
		QStyledItemDelegate::setEditorData(editor, index);
	}
}
