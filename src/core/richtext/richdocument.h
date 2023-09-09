/*
    SPDX-FileCopyrightText: 2020-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RICHDOCUMENT_H
#define RICHDOCUMENT_H

#include "core/richstring.h"
#include "core/richtext/richcss.h"
#include "core/richtext/richdom.h"
#include "core/richtext/richdocumentlayout.h"

#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFormat>
#include <QObject>
#include <QPalette>

QT_FORWARD_DECLARE_CLASS(QStyle)
QT_FORWARD_DECLARE_CLASS(QStyleOptionViewItem)

namespace SubtitleComposer {
class RichDOM;

class RichDocument : public QTextDocument
{
	Q_OBJECT

public:
	enum Property {
		Class = QTextFormat::UserProperty,
		Voice = QTextFormat::UserProperty + 1,
		Merged = QTextFormat::UserProperty + 2,
	};
	Q_ENUM(Property)

	explicit RichDocument(QObject *parent=nullptr);
	virtual ~RichDocument();

	RichString toRichText() const;
	void setRichText(const RichString &text, bool resetUndo=false);

	QString toHtml() const;
	void setHtml(const QString &html, bool resetUndo=false);

	void setPlainText(const QString &text, bool resetUndo=false);

	void setDocument(const QTextDocument *doc, bool resetUndo=false);

	inline void setDocumentLayout(RichDocumentLayout *layout) { QTextDocument::setDocumentLayout(layout); }
	inline RichDocumentLayout * documentLayout() const { return static_cast<RichDocumentLayout *>(QTextDocument::documentLayout()); }

	void clear(bool resetUndo);
	inline void clear() override { clear(false); }

	inline int length() const { const QTextBlock &b = lastBlock(); return b.position() + b.length(); }

	void joinLines();

	void replace(const QRegularExpression &search, const QString &replacement, bool replacementIsHtml=true);
	void replace(QChar before, QChar after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
	void replace(int index, int len, const QString &replacement);
	int indexOf(const QRegularExpression &re, int from = 0);
	int cummulativeStyleFlags() const;
	QRgb styleColorAt(int index) const;

	void cleanupSpaces();
	void fixPunctuation(bool spaces, bool quotes, bool englishI, bool ellipsis, bool *cont, bool testOnly=false);
	void toLower();
	void toUpper();
	void toSentenceCase(bool *isSentenceStart, bool convertLowerCase=true, bool titleCase=false, bool testOnly=false);
	void breakText(int minBreakLength);

	inline QTextCursor *undoableCursor() { return &m_undoableCursor; }

	void setStylesheet(const RichCSS *css);
	inline const RichCSS *stylesheet() const { return m_stylesheet; }

	RichDOM *dom();
	QString crumbAt(RichDOM::Node *n);
	RichDOM::Node *nodeAt(quint32 pos, RichDOM::Node *root=nullptr);
	inline QString crumbAt(quint32 pos) { return crumbAt(nodeAt(pos ? pos - 1 : 0, dom()->m_root)); }

signals:
	void domChanged();

public slots:
	inline void undo() { QTextDocument::undo(&m_undoableCursor); }
	inline void redo() { QTextDocument::redo(&m_undoableCursor); }

private:
	inline void setUndoRedoEnabled(bool enable) { QTextDocument::setUndoRedoEnabled(enable); }
	inline int length(int index, int len) const { const int dl = length(); return len < 0 || (index + len) > dl ? dl - index : len; }
	void linesToBlocks();

	void markStylesheetDirty();

private:
	QTextCursor m_undoableCursor;
	const RichCSS *m_stylesheet;
	bool m_domDirty;
	RichDOM *m_dom;

	void applyChanges(const void *changeList);

	Q_DISABLE_COPY(RichDocument)
};

}

#endif // RICHDOCUMENT_H
