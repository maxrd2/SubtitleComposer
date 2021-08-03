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

#include "core/richdocument.h"
#include "core/subtitle.h"
#include "core/subtitleline.h"
#include "core/subtitleiterator.h"
#include "core/undo/subtitleactions.h"
#include "core/undo/subtitlelineactions.h"
#include "core/undo/undostack.h"
#include "helpers/objectref.h"
#include "scconfig.h"
#include "application.h"
#include "gui/treeview/lineswidget.h"

#include <QTextDocumentFragment>

#include <KLocalizedString>

using namespace SubtitleComposer;

double Subtitle::s_defaultFramesPerSecond(23.976);

double
Subtitle::defaultFramesPerSecond()
{
	return s_defaultFramesPerSecond;
}

void
Subtitle::setDefaultFramesPerSecond(double framesPerSecond)
{
	s_defaultFramesPerSecond = framesPerSecond;
}

Subtitle::Subtitle(double framesPerSecond)
	: m_primaryDirtyState(false),
	  m_primaryCleanIndex(0),
	  m_secondaryDirtyState(false),
	  m_secondaryCleanIndex(0),
	  m_framesPerSecond(framesPerSecond),
	  m_formatData(nullptr)
{}

Subtitle::~Subtitle()
{
	qDeleteAll(m_lines);

	delete m_formatData;
}

void
Subtitle::setPrimaryData(const Subtitle &from, bool usePrimaryData)
{
	beginCompositeAction(i18n("Set Primary Data"));

	setFormatData(from.m_formatData);

	setFramesPerSecond(from.framesPerSecond());

	SubtitleIterator fromIt(from, Range::full());
	SubtitleIterator thisIt(*this, Range::full());

	// the errors that we are going to take from 'from':
	const int fromErrors = (usePrimaryData ? SubtitleLine::PrimaryOnlyErrors : SubtitleLine::SecondaryOnlyErrors) | SubtitleLine::SharedErrors;
	// the errors that we are going to keep:
	const int thisErrors = SubtitleLine::SecondaryOnlyErrors;

	for(SubtitleLine *fromLine = fromIt.current(), *thisLine = thisIt.current(); fromLine && thisLine; ++fromIt, ++thisIt, fromLine = fromIt.current(), thisLine = thisIt.current()) {
		thisLine->setPrimaryDoc(usePrimaryData ? fromLine->primaryDoc() : fromLine->secondaryDoc());
		thisLine->setTimes(fromLine->showTime(), fromLine->hideTime());
		thisLine->setErrorFlags((fromLine->errorFlags() & fromErrors) | (thisLine->errorFlags() & thisErrors));
		thisLine->setFormatData(fromLine->formatData());
	}

	if(fromIt.current()) { // from has more lines
		QList<SubtitleLine *> lines;
		for(; fromIt.current(); ++fromIt) {
			const SubtitleLine *cur = fromIt.current();
			SubtitleLine *thisLine = new SubtitleLine(cur->showTime(), cur->hideTime());
			thisLine->setPrimaryDoc(usePrimaryData ? cur->primaryDoc() : cur->secondaryDoc());
			thisLine->setErrorFlags(SubtitleLine::SecondaryOnlyErrors, false);
			thisLine->setFormatData(cur->formatData());
			lines.append(thisLine);
		}
		processAction(new InsertLinesAction(this, lines));
	} else if(thisIt.current()) { // this has more lines
		for(SubtitleLine *thisLine = thisIt.current(); thisLine; ++thisIt, thisLine = thisIt.current()) {
			thisLine->primaryDoc()->clear();
			thisLine->setErrorFlags(SubtitleLine::PrimaryOnlyErrors, false);
			thisLine->setFormatData(0);
		}
	}

	endCompositeAction();
}

void
Subtitle::clearPrimaryTextData()
{
	beginCompositeAction(i18n("Clear Primary Text Data"));

	for(SubtitleIterator it(*this); it.current(); ++it) {
		it.current()->primaryDoc()->clear();
		it.current()->setErrorFlags(SubtitleLine::PrimaryOnlyErrors, false);
	}

	endCompositeAction();
}

void
Subtitle::setSecondaryData(const Subtitle &from, bool usePrimaryData)
{
	beginCompositeAction(i18n("Set Secondary Data"));

	const int srcErrors = usePrimaryData ? SubtitleLine::PrimaryOnlyErrors : SubtitleLine::SecondaryOnlyErrors;
	const int dstErrors = SubtitleLine::PrimaryOnlyErrors | SubtitleLine::SharedErrors;

	for(int i = 0, n = qMin(m_lines.size(), from.m_lines.size()); i < n; i++) {
		const SubtitleLine *srcLine = from.m_lines.at(i).obj();
		SubtitleLine *dstLine = m_lines.at(i).obj();
		dstLine->setSecondaryDoc(usePrimaryData ? srcLine->primaryDoc() : srcLine->secondaryDoc());
		dstLine->setErrorFlags((dstLine->errorFlags() & dstErrors) | (srcLine->errorFlags() & srcErrors));
	}

	// clear remaining local translations
	for(int i = from.m_lines.size(), n = m_lines.size(); i < n; i++) {
		SubtitleLine *dstLine = m_lines.at(i).obj();
		dstLine->secondaryDoc()->clear();
		dstLine->setErrorFlags(SubtitleLine::SecondaryOnlyErrors, false);
	}

	// insert remaining source translations
	QList<SubtitleLine *> newLines;
	for(int i = m_lines.size(), n = from.m_lines.size(); i < n; i++) {
		const SubtitleLine *srcLine = from.m_lines.at(i).obj();
		SubtitleLine *dstLine = new SubtitleLine(srcLine->showTime(), srcLine->hideTime());
		dstLine->setSecondaryDoc(usePrimaryData ? srcLine->primaryDoc() : srcLine->secondaryDoc());
		dstLine->setErrorFlags(SubtitleLine::PrimaryOnlyErrors, false);
		newLines.append(dstLine);
	}
	if(!newLines.isEmpty())
		processAction(new InsertLinesAction(this, newLines));

	endCompositeAction(UndoStack::Secondary);
}

void
Subtitle::clearSecondaryTextData()
{
	beginCompositeAction(i18n("Clear Secondary Text Data"));

	for(SubtitleIterator it(*this); it.current(); ++it) {
		it.current()->secondaryDoc()->clear();
		it.current()->setErrorFlags(SubtitleLine::SecondaryOnlyErrors, false);
	}

	endCompositeAction();
}

void
Subtitle::clearPrimaryDirty()
{
	if(!m_primaryDirtyState)
		return;

	m_primaryDirtyState = false;
	m_primaryCleanIndex = app()->undoStack()->index();
	emit primaryDirtyStateChanged(false);
}

void
Subtitle::clearSecondaryDirty()
{
	if(!m_secondaryDirtyState)
		return;

	m_secondaryDirtyState = false;
	m_secondaryCleanIndex = app()->undoStack()->index();
	emit secondaryDirtyStateChanged(false);
}

FormatData *
Subtitle::formatData() const
{
	return m_formatData;
}

void
Subtitle::setFormatData(const FormatData *formatData)
{
	delete m_formatData;

	m_formatData = formatData ? new FormatData(*formatData) : NULL;
}

double
Subtitle::framesPerSecond() const
{
	return m_framesPerSecond;
}

void
Subtitle::setFramesPerSecond(double framesPerSecond)
{
	if(qAbs(m_framesPerSecond - framesPerSecond) > 1e-6)
		processAction(new SetFramesPerSecondAction(this, framesPerSecond));
}

void
Subtitle::changeFramesPerSecond(double toFramesPerSecond, double fromFramesPerSecond)
{
	if(toFramesPerSecond <= 0)
		return;

	if(fromFramesPerSecond <= 0)
		fromFramesPerSecond = m_framesPerSecond;

	beginCompositeAction(i18n("Change Frame Rate"));

	setFramesPerSecond(toFramesPerSecond);

	double scaleFactor = fromFramesPerSecond / toFramesPerSecond;

	if(scaleFactor != 1.0) {
		for(SubtitleIterator it(*this, Range::full()); it.current(); ++it) {
			Time showTime = it.current()->showTime();
			showTime *= scaleFactor;

			Time hideTime = it.current()->hideTime();
			hideTime *= scaleFactor;

			processAction(new SetLineTimesAction(it, showTime, hideTime));
		}
	}

	endCompositeAction();
}

SubtitleLine *
Subtitle::line(int index)
{
	return index < 0 || index >= m_lines.count() ? nullptr : m_lines.at(index).obj();
}

const SubtitleLine *
Subtitle::line(int index) const
{
	return index < 0 || index >= m_lines.count() ? nullptr : m_lines.at(index).obj();
}
bool
Subtitle::hasAnchors() const
{
	return !m_anchoredLines.empty();
}

bool
Subtitle::isLineAnchored(int index) const
{
	if(index < 0 || index >= m_lines.count())
		return false;

	return isLineAnchored(m_lines[index]);
}

bool
Subtitle::isLineAnchored(const SubtitleLine *line) const
{
	if(!line)
		return false;

	return m_anchoredLines.indexOf(line) != -1;
}

void
Subtitle::toggleLineAnchor(int index)
{
	if(index < 0 || index >= m_lines.count())
		return;

	toggleLineAnchor(m_lines[index]);
}

void
Subtitle::toggleLineAnchor(const SubtitleLine *line)
{
	if(!line)
		return;

	int anchorIndex = m_anchoredLines.indexOf(line);

	if(anchorIndex == -1)
		m_anchoredLines.append(line);
	else
		m_anchoredLines.removeAt(anchorIndex);

	emit lineAnchorChanged(line, anchorIndex == -1);
}

void
Subtitle::removeAllAnchors()
{
	QList<const SubtitleLine *> anchoredLines;

	m_anchoredLines.swap(anchoredLines);

	foreach(auto line, anchoredLines)
		emit lineAnchorChanged(line, false);
}

int
Subtitle::insertIndex(const Time &showTime, int start, int end) const
{
	while(end - start > 1) {
		const int mid = (start + end) / 2;
		if(showTime < m_lines.at(mid)->showTime())
			end = mid - 1;
		else
			start = mid;
	}
	if(m_lines.empty() || showTime < m_lines.at(start)->showTime())
		return start;
	return showTime < m_lines.at(end)->showTime() ? end : end + 1;
}

void
Subtitle::insertLine(SubtitleLine *line)
{
	QList<SubtitleLine *> lines;
	lines.append(line);
	processAction(new InsertLinesAction(this, lines, insertIndex(line->showTime())));
}

void
Subtitle::insertLine(SubtitleLine *line, int index)
{
	Q_ASSERT(index >= 0 && index <= m_lines.count());
	QList<SubtitleLine *> lines;
	lines.append(line);
	processAction(new InsertLinesAction(this, lines, index));
}

SubtitleLine *
Subtitle::insertNewLine(int index, bool insertAfter, SubtitleTarget target)
{
	Q_ASSERT(index <= count());

	if(index < 0)
		index = count();

	SubtitleLine *newLine = new SubtitleLine();
	const int newLineIndex = (target == Secondary) ? m_lines.count() : index;

	const double linePause = (double)SCConfig::linePause();
	const double lineDuration = (double)SCConfig::lineDuration();
	const double lineDurationAndPause = lineDuration + linePause;

	if(insertAfter) {
		if(newLineIndex) {              // there is a previous line
			const SubtitleLine *prevLine = at(newLineIndex - 1);
			newLine->setTimes(prevLine->hideTime() + linePause, prevLine->hideTime() + lineDurationAndPause);
		} else if(newLineIndex < count()) {     // there is a next line
			const SubtitleLine *nextLine = at(newLineIndex);
			newLine->setTimes(nextLine->showTime() - lineDurationAndPause, nextLine->showTime() - linePause);
		} else
			newLine->setHideTime(lineDuration);
	} else {
		if(newLineIndex < count()) {    // there is a next line
			const SubtitleLine *nextLine = at(newLineIndex);
			newLine->setTimes(nextLine->showTime() - lineDurationAndPause, nextLine->showTime() - linePause);
		} else if(newLineIndex) {       // there is a previous line
			const SubtitleLine *prevLine = at(newLineIndex - 1);
			newLine->setTimes(prevLine->hideTime() + linePause, prevLine->hideTime() + lineDurationAndPause);
		} else
			newLine->setHideTime(lineDuration);
	}

	if(target == Both || index == count()) {
		insertLine(newLine, newLineIndex);
	} else if(target == Primary) {
		beginCompositeAction(i18n("Insert Line"));

		insertLine(newLine, newLineIndex);

		SubtitleLine *line = newLine;
		SubtitleIterator it(*this, Range::full(), false);
		for(it.toIndex(newLineIndex + 1); it.current(); ++it) {
			line->setSecondaryDoc(it.current()->secondaryDoc());
			line = it.current();
		}
		line->secondaryDoc()->clear();

		endCompositeAction();
	} else if(target == Secondary) {
		beginCompositeAction(i18n("Insert Line"));

		insertLine(newLine, newLineIndex);

		SubtitleIterator it(*this, Range::full(), true);
		SubtitleLine *line = it.current();
		for(--it; it.index() >= index; --it) {
			line->setSecondaryDoc(it.current()->secondaryDoc());
			line = it.current();
		}
		line->secondaryDoc()->clear();

		newLine = line;

		endCompositeAction();
	}

	return newLine;
}

void
Subtitle::removeLines(const RangeList &r, SubtitleTarget target)
{
	if(m_lines.isEmpty())
		return;

	RangeList ranges = r;
	ranges.trimToIndex(m_lines.count() - 1);

	if(ranges.isEmpty())
		return;

	if(target == Both) {
		beginCompositeAction(i18n("Remove Lines"));

		RangeList::ConstIterator rangesIt = ranges.end(), begin = ranges.begin();
		do {
			rangesIt--;
			processAction(new RemoveLinesAction(this, rangesIt->start(), rangesIt->end()));
		} while(rangesIt != begin);

		endCompositeAction();
	} else if(target == Secondary) {
		beginCompositeAction(i18n("Remove Lines"));

		RangeList rangesComplement = ranges.complement();
		rangesComplement.trimToRange(Range(ranges.firstIndex(), m_lines.count() - 1));

		// we have to move the secondary texts up (we do it in chunks)
		SubtitleIterator srcIt(*this, rangesComplement);
		SubtitleIterator dstIt(*this, Range::upper(ranges.firstIndex()));
		for(; srcIt.current() && dstIt.current(); ++srcIt, ++dstIt)
			dstIt.current()->setSecondaryDoc(srcIt.current()->secondaryDoc());

		// the remaining lines secondary text must be cleared
		for(; dstIt.current(); ++dstIt)
			dstIt.current()->secondaryDoc()->clear();

		endCompositeAction();
	} else { // target == Primary
		beginCompositeAction(i18n("Remove Lines"));

		RangeList mutableRanges(ranges);
		mutableRanges.trimToIndex(m_lines.count() - 1);

		// first, we need to append as many empty lines as we're to remove
		// we insert them with a greater time than the one of the last (non deleted) line

		int linesCount = m_lines.count();

		Range lastRange = mutableRanges.last();
		int lastIndex = lastRange.end() == linesCount - 1 ? lastRange.start() - 1 : linesCount - 1;
		SubtitleLine *lastLine = lastIndex < linesCount ? at(lastIndex) : 0;
		Time showTime(lastLine ? lastLine->hideTime() + 100. : Time());
		Time hideTime(showTime + 1000.);

		QList<SubtitleLine *> lines;
		for(int index = 0, size = ranges.indexesCount(); index < size; ++index) {
			lines.append(new SubtitleLine(showTime, hideTime));
			showTime.shift(1100.);
			hideTime.shift(1100.);
		}

		processAction(new InsertLinesAction(this, lines));

		// then, we move the secondary texts down (we need to iterate from bottom to top for that)
		RangeList rangesComplement = mutableRanges.complement();

		SubtitleIterator srcIt(*this, Range(ranges.firstIndex(), m_lines.count() - lines.count() - 1), true);
		SubtitleIterator dstIt(*this, rangesComplement, true);
		for(; srcIt.current() && dstIt.current(); --srcIt, --dstIt)
			dstIt.current()->setSecondaryDoc(srcIt.current()->secondaryDoc());

		// finally, we can remove the specified lines
		RangeList::ConstIterator rangesIt = ranges.end(), begin = ranges.begin();
		do {
			rangesIt--;
			processAction(new RemoveLinesAction(this, rangesIt->start(), rangesIt->end()));
		} while(rangesIt != begin);

		endCompositeAction();
	}
}

void
Subtitle::swapTexts(const RangeList &ranges)
{
	processAction(new SwapLinesTextsAction(this, ranges));
}

void
Subtitle::splitLines(const RangeList &ranges)
{
	auto splitOnSpace = [&](RichDocument *doc)->bool{
		if(doc->isEmpty())
			return false;
		const QString &text = doc->toPlainText();
		QTextCursor *c = doc->undoableCursor();
		int len = text.length();
		int i = len / 2;
		int j = i + len % 2;
		for(; ; i--, j++) {
			if(text.at(i) == QChar::Space) {
				c->movePosition(QTextCursor::Start);
				c->movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, i);
				c->movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
				c->insertText(QString(QChar::LineFeed));
				return true;
			}
			if(text.at(j) == QChar::Space) {
				c->movePosition(QTextCursor::Start);
				c->movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, j);
				c->movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
				c->insertText(QString(QChar::LineFeed));
				return true;
			}
			if(i == 0) {
				c->movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
				c->insertText(QString(QChar::LineFeed));
				return true;
			}
		}
		return false;
	};

	beginCompositeAction(i18n("Split Lines"));

	bool hasMultipleLines = false;

	for(SubtitleIterator it(*this, ranges, true); it.current(); --it) {
		SubtitleLine *ln = it.current();
		ln->simplifyTextWhiteSpace(Both);
		if(!hasMultipleLines && (ln->primaryDoc()->lineCount() > 1 || ln->secondaryDoc()->lineCount() > 1))
			hasMultipleLines = true;
	}

	for(SubtitleIterator it(*this, ranges, true); it.current(); --it) {
		SubtitleLine *line = it.current();

		if(line->primaryDoc()->isEmpty())
			continue;

		if(!hasMultipleLines) {
			if(splitOnSpace(line->primaryDoc()))
				splitOnSpace(line->secondaryDoc());
			else
				continue;
		}

		QTextCursor c1(line->primaryDoc());
		QTextCursor c2(line->secondaryDoc());

		c1.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
		QVector<quint32> dur;
		dur.push_back(c1.selectedText().size());
		quint32 totalDuration = dur.back();

		QVector<SubtitleLine *> newLines;
		for(;;) {
			if(!c1.movePosition(QTextCursor::NextBlock))
				c1.movePosition(QTextCursor::EndOfBlock);
			if(!c2.movePosition(QTextCursor::NextBlock))
				c2.movePosition(QTextCursor::EndOfBlock);
			if(c1.atEnd() && c2.atEnd())
				break;

			c1.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
			c2.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

			dur.push_back(c1.selectedText().size());
			totalDuration += dur.back();

			SubtitleLine *nl = new SubtitleLine();
			QTextCursor(nl->primaryDoc()).insertFragment(c1.selection());
			QTextCursor(nl->secondaryDoc()).insertFragment(c2.selection());
			newLines.push_back(nl);
		}

		c1.movePosition(QTextCursor::Start);
		c1.movePosition(QTextCursor::EndOfBlock);
		c1.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
		c1.removeSelectedText();

		c2.movePosition(QTextCursor::Start);
		c2.movePosition(QTextCursor::EndOfBlock);
		c2.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
		c2.removeSelectedText();

		const double lineDur = line->durationTime().toMillis();
		SubtitleLine *pl = line;
		auto duri = dur.begin();
		pl->setDurationTime(lineDur * *duri / totalDuration);
		for(SubtitleLine *nl: newLines) {
			++duri;
			const Time st = pl->hideTime() + 1.;
			nl->setTimes(st, qMax(st, pl->hideTime() + lineDur * *duri / totalDuration));
			insertLine(nl, pl->index() + 1);
			pl = nl;
		}
	}

	endCompositeAction();
}

void
Subtitle::joinLines(const RangeList &ranges)
{
	beginCompositeAction(i18n("Join Lines"));

	RangeList deleteRanges;

	for(RangeList::ConstIterator rangesIt = ranges.begin(), end = ranges.end(); rangesIt != end; ++rangesIt) {
		int rangeStart = rangesIt->start();
		int rangeEnd = normalizeRangeIndex(rangesIt->end());

		if(rangeStart >= rangeEnd)
			continue;

		SubtitleLine *line = at(rangeStart);
		line->setHideTime(at(rangeEnd)->hideTime());
		Range postLines(rangeStart + 1, rangeEnd);
		for(SubtitleIterator it(*this, postLines); it.current(); ++it) {
			SubtitleLine *ln = it.current();
			if(!ln->primaryDoc()->isEmpty()) {
				QTextCursor *c = line->primaryDoc()->undoableCursor();
				c->movePosition(QTextCursor::End);
				c->insertBlock();
				c->insertFragment(QTextDocumentFragment(ln->primaryDoc()));
			}
			if(!ln->secondaryDoc()->isEmpty()) {
				QTextCursor *c = line->secondaryDoc()->undoableCursor();
				c->movePosition(QTextCursor::End);
				c->insertBlock();
				c->insertFragment(QTextDocumentFragment(ln->secondaryDoc()));
			}
		}
		deleteRanges << postLines;
	}

	removeLines(deleteRanges, Both);

	endCompositeAction();
}

void
Subtitle::shiftAnchoredLine(SubtitleLine *anchoredLine, const Time &newShowTime)
{
	if(m_anchoredLines.indexOf(anchoredLine) == -1 || m_lines.isEmpty())
		return;

	const SubtitleLine *prevAnchor = nullptr;
	const SubtitleLine *nextAnchor = nullptr;
	foreach(auto anchor, m_anchoredLines) {
		if((prevAnchor == nullptr || prevAnchor->m_showTime < anchor->m_showTime) && anchor->m_showTime < anchoredLine->m_showTime)
			prevAnchor = anchor;
		if((nextAnchor == nullptr || nextAnchor->m_showTime > anchor->m_showTime) && anchor->m_showTime > anchoredLine->m_showTime)
			nextAnchor = anchor;
	}
	if((prevAnchor && prevAnchor->m_showTime > newShowTime) || (nextAnchor && nextAnchor->m_showTime < newShowTime))
		return;

	if(!prevAnchor && !nextAnchor) {
		double shift = newShowTime.toMillis() - anchoredLine->m_showTime.toMillis();
		for(int i = 0, n = count(); i < n; i++)
			at(i)->shiftTimes(shift);
	} else {
		// save times as adjustLines() will modify them, and processing nextAnchor will modify them again
		Time savedShowTime(anchoredLine->m_showTime);
		Time savedHideTime(anchoredLine->m_hideTime);
		if(prevAnchor) {
			adjustLines(Range(prevAnchor->index(), anchoredLine->index()), prevAnchor->m_showTime.toMillis(), newShowTime.toMillis());
		} else if(nextAnchor->m_showTime != anchoredLine->m_showTime) {
			const SubtitleLine *first = firstLine();
			double scaleFactor = (nextAnchor->m_showTime.toMillis() - newShowTime.toMillis()) / (nextAnchor->m_showTime.toMillis() - anchoredLine->m_showTime.toMillis());
			Time firstShowTime(scaleFactor * (first->m_showTime.toMillis() - nextAnchor->m_showTime.toMillis()) + nextAnchor->m_showTime.toMillis());
			adjustLines(Range(first->index(), anchoredLine->index()), firstShowTime.toMillis(), newShowTime.toMillis());
		}

		double lastShowTime;
		const SubtitleLine *last;
		if(nextAnchor) {
			last = nextAnchor;
			lastShowTime = nextAnchor->m_showTime.toMillis();
		} else if(anchoredLine->m_showTime != prevAnchor->m_showTime) {
			last = lastLine();
			double scaleFactor = (newShowTime.toMillis() - prevAnchor->m_showTime.toMillis()) / (savedShowTime.toMillis() - prevAnchor->m_showTime.toMillis());
			lastShowTime = scaleFactor * (last->m_showTime.toMillis() - prevAnchor->m_showTime.toMillis()) + prevAnchor->m_showTime.toMillis();
		} else {
			last = nullptr;
			lastShowTime = 0;
		}
		if(newShowTime.toMillis() < lastShowTime && anchoredLine != last) {
			anchoredLine->m_showTime = savedShowTime;
			anchoredLine->m_hideTime = savedHideTime;
			adjustLines(Range(anchoredLine->index(), last->index()), newShowTime.toMillis(), lastShowTime);
		}
	}
}

void
Subtitle::shiftLines(const RangeList &ranges, long msecs)
{
	if(msecs == 0)
		return;

	beginCompositeAction(i18n("Shift Lines"));

	if(!m_anchoredLines.empty()) {
		for(SubtitleIterator it(*this, ranges); it.current(); ++it) {
			SubtitleLine *line = it.current();
			if(m_anchoredLines.indexOf(line) != -1) {
				shiftAnchoredLine(line, line->showTime().shifted(msecs));
				break;
			}
		}
	} else {
		for(SubtitleIterator it(*this, ranges); it.current(); ++it)
			it.current()->shiftTimes(msecs);
	}

	endCompositeAction();
}

void
Subtitle::adjustLines(const Range &range, long newFirstTime, long newLastTime)
{
	if(m_lines.isEmpty() || newFirstTime >= newLastTime)
		return;

	int firstIndex = range.start();
	int lastIndex = normalizeRangeIndex(range.end());

	if(firstIndex >= lastIndex)
		return;

	double oldFirstTime = at(firstIndex)->showTime().toMillis();
	double oldLastTime = at(lastIndex)->showTime().toMillis();
	double oldDeltaTime = oldLastTime - oldFirstTime;

	double newDeltaTime = newLastTime - newFirstTime;

	// special case in which we can't proceed as there's no way to
	// linearly transform the same time into two different ones...
	if(!oldDeltaTime && newDeltaTime)
		return;

	double shiftMseconds;
	double scaleFactor;

	if(oldDeltaTime) {
		shiftMseconds = newFirstTime - (newDeltaTime / oldDeltaTime) * oldFirstTime;
		scaleFactor = newDeltaTime / oldDeltaTime;
	} else {                                        // oldDeltaTime == 0 && newDeltaTime == 0
		// in this particular case we can make the adjust transformation act as a plain shift
		shiftMseconds = newFirstTime - oldFirstTime;
		scaleFactor = 1.0;
	}

	if(shiftMseconds == 0 && scaleFactor == 1.0)
		return;

	beginCompositeAction(i18n("Adjust Lines"));

	for(SubtitleIterator it(*this, range); it.current(); ++it)
		it.current()->adjustTimes(shiftMseconds, scaleFactor);

	endCompositeAction();
}

void
Subtitle::sortLines(const Range &range)
{
	beginCompositeAction(i18n("Sort"));

	SubtitleIterator it(*this, range);
	SubtitleLine *line = it.current();
	SubtitleLine *nextLine = (++it).current();
	for(; nextLine; ++it, line = nextLine, nextLine = it.current()) {
//		qDebug() << "sort: test" << nextLine->index();
		if(line->showTime() <= nextLine->showTime()) // already sorted
			continue;

		SubtitleIterator tmp(it);
		int fromIndex = tmp.index();
		int toIndex = -1;
		while((--tmp).current() && tmp.current()->showTime() > nextLine->showTime())
			toIndex = tmp.index();

		Q_ASSERT(toIndex != -1);

//		qDebug() << "sort: move" << fromIndex << "to" << toIndex;
		processAction(new MoveLineAction(this, fromIndex, toIndex));

		--it;
	}

	endCompositeAction();
}

void
Subtitle::applyDurationLimits(const RangeList &ranges, const Time &minDuration, const Time &maxDuration, bool canOverlap)
{
	if(m_lines.isEmpty() || minDuration > maxDuration)
		return;

	beginCompositeAction(i18n("Enforce Duration Limits"));

	for(RangeList::ConstIterator rangesIt = ranges.begin(), end = ranges.end(); rangesIt != end; ++rangesIt) {
		Time lineDuration;
		SubtitleIterator it(*this, *rangesIt);
		SubtitleLine *line = it.current();
		++it;
		SubtitleLine *nextLine = it.current();

		for(; line; ++it, line = nextLine, nextLine = it.current()) {
			lineDuration = line->durationTime();

			if(lineDuration > maxDuration)
				line->setDurationTime(maxDuration);
			else if(lineDuration < minDuration) {
				if(!nextLine) // the last line doesn't have risk of overlapping
					line->setDurationTime(minDuration);
				else {
					if(canOverlap || line->showTime() + minDuration < nextLine->showTime())
						line->setDurationTime(minDuration);
					else {          // setting the duration to minDuration will cause an unwanted overlap
						if(line->hideTime() < nextLine->showTime()) // make duration as big as possible without overlap
							line->setHideTime(nextLine->showTime() - 1);
						// else line is already at the maximum duration without overlap (or overlapping) so we don't change it
					}
				}
			}
		}
	}

	endCompositeAction();
}

void
Subtitle::setMaximumDurations(const RangeList &ranges)
{
	if(m_lines.isEmpty())
		return;

	beginCompositeAction(i18n("Maximize Durations"));

	for(RangeList::ConstIterator rangesIt = ranges.begin(), end = ranges.end(); rangesIt != end; ++rangesIt) {
		SubtitleIterator it(*this, *rangesIt);
		SubtitleLine *line = it.current();
		++it;
		SubtitleLine *nextLine = it.current();

		for(; line && nextLine; ++it, line = nextLine, nextLine = it.current()) {
			if(line->hideTime() < nextLine->showTime())
				line->setHideTime(nextLine->showTime() - 1);
		}
	}

	endCompositeAction();
}

void
Subtitle::setAutoDurations(const RangeList &ranges, int msecsPerChar, int msecsPerWord, int msecsPerLine, bool canOverlap, SubtitleTarget calculationTarget)
{
	if(m_lines.isEmpty())
		return;

	beginCompositeAction(i18n("Set Automatic Durations"));

	for(RangeList::ConstIterator rangesIt = ranges.begin(), end = ranges.end(); rangesIt != end; ++rangesIt) {
		Time autoDuration;

		SubtitleIterator it(*this, *rangesIt);
		SubtitleLine *line = it.current();
		++it;
		SubtitleLine *nextLine = it.current();

		for(; line; ++it, line = nextLine, nextLine = it.current()) {
			autoDuration = line->autoDuration(msecsPerChar, msecsPerWord, msecsPerLine, calculationTarget);

			if(!nextLine) // the last line doesn't have risk of overlapping
				line->setDurationTime(autoDuration);
			else {
				if(canOverlap || line->showTime() + autoDuration < nextLine->showTime())
					line->setDurationTime(autoDuration);
				else // setting the duration to autoDuration will cause an unwanted overlap
					line->setHideTime(nextLine->showTime() - 1);
			}
		}
	}

	endCompositeAction();
}

void
Subtitle::fixOverlappingLines(const RangeList &ranges, const Time &minInterval)
{
	if(m_lines.isEmpty())
		return;

	beginCompositeAction(i18n("Fix Overlapping Times"));

	for(RangeList::ConstIterator rangesIt = ranges.begin(), end = ranges.end(); rangesIt != end; ++rangesIt) {
		int rangeStart = rangesIt->start();
		int rangeEnd = normalizeRangeIndex(rangesIt->end() + 1);

		if(rangeStart >= rangeEnd)
			break;

		SubtitleIterator it(*this, Range(rangeStart, rangeEnd));
		SubtitleLine *line = it.current();
		++it;
		SubtitleLine *nextLine = it.current();

		for(; nextLine; ++it, line = nextLine, nextLine = it.current()) {
			if(line->hideTime() + minInterval >= nextLine->showTime()) {
				Time newHideTime = nextLine->showTime() - minInterval;
				line->setHideTime(newHideTime >= line->showTime() ? newHideTime : line->showTime());
			}
		}
	}

	endCompositeAction();
}

void
Subtitle::fixPunctuation(const RangeList &ranges, bool spaces, bool quotes, bool engI, bool ellipsis, SubtitleTarget target)
{
	if(m_lines.isEmpty() || (!spaces && !quotes && !engI && !ellipsis) || target >= SubtitleTargetSize)
		return;

	beginCompositeAction(i18n("Fix Lines Punctuation"));

	for(RangeList::ConstIterator rangesIt = ranges.begin(), end = ranges.end(); rangesIt != end; ++rangesIt) {
		SubtitleIterator it(*this, *rangesIt);

		bool primaryContinues = false;
		bool secondaryContinues = false;

		if(it.index() > 0) {
			if(target == Primary || target == Both) // init primaryContinues
				at(it.index() - 1)->primaryDoc()->fixPunctuation(spaces, quotes, engI, ellipsis, &primaryContinues, true);
			if(target == Secondary || target == Both) // init secondaryContinues
				at(it.index() - 1)->secondaryDoc()->fixPunctuation(spaces, quotes, engI, ellipsis, &secondaryContinues, true);
		}

		for(; it.current(); ++it) {
			switch(target) {
			case Primary:
				it.current()->primaryDoc()->fixPunctuation(spaces, quotes, engI, ellipsis, &primaryContinues);
				break;
			case Secondary:
				it.current()->secondaryDoc()->fixPunctuation(spaces, quotes, engI, ellipsis, &secondaryContinues);
				break;
			case Both:
				it.current()->primaryDoc()->fixPunctuation(spaces, quotes, engI, ellipsis, &primaryContinues);
				it.current()->secondaryDoc()->fixPunctuation(spaces, quotes, engI, ellipsis, &secondaryContinues);
				break;
			default:
				break;
			}
		}
	}

	endCompositeAction();
}

void
Subtitle::lowerCase(const RangeList &ranges, SubtitleTarget target)
{
	if(m_lines.isEmpty() || target >= SubtitleTargetSize)
		return;

	beginCompositeAction(i18n("Lower Case"));

	switch(target) {
	case Primary:
		for(SubtitleIterator it(*this, ranges); it.current(); ++it)
			it.current()->primaryDoc()->toLower();
		break;
	case Secondary:
		for(SubtitleIterator it(*this, ranges); it.current(); ++it)
			it.current()->secondaryDoc()->toLower();
		break;
	case Both:
		for(SubtitleIterator it(*this, ranges); it.current(); ++it) {
			it.current()->primaryDoc()->toLower();
			it.current()->secondaryDoc()->toLower();
		}
		break;
	default:
		break;
	}

	endCompositeAction();
}

void
Subtitle::upperCase(const RangeList &ranges, SubtitleTarget target)
{
	if(m_lines.isEmpty() || target >= SubtitleTargetSize)
		return;

	beginCompositeAction(i18n("Upper Case"));

	switch(target) {
	case Primary:
		for(SubtitleIterator it(*this, ranges); it.current(); ++it)
			it.current()->primaryDoc()->toUpper();
		break;
	case Secondary:
		for(SubtitleIterator it(*this, ranges); it.current(); ++it)
			it.current()->secondaryDoc()->toUpper();
		break;
	case Both:
		for(SubtitleIterator it(*this, ranges); it.current(); ++it) {
			it.current()->primaryDoc()->toUpper();
			it.current()->secondaryDoc()->toUpper();
		}
		break;
	default:
		break;
	}

	endCompositeAction();
}

void
Subtitle::titleCase(const RangeList &ranges, bool lowerFirst, SubtitleTarget target)
{
	if(m_lines.isEmpty() || target >= SubtitleTargetSize)
		return;

	beginCompositeAction(i18n("Title Case"));

	switch(target) {
	case Primary: {
		for(SubtitleIterator it(*this, ranges); it.current(); ++it)
			it.current()->primaryDoc()->toSentenceCase(nullptr, lowerFirst, true);
		break;
	}
	case Secondary: {
		for(SubtitleIterator it(*this, ranges); it.current(); ++it)
			it.current()->secondaryDoc()->toSentenceCase(nullptr, lowerFirst, true);
		break;
	}
	case Both: {
		for(SubtitleIterator it(*this, ranges); it.current(); ++it) {
			it.current()->primaryDoc()->toSentenceCase(nullptr, lowerFirst, true);
			it.current()->secondaryDoc()->toSentenceCase(nullptr, lowerFirst, true);
		}
		break;
	}
	default:
		break;
	}

	endCompositeAction();
}

void
Subtitle::sentenceCase(const RangeList &ranges, bool lowerFirst, SubtitleTarget target)
{
	if(m_lines.isEmpty() || target >= SubtitleTargetSize)
		return;

	beginCompositeAction(i18n("Sentence Case"));

	for(RangeList::ConstIterator rangesIt = ranges.begin(), rangesEnd = ranges.end(); rangesIt != rangesEnd; ++rangesIt) {
		SubtitleIterator it(*this, *rangesIt);

		bool pCont = false;
		bool sCont = false;

		if(it.index() > 0) {
			if(target == Primary || target == Both)
				at(it.index() - 1)->primaryDoc()->toSentenceCase(&pCont, lowerFirst, false, true);
			if(target == Secondary || target == Both)
				at(it.index() - 1)->secondaryDoc()->toSentenceCase(&sCont, lowerFirst, false, true);
		}

		switch(target) {
		case Primary: {
			for(; it.current(); ++it)
				it.current()->primaryDoc()->toSentenceCase(&pCont, lowerFirst);
			break;
		}
		case Secondary: {
			for(; it.current(); ++it)
				it.current()->secondaryDoc()->toSentenceCase(&sCont, lowerFirst);
			break;
		}
		case Both: {
			for(; it.current(); ++it) {
				it.current()->primaryDoc()->toSentenceCase(&pCont, lowerFirst);
				it.current()->secondaryDoc()->toSentenceCase(&sCont, lowerFirst);
			}
			break;
		}
		default:
			break;
		}
	}

	endCompositeAction();
}

void
Subtitle::breakLines(const RangeList &ranges, unsigned minLengthForLineBreak, SubtitleTarget target)
{
	SubtitleCompositeActionExecutor executor(this, i18n("Break Lines"));

	for(SubtitleIterator it(*this, ranges); it.current(); ++it)
		it.current()->breakText(minLengthForLineBreak, target);
}

void
Subtitle::unbreakTexts(const RangeList &ranges, SubtitleTarget target)
{
	SubtitleCompositeActionExecutor executor(this, i18n("Unbreak Lines"));

	for(SubtitleIterator it(*this, ranges); it.current(); ++it)
		it.current()->unbreakText(target);
}

void
Subtitle::simplifyTextWhiteSpace(const RangeList &ranges, SubtitleTarget target)
{
	SubtitleCompositeActionExecutor executor(this, i18n("Simplify Spaces"));

	for(SubtitleIterator it(*this, ranges); it.current(); ++it)
		it.current()->simplifyTextWhiteSpace(target);
}

void
Subtitle::syncWithSubtitle(const Subtitle &refSubtitle)
{
	beginCompositeAction(i18n("Synchronize Subtitles"));

	for(SubtitleIterator it(*this, Range::full()), refIt(refSubtitle, Range::full()); it.current() && refIt.current(); ++it, ++refIt)
		processAction(new SetLineTimesAction(it.current(), refIt.current()->showTime(), refIt.current()->hideTime()));

	sortLines(Range::full());

	endCompositeAction();
}

void
Subtitle::appendSubtitle(const Subtitle &srcSubtitle, double shiftMsecsBeforeAppend)
{
	if(!srcSubtitle.count())
		return;

	QList<SubtitleLine *> lines;
	for(SubtitleIterator it(srcSubtitle); it.current(); ++it) {
		SubtitleLine *ln = it.current();
		SubtitleLine *newLine = new SubtitleLine(ln->showTime() + shiftMsecsBeforeAppend, ln->hideTime() + shiftMsecsBeforeAppend);
		newLine->primaryDoc()->setDocument(ln->primaryDoc());
		newLine->secondaryDoc()->setDocument(ln->secondaryDoc());
		lines.append(newLine);
	}

	beginCompositeAction(i18n("Join Subtitles"));

	processAction(new InsertLinesAction(this, lines));

	endCompositeAction();
}

void
Subtitle::splitSubtitle(Subtitle &dstSubtitle, const Time &splitTime, bool shiftSplitLines)
{
	if(!m_lines.count())
		return;

	int splitIndex = -1; // the index of the first line to move (or copy) to dstSub
	bool splitsLine = false; // splitTime falls in within a line's time
	const double shiftTime = shiftSplitLines ? -splitTime.toMillis() : 0.;
	const double dstSplitTime = splitTime.toMillis() + shiftTime;

	QList<SubtitleLine *> lines;
	for(SubtitleIterator it(*this, Range::full()); it.current(); ++it) {
		if(splitTime <= it.current()->hideTime()) {
			SubtitleLine *ln = it.current();
			double newShowTime = ln->showTime().toMillis() + shiftTime;

			if(splitIndex < 0) { // first line of the new subtitle
				splitIndex = it.index();
				splitsLine = dstSplitTime > newShowTime;
				if(splitsLine)
					newShowTime = dstSplitTime;
			}

			SubtitleLine *newLine = new SubtitleLine(newShowTime, ln->hideTime() + shiftTime);
			newLine->primaryDoc()->setDocument(ln->primaryDoc());
			newLine->secondaryDoc()->setDocument(ln->secondaryDoc());
			if(ln->m_formatData)
				newLine->m_formatData = new FormatData(*ln->m_formatData);

			lines.append(newLine);
		}
	}

	if(splitIndex > 0 || (splitIndex == 0 && splitsLine)) {
		dstSubtitle.m_formatData = m_formatData ? new FormatData(*m_formatData) : 0;

		dstSubtitle.beginCompositeAction(i18n("Split Subtitles"));
		if(dstSubtitle.count())
			dstSubtitle.processAction(new RemoveLinesAction(&dstSubtitle, 0, -1));
		dstSubtitle.processAction(new InsertLinesAction(&dstSubtitle, lines, 0));
		dstSubtitle.endCompositeAction();

		beginCompositeAction(i18n("Split Subtitles"));
		if(splitsLine) {
			at(splitIndex)->setHideTime(splitTime);
			splitIndex++;
			if(splitIndex < count())
				processAction(new RemoveLinesAction(this, splitIndex));
		} else
			processAction(new RemoveLinesAction(this, splitIndex));
		endCompositeAction();
	}
}

void
Subtitle::toggleStyleFlag(const RangeList &ranges, SString::StyleFlag styleFlag)
{
	SubtitleIterator it(*this, ranges);
	if(!it.current())
		return;

	beginCompositeAction(i18n("Toggle Lines Style"));

	QTextCharFormat fmtPri, fmtSec;
	switch(styleFlag) {
	case SString::Bold:
		fmtPri.setFontWeight(QTextCursor(it.current()->primaryDoc()).charFormat().fontWeight() == QFont::Bold ? QFont::Normal : QFont::Bold);
		fmtSec.setFontWeight(QTextCursor(it.current()->secondaryDoc()).charFormat().fontWeight() == QFont::Bold ? QFont::Normal : QFont::Bold);
		break;
	case SString::Italic:
		fmtPri.setFontItalic(!QTextCursor(it.current()->primaryDoc()).charFormat().fontItalic());
		fmtSec.setFontItalic(!QTextCursor(it.current()->secondaryDoc()).charFormat().fontItalic());
		break;
	case SString::Underline:
		fmtPri.setFontUnderline(!QTextCursor(it.current()->primaryDoc()).charFormat().fontUnderline());
		fmtSec.setFontUnderline(!QTextCursor(it.current()->secondaryDoc()).charFormat().fontUnderline());
		break;
	case SString::StrikeThrough:
		fmtPri.setFontStrikeOut(!QTextCursor(it.current()->primaryDoc()).charFormat().fontStrikeOut());
		fmtSec.setFontStrikeOut(!QTextCursor(it.current()->secondaryDoc()).charFormat().fontStrikeOut());
		break;
	default:
		Q_ASSERT_X(false, "Subtitle::toggleStyleFlag", "Unsupported format");
		break;
	}

	for(; it.current(); ++it) {
		QTextCursor cp(it.current()->primaryDoc());
		cp.select(QTextCursor::Document);
		cp.mergeCharFormat(fmtPri);
		QTextCursor cs(it.current()->secondaryDoc());
		cs.select(QTextCursor::Document);
		cs.mergeCharFormat(fmtPri);
	}

	endCompositeAction();
}

void
Subtitle::changeTextColor(const RangeList &ranges, QRgb color)
{
	SubtitleIterator it(*this, ranges);
	if(!it.current())
		return;

	beginCompositeAction(i18n("Change Lines Text Color"));

	QTextCharFormat fmt;
	fmt.setForeground(color ? QBrush(QColor(color)) : QBrush());

	for(; it.current(); ++it) {
		QTextCursor cp(it.current()->primaryDoc());
		cp.select(QTextCursor::Document);
		cp.mergeCharFormat(fmt);
		QTextCursor cs(it.current()->secondaryDoc());
		cs.select(QTextCursor::Document);
		cs.mergeCharFormat(fmt);
	}

	endCompositeAction();
}

void
Subtitle::setMarked(const RangeList &ranges, bool value)
{
	beginCompositeAction(value ? i18n("Set Lines Mark") : i18n("Clear Lines Mark"));

	for(SubtitleIterator it(*this, ranges); it.current(); ++it)
		it.current()->setErrorFlags(SubtitleLine::UserMark, value);

	endCompositeAction();
}

void
Subtitle::toggleMarked(const RangeList &ranges)
{
	SubtitleIterator it(*this, ranges);
	if(!it.current())
		return;

	beginCompositeAction(i18n("Toggle Lines Mark"));

	setMarked(ranges, !(it.current()->errorFlags() & SubtitleLine::UserMark));

	endCompositeAction();
}

void
Subtitle::clearErrors(const RangeList &ranges, int errorFlags)
{
	beginCompositeAction(i18n("Clear Line Errors"));

	for(SubtitleIterator it(*this, ranges); it.current(); ++it)
		it.current()->setErrorFlags(errorFlags, false);

	endCompositeAction();
}

void
Subtitle::checkErrors(const RangeList &ranges, int errorFlags)
{
	beginCompositeAction(i18n("Check Lines Errors"));

	for(SubtitleIterator it(*this, ranges); it.current(); ++it)
		it.current()->check(errorFlags);

	endCompositeAction();
}

void
Subtitle::recheckErrors(const RangeList &ranges)
{
	beginCompositeAction(i18n("Check Lines Errors"));

	for(SubtitleIterator it(*this, ranges); it.current(); ++it)
		it.current()->check(it.current()->errorFlags());

	endCompositeAction();
}

void
Subtitle::processAction(UndoAction *action) const
{
	if(app()->subtitle() == this)
		app()->undoStack()->push(action);
	else
		action->redo();
}

void
Subtitle::beginCompositeAction(const QString &title) const
{
	if(app()->subtitle() == this)
		app()->undoStack()->beginMacro(title);
}

void
Subtitle::endCompositeAction(UndoStack::DirtyMode dirtyOverride) const
{
	if(app()->subtitle() == this)
		app()->undoStack()->endMacro(dirtyOverride);
}

bool
Subtitle::isPrimaryDirty(int index) const
{
	const UndoStack *undoStack = app()->undoStack();

	int i = m_primaryCleanIndex;
	const int d = i > index ? -1 : 1;
	for(;;) {
		if(i < 0)
			return m_primaryCleanIndex >= 0;
		const UndoStack::DirtyMode dirtyMode = i > 0 ? undoStack->dirtyMode(i - 1) : UndoStack::None;
		if(i != m_primaryCleanIndex && (dirtyMode & UndoStack::Primary))
			return true;
		if(i == index)
			return false;
		i += d;
	}
}

bool
Subtitle::isSecondaryDirty(int index) const
{
	const UndoStack *undoStack = app()->undoStack();

	int i = m_secondaryCleanIndex;
	const int d = i > index ? -1 : 1;
	for(;;) {
		if(i < 0)
			return m_secondaryCleanIndex >= 0;
		const UndoStack::DirtyMode dirtyMode = i > 0 ? undoStack->dirtyMode(i - 1) : UndoStack::None;
		if(i != m_secondaryCleanIndex && (dirtyMode & UndoStack::Secondary))
			return true;
		if(i == index)
			return false;
		i += d;
	}
}

void
Subtitle::updateState()
{
	const UndoStack *undoStack = app()->undoStack();
	const int index = undoStack->index();
	const UndoStack::DirtyMode dirtyMode = index > 0 ? undoStack->dirtyMode(index - 1) : UndoStack::Both;

	if(m_primaryDirtyState != isPrimaryDirty(index)) {
		m_primaryDirtyState = !m_primaryDirtyState;
		emit primaryDirtyStateChanged(m_primaryDirtyState);
	}
	if(dirtyMode & UndoStack::Primary)
		emit primaryChanged();

	if(m_secondaryDirtyState != isSecondaryDirty(index)) {
		m_secondaryDirtyState = !m_secondaryDirtyState;
		emit secondaryDirtyStateChanged(m_secondaryDirtyState);
	}
	if(dirtyMode & UndoStack::Secondary)
		emit secondaryChanged();
}

/// SUBTITLECOMPOSITEACTIONEXECUTOR

SubtitleCompositeActionExecutor::SubtitleCompositeActionExecutor(const Subtitle *subtitle, const QString &title)
	: m_subtitle(subtitle)
{
	m_subtitle->beginCompositeAction(title);
}

SubtitleCompositeActionExecutor::~SubtitleCompositeActionExecutor()
{
	m_subtitle->endCompositeAction();
}
