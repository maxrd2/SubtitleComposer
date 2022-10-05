/*
    SPDX-FileCopyrightText: 2020-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "richdocumentlayout.h"

#include "helpers/common.h"
#include "helpers/debug.h"
#include "core/richtext/richcss.h"
#include "core/richtext/richdocument.h"

#include <climits>

#include <QBasicTimer>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QSet>
#include <QStringBuilder>
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

QTextCharFormat
RichDocumentLayout::applyCSS(const QTextCharFormat &format) const
{
	QTextCharFormat fmt(format);
	const RichCSS *css = m_doc->stylesheet();
	if(!css)
		return fmt;

	QSet<QString> selectors;
	if(format.fontWeight() == QFont::Bold)
		selectors << $("b");
	if(format.fontItalic())
		selectors << $("i");
	if(format.fontUnderline())
		selectors << $("u");
	if(format.fontStrikeOut())
		selectors << $("s");
	if(format.hasProperty(RichDocument::Class)) {
		selectors << $("c");
		selectors << QChar('.') % format.property(RichDocument::Class).toString();
	}
	if(format.hasProperty(RichDocument::Voice)) {
		selectors << $("v");
		selectors << $("v") % QChar(' ') % format.property(RichDocument::Voice).toString();
	}

	QMap<QByteArray, QString> styles = css->match(selectors);
	for(auto it = styles.cbegin(); it != styles.cend(); ++it) {
		if(it.key() == "font-weight") {
			static const QMap<QString, QFont::Weight> wm = {
				{ $("normal"), QFont::Normal },
				{ $("bold"), QFont::Bold },
				{ $("100"), QFont::Thin },
				{ $("200"), QFont::ExtraLight },
				{ $("300"), QFont::Light },
				{ $("400"), QFont::Normal },
				{ $("500"), QFont::Medium },
				{ $("600"), QFont::DemiBold },
				{ $("700"), QFont::Bold },
				{ $("800"), QFont::ExtraBold },
				{ $("900"), QFont::Black },
			};
			auto iw = wm.find(it.value());
			if(iw != wm.cend())
				fmt.setFontWeight(iw.value());
		} else if(it.key() == "font-style") {
			fmt.setFontItalic(it.value() != $("normal"));
		} else if(it.key() == "text-decoration") {
			fmt.setFontUnderline(it.value() == $("underline"));
			fmt.setFontStrikeOut(it.value() == $("line-through"));
		} else if(it.key() == "color") {
			QColor color;
			color.setNamedColor(it.value());
			fmt.setForeground(QBrush(color));
		} else if(it.key() == "background-color") {
			QColor color;
			color.setNamedColor(it.value());
			fmt.setBackground(QBrush(color));
		}
		// TODO: check what else WebVTT requires
	}
	return fmt;
}

QVector<QTextLayout::FormatRange>
RichDocumentLayout::applyCSS(const QVector<QTextLayout::FormatRange> &docFormat) const
{
	QVector<QTextLayout::FormatRange> fmts;
	for(auto it = docFormat.cbegin(); it != docFormat.cend(); ++it)
		fmts.push_back(QTextLayout::FormatRange{it->start, it->length, applyCSS(it->format)});
	return fmts;
}

void
RichDocumentLayout::mergeFormat(QTextCharFormat &fmt, const QTextCharFormat &upper)
{
	const QVariant &u = upper.property(RichDocument::Merged);
	QTextFormat upp = u.isNull() ? static_cast<QTextFormat>(upper) : u.value<QTextFormat>();

	const QMap<int, QVariant> pa = fmt.properties();
	const QMap<int, QVariant> pb = upp.properties();
	for(auto it = pb.cbegin(); it != pb.cend(); ++it) {
		if(pa.contains(it.key()) && (it.key() == QTextFormat::FontWeight || it.key() == QTextFormat::FontItalic
		|| it.key() == QTextFormat::FontUnderline || it.key() == QTextFormat::TextUnderlineStyle
		|| it.key() == QTextFormat::FontStrikeOut)) {
			const QVariant &v = pa.find(it.key()).value();
			fmt.setProperty(it.key(), v.toInt() > it.value().toInt() ? v : it.value());
		} else {
			fmt.setProperty(it.key(), it.value());
		}
	}
	if(!fmt.isEmpty())
		fmt.setProperty(RichDocument::Merged, upp);
}

QVector<QTextLayout::FormatRange>
RichDocumentLayout::mergeCSS(const QVector<QTextLayout::FormatRange> &docFormat, const QVector<QTextLayout::FormatRange> &layoutFormat) const
{
	QVector<QTextLayout::FormatRange> fmts;
	auto di = docFormat.cbegin();
	auto li = layoutFormat.cbegin();
	int off = 0;
	bool docFmtValid = false;
	QTextCharFormat docFmt;
	for(;;) {
		const bool offPastDoc = di == docFormat.cend();
		if(!offPastDoc) {
			if(!docFmtValid) {
				docFmt = applyCSS(di->format);
				docFmtValid = true;
			}
			if(off >= di->start + di->length) {
				++di;
				docFmtValid = false;
				continue;
			}
		}
		const bool offPastLayout = li == layoutFormat.cend();
		if(!offPastLayout && off >= li->start + li->length) {
			++li;
			continue;
		}
		if(offPastDoc && offPastLayout)
			break;

		bool offNotInDoc = offPastDoc || off < di->start;
		bool offNotInLayout = offPastLayout || off < li->start;
		if(offNotInDoc && offNotInLayout) {
			if(offPastDoc)
				off = li->start;
			else if(offPastLayout)
				off = di->start;
			else
				off = qMin(di->start, li->start);
			continue;
		}

		Q_ASSERT(!offNotInDoc || !offNotInLayout);
		QTextCharFormat fmt;
		if(!offNotInDoc)
			fmt = docFmt;
		mergeFormat(fmt, offNotInLayout ? QTextCharFormat() : li->format);
		int end;
		if(!offNotInDoc && !offNotInLayout)
			end = qMin(di->start + di->length, li->start + li->length);
		else if(!offNotInDoc) // && offNotInLayout
			end = offPastLayout ? di->start + di->length : qMin(di->start + di->length, li->start);
		else // !offNotInLayout && offNotInDoc
			end = offPastDoc ? li->start + li->length : qMin(li->start + li->length, di->start);
		if(!fmt.isEmpty())
			fmts.push_back(QTextLayout::FormatRange{off, end - off, fmt});
		off = end;
	}
	return fmts;
}

void
RichDocumentLayout::processLayout(int from, int oldLength, int length)
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
		tl->setFormats(mergeCSS(bi.textFormats(), tl->formats()));
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

	if(!updateRect.isValid() || oldLength || m_layoutSize != newSize)
		updateRect = QRectF(0., 0., qreal(INT_MAX), qreal(INT_MAX));
	emit update(updateRect);
}

void
RichDocumentLayout::flagDirty()
{
	m_layoutPosition = 0;
	m_doc->markContentsDirty(0, m_doc->length());
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
			painter->save();
			if(bg.style() < Qt::LinearGradientPattern || bg.style() > Qt::ConicalGradientPattern)
				painter->setBrushOrigin(rc.topLeft());
			painter->fillRect(rc, bg);
			painter->restore();
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
	const_cast<RichDocumentLayout *>(this)->processLayout(m_layoutPosition, 0, position - m_layoutPosition);
}

void
RichDocumentLayout::documentChanged(int from, int oldLength, int length)
{
	if(m_layoutPosition > from)
		m_layoutPosition = from;
	processLayout(from, oldLength, length);
}
