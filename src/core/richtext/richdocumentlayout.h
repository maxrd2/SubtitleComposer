/*
    SPDX-FileCopyrightText: 2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RICHDOCUMENTLAYOUT_H
#define RICHDOCUMENTLAYOUT_H

#include <QAbstractTextDocumentLayout>
#include <QTextCharFormat>

QT_FORWARD_DECLARE_CLASS(QBasicTimer)

namespace SubtitleComposer {
class RichDocument;

class RichDocumentLayout : public QAbstractTextDocumentLayout
{
	Q_OBJECT

public:
	explicit RichDocumentLayout(RichDocument *doc);

	void flagDirty();

	void draw(QPainter *painter, const PaintContext &context) override;
	int hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const override;

	int pageCount() const override;
	QSizeF documentSize() const override;
	QSizeF minimumDocumentSize() const;

	QRectF frameBoundingRect(QTextFrame *frame) const override;
	QRectF blockBoundingRect(const QTextBlock &block) const override;

	QTextCharFormat applyCSS(const QTextCharFormat &format) const;
	QVector<QTextLayout::FormatRange> applyCSS(const QVector<QTextLayout::FormatRange> &docFormat) const;
	static void mergeFormat(QTextCharFormat &fmt, const QTextCharFormat &upper);
	QVector<QTextLayout::FormatRange> mergeCSS(const QVector<QTextLayout::FormatRange> &docFormat, const QVector<QTextLayout::FormatRange> &layoutFormat) const;

protected:
	void documentChanged(int from, int oldLength, int length) override;

private:
	void ensureLayout(int position) const;
	void processLayout(int from, int oldLength, int length);

private:
	RichDocument *m_doc;
	int m_layoutPosition;
	QSizeF m_layoutSize;
	QSizeF m_naturalSize;
};

}

#endif // RICHDOCUMENTLAYOUT_H
