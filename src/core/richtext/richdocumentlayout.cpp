/*
    SPDX-FileCopyrightText: 2020-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "richdocumentlayout.h"

#include "core/richtext/richdocument.h"

#include <climits>

#include <QBasicTimer>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QtMath>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextFrame>
#include <QTextLayout>


using namespace SubtitleComposer;

Q_GUI_EXPORT int qt_defaultDpiY();

RichDocumentLayout::RichDocumentLayout(RichDocument *doc)
	: QAbstractTextDocumentLayout(doc),
	  m_doc(doc),
	  m_layoutPosition(0)
{
}

void
RichDocumentLayout::layout(int from, int oldLength, int length)
{
	Q_UNUSED(oldLength);

	const QTextFrameFormat &ff = m_doc->rootFrame()->frameFormat();
	const qreal lineLeft = ff.border() + ff.padding() + ff.leftMargin();
	const qreal lineRight = m_doc->pageSize().width() - ff.border() - ff.padding() - ff.rightMargin();
	const qreal lineWidth = ff.width().value(lineRight - lineLeft);

	QTextBlock bi = m_doc->begin();
	QTextBlock end = m_doc->findBlock(qMin(m_layoutPosition, from));

	qreal width = 0;
	qreal height = ff.border() + ff.padding() + ff.topMargin();

	for(; bi != end; bi = bi.next()) {
		QTextLayout *tl = bi.layout();
		tl->setPosition(QPointF(lineLeft, height));
		const int n = tl->lineCount();
		for(int i = 0; i < n; i++) {
			QTextLine line = tl->lineAt(i);
			width = qMax(width, line.naturalTextWidth());
			height += line.height();
		}
	}

	QRectF updateRect;
	updateRect.setTopLeft(QPointF(lineLeft, height));
	end = m_doc->findBlock(qMax(0, from + length));
	if(end.isValid())
		end = end.next();
	for(; bi != end; bi = bi.next()) {
		if(!bi.isVisible())
			continue;
		QTextLayout *tl = bi.layout();
		const qreal layoutStart = height;
		tl->setPosition(QPointF(lineLeft, layoutStart));

		QTextOption option = m_doc->defaultTextOption();
		const QTextBlockFormat &bf = bi.blockFormat();
		option.setTextDirection(bf.layoutDirection());
		option.setTabs(bf.tabPositions());
		Qt::Alignment align = option.alignment();
		if(bf.hasProperty(QTextFormat::BlockAlignment))
			align = bf.alignment();
		option.setAlignment(align);
		if(bf.nonBreakableLines() || m_doc->pageSize().width() < 0)
			option.setWrapMode(QTextOption::ManualWrap);
		tl->setTextOption(option);

		tl->beginLayout();
		for(;;) {
			QTextLine line = tl->createLine();
			if(!line.isValid())
				break;
			line.setLeadingIncluded(true);
			line.setLineWidth(lineWidth);
			line.setPosition(QPointF(0., height - layoutStart));
			width = qMax(width, line.naturalTextWidth());
			height += line.height();
		}
		tl->endLayout();

		m_layoutPosition = bi.position() + bi.length();
	}
	updateRect.setBottomRight(QPointF(lineRight, height));

	end = m_doc->end();
	if(bi == end) {
		oldLength = 1; // needed to make updateRect full area if document got shorter
		m_layoutPosition = INT_MAX;
	} else {
		for(; bi != end; bi = bi.next()) {
			QTextLayout *tl = bi.layout();
			const QPointF newPos(lineLeft, height);
			tl->setPosition(newPos);
			const int n = tl->lineCount();
			for(int i = 0; i < n; i++) {
				QTextLine line = tl->lineAt(i);
				width = qMax(width, line.naturalTextWidth());
				height += line.height();
			}
		}
	}

	height += ff.border() + ff.padding() + ff.bottomMargin();
	const QSizeF newSize(m_doc->pageSize().width(), height);
	if(m_layoutSize != newSize) {
		m_layoutSize = newSize;
		m_naturalSize = QSizeF(width, height);
		emit documentSizeChanged(m_layoutSize);
	}

	if(oldLength || m_layoutSize != newSize)
		updateRect.setSize(QSizeF(qreal(INT_MAX), qreal(INT_MAX)));
	emit update(updateRect);
}

void
RichDocumentLayout::layout()
{
	layout(0, 0, INT_MAX);
}

static void
fillBackground(QPainter *p, const QRectF &rect, QBrush brush, const QPointF &origin, const QRectF &gradientRect = QRectF())
{
	p->save();
	if(brush.style() >= Qt::LinearGradientPattern && brush.style() <= Qt::ConicalGradientPattern) {
		if(!gradientRect.isNull()) {
			QTransform m;
			m.translate(gradientRect.left(), gradientRect.top());
			m.scale(gradientRect.width(), gradientRect.height());
			brush.setTransform(m);
			const_cast<QGradient *>(brush.gradient())->setCoordinateMode(QGradient::LogicalMode);
		}
	} else {
		p->setBrushOrigin(origin);
	}
	p->fillRect(rect, brush);
	p->restore();
}

void
RichDocumentLayout::draw(QPainter *painter, const PaintContext &context)
{
	ensureLayout(INT_MAX);

	for(QTextBlock bi = m_doc->begin(); bi != m_doc->end(); bi = bi.next()) {
		QTextLayout *bl = bi.layout();
		const int bPos = bi.position();
		const int bLen = bi.length();

		const QBrush bg = bi.blockFormat().background();
		if(bg != Qt::NoBrush) {
			const QRectF rc = bl->boundingRect().translated(bl->position());
			fillBackground(painter, rc, bg, rc.topLeft());
		}

		// draw selection
		QVector<QTextLayout::FormatRange> selections;
		for(const Selection &s: context.selections) {
			const int ss = qMin(qMax(0, s.cursor.selectionStart() - bPos), bLen);
			const int sl = qMin(qMax(0, s.cursor.selectionEnd() - bPos), bLen) - ss;
			if(sl > 0)
				selections.append(QTextLayout::FormatRange{ss, sl, s.format});
		}

		// draw text
		bl->draw(painter, QPointF(), selections, context.clip);

		// draw cursor
		if(context.cursorPosition >= 0) {
			const int off = context.cursorPosition - bi.position();
			if(off >= 0 && off < bi.length())
				bl->drawCursor(painter, QPointF(), off, 1);
		}
	}
}

int
RichDocumentLayout::hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const
{
	ensureLayout(INT_MAX);

	const QTextBlock end = m_doc->end();
	for(QTextBlock bi = m_doc->begin(); bi != end; bi = bi.next()) {
		 const QTextLayout *tl = bi.layout();
		 const QRectF brc = tl->boundingRect().translated(tl->position());
		 if(point.y() < brc.top())
			 return accuracy == Qt::ExactHit ? -1 : 0;
		 if(point.y() > brc.bottom())
			 continue;
		 // point inside block rect
		 const QTextLine::CursorPosition cp = accuracy == Qt::ExactHit ? QTextLine::CursorOnCharacter : QTextLine::CursorBetweenCharacters;
		 const int n = tl->lineCount();
		 for(int i = 0; i < n; i++) {
			const QTextLine &ln = tl->lineAt(i);
			const QRectF lrc = ln.naturalTextRect().translated(tl->position());
			if(point.y() < lrc.top())
				return accuracy == Qt::ExactHit ? -1 : 0;
			if(point.y() > lrc.bottom())
				continue;
			// point inside line rect
			return bi.position() + ln.xToCursor(point.x() - tl->position().x(), cp);
		 }
		 break;
	}
	if(accuracy == Qt::ExactHit)
		return -1;
	const QTextBlock bi = m_doc->lastBlock();
	return bi.position() + bi.length() - 1;
}


int
RichDocumentLayout::pageCount() const
{
	const qreal pgHeight = m_doc->pageSize().height();
	if(pgHeight < 0)
		return 1;
	ensureLayout(INT_MAX);
	return qCeil(m_layoutSize.height() / pgHeight);
}

QSizeF
RichDocumentLayout::documentSize() const
{
	ensureLayout(INT_MAX);
	return m_layoutSize;
}

QSizeF
RichDocumentLayout::minimumDocumentSize() const
{
	ensureLayout(INT_MAX);
	return m_naturalSize;
}

QRectF
RichDocumentLayout::frameBoundingRect(QTextFrame *frame) const
{
	Q_ASSERT(frame == m_doc->rootFrame());
	return QRectF(QPointF(0., 0.), m_layoutSize);
}

QRectF
RichDocumentLayout::blockBoundingRect(const QTextBlock &block) const
{
	const QTextLayout *tl = block.layout();
	QRectF rc = tl->boundingRect();
	rc.translate(tl->position());
	return rc;
}

void
RichDocumentLayout::ensureLayout(int position) const
{
	if(position <= m_layoutPosition)
		return;
	const_cast<RichDocumentLayout *>(this)->layout(m_layoutPosition, 0, position - m_layoutPosition);
}

void
RichDocumentLayout::documentChanged(int from, int oldLength, int length)
{
	layout(from, oldLength, length);
}
