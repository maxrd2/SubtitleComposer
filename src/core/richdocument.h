#ifndef RICHDOCUMENT_H
#define RICHDOCUMENT_H
/*
 * SPDX-FileCopyrightText: 2020 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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

#include "core/sstring.h"

#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QObject>
#include <QPalette>

QT_FORWARD_DECLARE_CLASS(QStyle)
QT_FORWARD_DECLARE_CLASS(QStyleOptionViewItem)

namespace SubtitleComposer {

class RichDocument : public QTextDocument
{
	Q_OBJECT

public:
	explicit RichDocument(QObject *parent=nullptr);
	virtual ~RichDocument();

	SString toRichText() const;
	void setRichText(const SString &text, bool resetUndo=false);

	QString toHtml() const;
	void setHtml(const QString &html, bool resetUndo=false);

	void setPlainText(const QString &text, bool resetUndo=false);

	void setDocument(const QTextDocument *doc, bool resetUndo=false);

	void clear(bool resetUndo);
	inline void clear() override { clear(false); }

	inline int length() const { const QTextBlock &b = lastBlock(); return b.position() + b.length(); }

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

public slots:
	inline void undo() { QTextDocument::undo(&m_undoableCursor); }
	inline void redo() { QTextDocument::redo(&m_undoableCursor); }

private:
	inline void setUndoRedoEnabled(bool enable) { QTextDocument::setUndoRedoEnabled(enable); }
	inline int length(int index, int len) const { const int dl = length(); return len < 0 || (index + len) > dl ? dl - index : len; }
	void linesToBlocks();

private:
	QTextCursor m_undoableCursor;

	void applyChanges(const void *changeList);

	Q_DISABLE_COPY(RichDocument)
};

}

#endif // RICHDOCUMENT_H
