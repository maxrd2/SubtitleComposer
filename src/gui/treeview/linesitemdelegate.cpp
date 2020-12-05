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

#include <QApplication>
#include <QAbstractTextDocumentLayout>
#include <QKeyEvent>
#include <QPainter>
#include <QTextDocument>
#include <QTextOption>

using namespace SubtitleComposer;

LinesItemDelegate::LinesItemDelegate(bool useStyle, bool singleLineMode, bool richTextMode, LinesWidget *parent)
	: QStyledItemDelegate(parent),
	  m_useStyle(useStyle),
	  m_singleLineMode(singleLineMode),
	  m_textDocument(nullptr)
{
	setRichTextMode(richTextMode);
}

LinesItemDelegate::~LinesItemDelegate()
{
	delete m_textDocument;
}

bool
LinesItemDelegate::useStyle() const
{
	return m_useStyle;
}

void
LinesItemDelegate::setUseStyle(bool useStyle)
{
	m_useStyle = useStyle;
}

bool
LinesItemDelegate::singleLineMode() const
{
	return m_singleLineMode;
}

void
LinesItemDelegate::setSingleLineMode(bool singleLineMode)
{
	m_singleLineMode = singleLineMode;
}

bool
LinesItemDelegate::richTextMode() const
{
	return m_textDocument != nullptr;
}

void
LinesItemDelegate::setRichTextMode(bool richTextMode)
{
	if(richTextMode != (m_textDocument != nullptr)) {
		if(richTextMode) {
			QTextOption defaultTextOption;
			defaultTextOption.setWrapMode(QTextOption::NoWrap);

			m_textDocument = new QTextDocument();
			m_textDocument->setDefaultTextOption(defaultTextOption);
			m_textDocument->setUndoRedoEnabled(false);
			updateStyle();
		} else {
			delete m_textDocument;
			m_textDocument = nullptr;
		}
	}
}

void
LinesItemDelegate::updateStyle()
{
	if(m_textDocument) {
		m_textDocument->setDefaultStyleSheet("p { display:inline; white-space:pre; vertical-align:baseline; margin:0; }");
		const LinesWidget *lw = static_cast<LinesWidget *>(parent());
		m_textDocument->setDefaultFont(QApplication::font(lw));
	}
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
			emit closeEditor(editor, (QAbstractItemDelegate::EndEditHint)EditUpperItem);
			return true;

		case Qt::Key_Down:
			emit commitData(editor);
			emit closeEditor(editor, (QAbstractItemDelegate::EndEditHint)EditLowerItem);
			return true;

		default:
			break;
		}
	}

	return QStyledItemDelegate::eventFilter(object, event);
}

void
LinesItemDelegate::drawBackgroundPrimitive(QPainter *painter, const QStyle *style, const QStyleOptionViewItem &option) const
{
	if(m_useStyle) {
		style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, option.widget);
	} else {
		QPalette::ColorGroup cg = option.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;

		if(cg == QPalette::Normal && !(option.state & QStyle::State_Active))
			cg = QPalette::Inactive;

		if(option.showDecorationSelected && (option.state & QStyle::State_Selected)) {
			painter->fillRect(option.rect, option.palette.brush(cg, QPalette::Highlight));
		} else {
			if(option.backgroundBrush.style() != Qt::NoBrush) {
				QPointF oldBO = painter->brushOrigin();
				painter->setBrushOrigin(option.rect.topLeft());
				painter->fillRect(option.rect, option.backgroundBrush);
				painter->setBrushOrigin(oldBO);
			}

			if(option.state & QStyle::State_Selected) {
				QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &option, option.widget);
				painter->fillRect(textRect, option.palette.brush(cg, QPalette::Highlight));
			}
		}
	}
}

void
LinesItemDelegate::drawTextPrimitive(QPainter *painter, const QStyle *style, const QStyleOptionViewItem &option, const QRect &rect, QPalette::ColorGroup cg, const QModelIndex &index) const
{
	const int textMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, nullptr, option.widget) + 1;
	const int alignment = QStyle::visualAlignment(option.direction, option.displayAlignment);

	QRect textRect = rect.adjusted(textMargin, 0, -textMargin, 0);  // remove width padding

	QString text = QString(option.text);

	QFontMetrics fm = painter->fontMetrics();
	text = fm.elidedText(option.text, option.textElideMode, textRect.width());

	if(m_textDocument) {
		QTextOption textOption;
		textOption.setTextDirection(option.direction);
		textOption.setAlignment((Qt::Alignment)alignment);

		m_textDocument->setDefaultTextOption(textOption);
		m_textDocument->setTextWidth(textRect.width() - textMargin);
		m_textDocument->setHtml("<p>" + text + "</p>");

		QPalette palette(option.palette);
		palette.setColor(QPalette::Text, palette.color(cg, option.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text));

		QAbstractTextDocumentLayout::PaintContext context;
		context.palette = palette;

		const int yOffset = (textRect.height() - m_textDocument->documentLayout()->documentSize().height()) / 2.;
		textRect.adjust(0, yOffset, 0, yOffset);

		painter->translate(textRect.x(), textRect.y());
		m_textDocument->documentLayout()->draw(painter, context);
		painter->translate(-textRect.x(), -textRect.y());
	} else {
		QColor textColor;
		if(option.state & QStyle::State_Selected)
			textColor = option.palette.color(cg, QPalette::HighlightedText);
		else
			textColor = option.palette.color(cg, QPalette::Text);

		if(index.column() == LinesModel::ShowTime && index.data(LinesModel::AnchoredRole).toInt() == -1)
			textColor.setAlpha(50);

		painter->setPen(textColor);

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
	drawBackgroundPrimitive(painter, style, option);

	// draw the icon(s)
	bool showMarkedIcon = index.data(LinesModel::MarkedRole).toBool();
	bool showErrorIcon = index.data(LinesModel::ErrorRole).toBool();

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
	if(!option.text.isEmpty()) {
		if(option.state & QStyle::State_Editing) {
			painter->setPen(option.palette.color(cg, QPalette::Text));
			painter->drawRect(textRect.adjusted(0, 0, -1, -1));
		}

		drawTextPrimitive(painter, style, option, textRect, cg, index);
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
	static const QChar pipeChar('|');
	static const QChar newLineChar(QChar::LineSeparator);

	if(value.type() == QVariant::String)
		return value.toString().replace('\n', m_singleLineMode ? pipeChar : newLineChar);
	else
		return QStyledItemDelegate::displayText(value, locale);
}

const QIcon &
LinesItemDelegate::markIcon()
{
	static QIcon icon;
	if(icon.isNull())
		icon = QIcon::fromTheme(QStringLiteral("dialog-warning"));
	return icon;
}

const QIcon &
LinesItemDelegate::errorIcon()
{
	static QIcon icon;
	if(icon.isNull())
		icon = QIcon::fromTheme(QStringLiteral("dialog-error"));
	return icon;
}

const QIcon &
LinesItemDelegate::anchorIcon()
{
	static QIcon icon;
	if(icon.isNull())
		icon = QIcon::fromTheme(QStringLiteral("anchor"));
	return icon;
}
