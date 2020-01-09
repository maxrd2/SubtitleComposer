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

#include "core/subtitle.h"
#include "core/subtitleline.h"
#include "core/subtitleiterator.h"
#include "core/subtitleactions.h"
#include "core/subtitlelineactions.h"
#include "helpers/objectref.h"
#include "scconfig.h"
#include "application.h"
#include "gui/lineswidget.h"

#include <KLocalizedString>
#include <QUndoStack>

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
	: m_primaryState(0),
	  m_primaryCleanState(0),
	  m_secondaryState(0),
	  m_secondaryCleanState(0),
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
		thisLine->setPrimaryText(usePrimaryData ? fromLine->primaryText() : fromLine->secondaryText());
		thisLine->setTimes(fromLine->showTime(), fromLine->hideTime());
		thisLine->setErrorFlags((fromLine->errorFlags() & fromErrors) | (thisLine->errorFlags() & thisErrors));
		thisLine->setFormatData(fromLine->formatData());
	}

	if(fromIt.current()) {          // 'from' had more lines than '*this'
		QList<SubtitleLine *> lines;
		for(; fromIt.current(); ++fromIt) {
			SubtitleLine *thisLine = new SubtitleLine(*fromIt.current());
			if(!usePrimaryData)
				thisLine->setPrimaryText(thisLine->secondaryText());
			thisLine->setSecondaryText(SString());
			thisLine->setErrorFlags(SubtitleLine::SecondaryOnlyErrors, false);
			thisLine->setFormatData(fromIt.current()->formatData());
			lines.append(thisLine);
		}
		processAction(new InsertLinesAction(*this, lines));
	} else if(thisIt.current()) {   // '*this'  had more lines than 'from'
		for(SubtitleLine *thisLine = thisIt.current(); thisLine; ++thisIt, thisLine = thisIt.current()) {
			thisLine->setPrimaryText(SString());
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
		it.current()->setPrimaryText(SString());
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

	for(int i = 0, n = m_lines.size(); i < n; i++) {
		const SubtitleLine *srcLine = from.m_lines.at(i).obj();
		SubtitleLine *dstLine = m_lines.at(i).obj();
		dstLine->setSecondaryText(usePrimaryData ? srcLine->primaryText() : srcLine->secondaryText());
		dstLine->setErrorFlags((dstLine->errorFlags() & dstErrors) | (srcLine->errorFlags() & srcErrors));
	}

	// clear remaining local translations
	for(int i = from.m_lines.size(), n = m_lines.size(); i < n; i++) {
		SubtitleLine *dstLine = m_lines.at(i).obj();
		dstLine->setSecondaryText(SString());
		dstLine->setErrorFlags(SubtitleLine::SecondaryOnlyErrors, false);
	}

	// insert remaining source translations
	QList<SubtitleLine *> newLines;
	for(int i = m_lines.size(), n = from.m_lines.size(); i < n; i++) {
		const SubtitleLine *srcLine = from.m_lines.at(i).obj();
		SubtitleLine *dstLine = new SubtitleLine(*srcLine);
		if(usePrimaryData)
			dstLine->setSecondaryText(dstLine->primaryText());
		dstLine->setPrimaryText(SString());
		dstLine->setErrorFlags(SubtitleLine::PrimaryOnlyErrors, false);
		newLines.append(dstLine);
	}
	if(!newLines.isEmpty())
		processAction(new InsertLinesAction(*this, newLines));

	endCompositeAction();
}

void
Subtitle::clearSecondaryTextData()
{
	beginCompositeAction(i18n("Clear Secondary Text Data"));

	for(SubtitleIterator it(*this); it.current(); ++it) {
		it.current()->setSecondaryText(SString());
		it.current()->setErrorFlags(SubtitleLine::SecondaryOnlyErrors, false);
	}

	endCompositeAction();
}

bool
Subtitle::isPrimaryDirty() const
{
	return m_primaryState != m_primaryCleanState;
}

void
Subtitle::clearPrimaryDirty()
{
	if(m_primaryCleanState != m_primaryState) {
		m_primaryCleanState = m_primaryState;
		emit primaryDirtyStateChanged(false);
	}
}

bool
Subtitle::isSecondaryDirty() const
{
	return m_secondaryState != m_secondaryCleanState;
}

void
Subtitle::clearSecondaryDirty()
{
	if(m_secondaryCleanState != m_secondaryState) {
		m_secondaryCleanState = m_secondaryState;
		emit secondaryDirtyStateChanged(false);
	}
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
		processAction(new SetFramesPerSecondAction(*this, framesPerSecond));
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
			showTime.adjust(0, scaleFactor);

			Time hideTime = it.current()->hideTime();
			hideTime.adjust(0, scaleFactor);

			processAction(new SetLineTimesAction(*it, showTime, hideTime));
		}
	}

	endCompositeAction();
}

bool
Subtitle::isEmpty() const
{
	return m_lines.empty();
}

int
Subtitle::linesCount() const
{
	return m_lines.count();
}

int
Subtitle::lastIndex() const
{
	return m_lines.count() - 1;
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

SubtitleLine *
Subtitle::firstLine()
{
	return m_lines.isEmpty() ? 0 : m_lines.first().obj();
}

const SubtitleLine *
Subtitle::firstLine() const
{
	return m_lines.isEmpty() ? 0 : m_lines.first().obj();
}

SubtitleLine *
Subtitle::lastLine()
{
	return m_lines.isEmpty() ? 0 : m_lines.last().obj();
}

const SubtitleLine *
Subtitle::lastLine() const
{
	return m_lines.isEmpty() ? 0 : m_lines.last().obj();
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
	QList<const SubtitleLine *> anchoredLines = m_anchoredLines;

	m_anchoredLines.clear();

	foreach(auto line, anchoredLines)
		emit lineAnchorChanged(line, false);
}

void
Subtitle::insertLine(SubtitleLine *line, int index)
{
	QList<SubtitleLine *> lines;
	lines.append(line);
	insertLines(lines, index);
}

void
Subtitle::insertLines(const QList<SubtitleLine *> &lines, int index)
{
	Q_ASSERT(index <= m_lines.count());

	if(index < 0)
		index = m_lines.count();

	processAction(new InsertLinesAction(*this, lines, index));
}

SubtitleLine *
Subtitle::insertNewLine(int index, bool insertAfter, TextTarget target)
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
			line->setSecondaryText(it.current()->secondaryText());
			line = it.current();
		}
		line->setSecondaryText(SString());

		endCompositeAction();
	} else if(target == Secondary) {
		beginCompositeAction(i18n("Insert Line"));

		insertLine(newLine, newLineIndex);

		SubtitleIterator it(*this, Range::full(), true);
		SubtitleLine *line = it.current();
		for(--it; it.index() >= index; --it) {
			line->setSecondaryText(it.current()->secondaryText());
			line = it.current();
		}
		line->setSecondaryText(SString());

		newLine = line;

		endCompositeAction();
	}

	return newLine;
}

void
Subtitle::removeLines(const RangeList &r, TextTarget target)
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
			processAction(new RemoveLinesAction(*this, (*rangesIt).start(), (*rangesIt).end()));
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
			dstIt.current()->setSecondaryText(srcIt.current()->secondaryText());

		// the remaining lines secondary text must be cleared
		for(; dstIt.current(); ++dstIt)
			dstIt.current()->setSecondaryText(SString());

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
			lines.append(new SubtitleLine(SString(), SString(), showTime, hideTime));
			showTime.shift(1100.);
			hideTime.shift(1100.);
		}

		processAction(new InsertLinesAction(*this, lines));

		// then, we move the secondary texts down (we need to iterate from bottom to top for that)
		RangeList rangesComplement = mutableRanges.complement();

		SubtitleIterator srcIt(*this, Range(ranges.firstIndex(), m_lines.count() - lines.count() - 1), true);
		SubtitleIterator dstIt(*this, rangesComplement, true);
		for(; srcIt.current() && dstIt.current(); --srcIt, --dstIt)
			dstIt.current()->setSecondaryText(srcIt.current()->secondaryText());

		// finally, we can remove the specified lines
		RangeList::ConstIterator rangesIt = ranges.end(), begin = ranges.begin();
		do {
			rangesIt--;
			processAction(new RemoveLinesAction(*this, (*rangesIt).start(), (*rangesIt).end()));
		} while(rangesIt != begin);

		endCompositeAction();
	}
}

void
Subtitle::swapTexts(const RangeList &ranges)
{
	processAction(new SwapLinesTextsAction(*this, ranges));
}

void
Subtitle::splitLines(const RangeList &ranges)
{
	auto splitOnSpace = [&](SString &text){
		int len = text.length();
		int i = len / 2;
		int j = i + len % 2;
		for(; ; i--, j++) {
			if(text[i] == QChar::Space) {
				text[i] = QChar::LineFeed;
				break;
			}
			Q_ASSERT(j <= len);
			if(text[j] == QChar::Space) {
				text[j] = QChar::LineFeed;
				break;
			}
			if(i == 0) {
				text.append(QChar::LineFeed);
				break;
			}
		}
	};

	beginCompositeAction(i18n("Split Lines"));

	bool hasMultipleLines = false;
	for(SubtitleIterator it(*this, ranges, true); it.current(); --it) {
		SubtitleLine *line = it.current();

		line->simplifyTextWhiteSpace(SubtitleLine::Both);

		if(line->primaryText().count(QChar::LineFeed) || line->secondaryText().count(QChar::LineFeed)) {
			hasMultipleLines = true;
			break;
		}
	}

	for(SubtitleIterator it(*this, ranges, true); it.current(); --it) {
		SubtitleLine *line = it.current();

		SString primaryText = line->primaryText();
		if(primaryText.isEmpty())
			continue;
		SString secondaryText = line->secondaryText();

		if(!hasMultipleLines) {
			if(primaryText.count(QChar::Space) == 0)
				continue;
			splitOnSpace(primaryText);
			splitOnSpace(secondaryText);
		}

		SStringList primaryLines = primaryText.split(QChar::LineFeed);
		SStringList secondaryLines = secondaryText.split(QChar::LineFeed);

		double autoDurationsSum = 0;
		QList<double> autoDurations;
		for(SStringList::ConstIterator ptIt = primaryLines.constBegin(), ptEnd = primaryLines.constEnd(); ptIt != ptEnd; ++ptIt) {
			const Time &autoDuration = SubtitleLine::autoDuration(ptIt->string(), 60, 50, 50);
			autoDurations.append(autoDuration.toMillis());
			autoDurationsSum += autoDuration.toMillis();
		}

		double autoDurationFactor = (line->durationTime().toMillis() + 1.) / autoDurationsSum;

		while(secondaryLines.count() < primaryLines.count())
			secondaryLines.append(SString());
		while(secondaryLines.count() > primaryLines.count())
			primaryLines.append(SString());

		int subLineIndex = it.index(),
				splitLineIndex = 0;
		SStringList::ConstIterator ptIt = primaryLines.constBegin(),
                                ptEnd = primaryLines.constEnd(),
                                stIt = secondaryLines.constBegin(),
                                stEnd = secondaryLines.constEnd();
		for(; ptIt != ptEnd && stIt != stEnd; ++ptIt, ++stIt, ++subLineIndex, ++splitLineIndex) {
			if(splitLineIndex) {
				SubtitleLine *newLine = new SubtitleLine();
				newLine->setShowTime(line->hideTime() + 1.);
				insertLine(newLine, subLineIndex);
				line = newLine;
			}

			line->setTexts(*ptIt, *stIt);
			if(primaryLines.count() > 1)
				line->setDurationTime(Time(autoDurationFactor * autoDurations[splitLineIndex] - 1.));
		}
	}

	endCompositeAction();
}

void
Subtitle::joinLines(const RangeList &ranges)
{
	beginCompositeAction(i18n("Join Lines"));

	RangeList obsoletedRanges;

	for(RangeList::ConstIterator rangesIt = ranges.begin(), end = ranges.end(); rangesIt != end; ++rangesIt) {
		int rangeStart = (*rangesIt).start();
		int rangeEnd = normalizeRangeIndex((*rangesIt).end());

		if(rangeStart >= rangeEnd)
			continue;

		SubtitleLine *firstLine = at(rangeStart);
		SubtitleLine *lastLine = at(rangeEnd);

		SString primaryText, secondaryText;

		for(SubtitleIterator it(*this, Range(rangeStart, rangeEnd - 1)); it.current(); ++it) {
			if(!it.current()->primaryText().isEmpty())
				primaryText.append(it.current()->primaryText()).append(QChar::LineFeed);

			if(!it.current()->secondaryText().isEmpty())
				secondaryText.append(it.current()->secondaryText()).append(QChar::LineFeed);
		}

		primaryText.append(lastLine->primaryText());
		secondaryText.append(lastLine->secondaryText());

		firstLine->setTexts(primaryText, secondaryText);
		firstLine->setHideTime(lastLine->hideTime());

		obsoletedRanges << Range(rangeStart + 1, rangeEnd);
	}

	removeLines(obsoletedRanges, Both);

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
		processAction(new MoveLineAction(*this, fromIndex, toIndex));

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
Subtitle::setAutoDurations(const RangeList &ranges, int msecsPerChar, int msecsPerWord, int msecsPerLine, bool canOverlap, TextTarget calculationTarget)
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
			autoDuration = line->autoDuration(msecsPerChar, msecsPerWord, msecsPerLine, (SubtitleLine::TextTarget)
											  calculationTarget);

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
		int rangeStart = (*rangesIt).start();
		int rangeEnd = normalizeRangeIndex((*rangesIt).end() + 1);

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
Subtitle::fixPunctuation(const RangeList &ranges, bool spaces, bool quotes, bool engI, bool ellipsis, TextTarget target)
{
	if(m_lines.isEmpty() || (!spaces && !quotes && !engI && !ellipsis)
	   || target >= TextTargetSIZE)
		return;

	beginCompositeAction(i18n("Fix Lines Punctuation"));

	for(RangeList::ConstIterator rangesIt = ranges.begin(), end = ranges.end(); rangesIt != end; ++rangesIt) {
		SubtitleIterator it(*this, *rangesIt);

		bool primaryContinues = false;
		bool secondaryContinues = false;

		if(it.index() > 0) {
			if(target == Primary || target == Both)
				// Initialize the value of primaryContinues
				SubtitleLine::fixPunctuation(at(it.index() - 1)->primaryText(), spaces, quotes, engI, ellipsis, &primaryContinues);

			if(target == Secondary || target == Both)
				// Initialize the value of secondaryContinues
				SubtitleLine::fixPunctuation(at(it.index() - 1)->secondaryText(), spaces, quotes, engI, ellipsis, &secondaryContinues);
		}

		for(; it.current(); ++it) {
			switch(target) {
			case Primary:
				it.current()->setPrimaryText(SubtitleLine::fixPunctuation(it.current()->primaryText(), spaces, quotes, engI, ellipsis, &primaryContinues)
											 );
				break;
			case Secondary:
				it.current()->setSecondaryText(SubtitleLine::fixPunctuation(it.current()->secondaryText(), spaces, quotes, engI, ellipsis, &secondaryContinues)
											   );
				break;
			case Both:
				it.current()->setTexts(SubtitleLine::fixPunctuation(it.current()->primaryText(), spaces, quotes, engI, ellipsis, &primaryContinues), SubtitleLine::fixPunctuation(it.current()->secondaryText(), spaces, quotes, engI, ellipsis, &secondaryContinues)
									   );
				break;
			default:
				break;
			}
		}
	}

	endCompositeAction();
}

void
Subtitle::lowerCase(const RangeList &ranges, TextTarget target)
{
	if(m_lines.isEmpty() || target >= TextTargetSIZE)
		return;

	beginCompositeAction(i18n("Lower Case"));

	switch(target) {
	case Primary: {
		for(SubtitleIterator it(*this, ranges); it.current(); ++it)
			it.current()->setPrimaryText(it.current()->primaryText().toLower());
		break;
	}
	case Secondary: {
		for(SubtitleIterator it(*this, ranges); it.current(); ++it)
			it.current()->setSecondaryText(it.current()->secondaryText().toLower());
		break;
	}
	case Both: {
		for(SubtitleIterator it(*this, ranges); it.current(); ++it)
			it.current()->setTexts(it.current()->primaryText().toLower(), it.current()->secondaryText().toLower());
		break;
	}
	default:
		break;
	}

	endCompositeAction();
}

void
Subtitle::upperCase(const RangeList &ranges, TextTarget target)
{
	if(m_lines.isEmpty() || target >= TextTargetSIZE)
		return;

	beginCompositeAction(i18n("Upper Case"));

	switch(target) {
	case Primary: {
		for(SubtitleIterator it(*this, ranges); it.current(); ++it)
			it.current()->setPrimaryText(it.current()->primaryText().toUpper());
		break;
	}
	case Secondary: {
		for(SubtitleIterator it(*this, ranges); it.current(); ++it)
			it.current()->setSecondaryText(it.current()->secondaryText().toUpper());
		break;
	}
	case Both: {
		for(SubtitleIterator it(*this, ranges); it.current(); ++it)
			it.current()->setTexts(it.current()->primaryText().toUpper(), it.current()->secondaryText().toUpper());
		break;
	}
	default:
		break;
	}

	endCompositeAction();
}

void
Subtitle::titleCase(const RangeList &ranges, bool lowerFirst, TextTarget target)
{
	if(m_lines.isEmpty() || target >= TextTargetSIZE)
		return;

	beginCompositeAction(i18n("Title Case"));

	switch(target) {
	case Primary: {
		for(SubtitleIterator it(*this, ranges); it.current(); ++it)
			it.current()->setPrimaryText(it.current()->primaryText().toTitleCase(lowerFirst));
		break;
	}
	case Secondary: {
		for(SubtitleIterator it(*this, ranges); it.current(); ++it)
			it.current()->setSecondaryText(it.current()->secondaryText().toTitleCase(lowerFirst));
		break;
	}
	case Both: {
		for(SubtitleIterator it(*this, ranges); it.current(); ++it)
			it.current()->setTexts(it.current()->primaryText().toTitleCase(lowerFirst), it.current()->secondaryText().toTitleCase(lowerFirst)
								   );
		break;
	}
	default:
		break;
	}

	endCompositeAction();
}

void
Subtitle::sentenceCase(const RangeList &ranges, bool lowerFirst, TextTarget target)
{
	if(m_lines.isEmpty() || target >= TextTargetSIZE)
		return;

	beginCompositeAction(i18n("Sentence Case"));

	for(RangeList::ConstIterator rangesIt = ranges.begin(), rangesEnd = ranges.end(); rangesIt != rangesEnd; ++rangesIt) {
		SubtitleIterator it(*this, *rangesIt);

		bool pCont = false;
		bool sCont = false;

		if(it.index() > 0) {
			if(target == Primary || target == Both) // Initialize pCont
				at(it.index() - 1)->primaryText().toSentenceCase(lowerFirst, &pCont);
			if(target == Secondary || target == Both) // Initialize sCont
				at(it.index() - 1)->secondaryText().toSentenceCase(lowerFirst, &sCont);
		}

		switch(target) {
		case Primary: {
			for(; it.current(); ++it)
				it.current()->setPrimaryText(it.current()->primaryText().toSentenceCase(lowerFirst, &pCont));
			break;
		}
		case Secondary: {
			for(; it.current(); ++it)
				it.current()->setSecondaryText(it.current()->secondaryText().toSentenceCase(lowerFirst, &sCont));
			break;
		}
		case Both: {
			for(; it.current(); ++it)
				it.current()->setTexts(it.current()->primaryText().toSentenceCase(lowerFirst, &pCont), it.current()->secondaryText().toSentenceCase(lowerFirst, &sCont)
									   );
			break;
		}
		default:
			break;
		}
	}

	endCompositeAction();
}

void
Subtitle::breakLines(const RangeList &ranges, unsigned minLengthForLineBreak, TextTarget target)
{
	SubtitleCompositeActionExecutor executor(*this, i18n("Break Lines"));

	for(SubtitleIterator it(*this, ranges); it.current(); ++it)
		it.current()->breakText(minLengthForLineBreak, (SubtitleLine::TextTarget)target);
}

void
Subtitle::unbreakTexts(const RangeList &ranges, TextTarget target)
{
	SubtitleCompositeActionExecutor executor(*this, i18n("Unbreak Lines"));

	for(SubtitleIterator it(*this, ranges); it.current(); ++it)
		it.current()->unbreakText((SubtitleLine::TextTarget)target);
}

void
Subtitle::simplifyTextWhiteSpace(const RangeList &ranges, TextTarget target)
{
	SubtitleCompositeActionExecutor executor(*this, i18n("Simplify Spaces"));

	for(SubtitleIterator it(*this, ranges); it.current(); ++it)
		it.current()->simplifyTextWhiteSpace((SubtitleLine::TextTarget)target);
}

void
Subtitle::syncWithSubtitle(const Subtitle &refSubtitle)
{
	beginCompositeAction(i18n("Synchronize Subtitles"));

	for(SubtitleIterator it(*this, Range::full()), refIt(refSubtitle, Range::full()); it.current() && refIt.current(); ++it, ++refIt)
		it.current()->setTimes(refIt.current()->showTime(), refIt.current()->hideTime());

	endCompositeAction();
}

void
Subtitle::appendSubtitle(const Subtitle &srcSubtitle, long shiftMsecsBeforeAppend)
{
	if(!srcSubtitle.count())
		return;

	QList<SubtitleLine *> lines;
	for(SubtitleIterator it(srcSubtitle); it.current(); ++it) {
		SubtitleLine *newLine = new SubtitleLine(*(it.current()));
		newLine->shiftTimes(shiftMsecsBeforeAppend);
		lines.append(newLine);
	}

	beginCompositeAction(i18n("Join Subtitles"));

	processAction(new InsertLinesAction(*this, lines));

	endCompositeAction();
}

void
Subtitle::splitSubtitle(Subtitle &dstSubtitle, const Time &splitTime, bool shiftSplitLines)
{
	if(!m_lines.count())
		return;

	int splitIndex = -1;            // the index of the first line to move (or copy) to dstSub
	bool splitsLine = false;        // splitTime falls in within a line's time

	QList<SubtitleLine *> lines;
	for(SubtitleIterator it(*this, Range::full()); it.current(); ++it) {
		if(splitTime <= it.current()->hideTime()) {
			SubtitleLine *newLine = new SubtitleLine(*(it.current()));

			if(splitIndex < 0) {    // the first line of the new subtitle
				splitIndex = it.index();
				splitsLine = splitTime > newLine->showTime();

				if(splitsLine)
					newLine->setShowTime(splitTime);
			}

			if(it.current()->m_formatData)
				newLine->m_formatData = new FormatData(*(it.current()->m_formatData));

			if(shiftSplitLines)
				newLine->shiftTimes(-splitTime.toMillis());

			lines.append(newLine);
		}
	}

	if(splitIndex > 0 || (splitIndex == 0 && splitsLine)) {
		dstSubtitle.m_formatData = m_formatData ? new FormatData(*m_formatData) : 0;

		dstSubtitle.beginCompositeAction(i18n("Split Subtitles"));
		if(dstSubtitle.count())
			dstSubtitle.processAction(new RemoveLinesAction(dstSubtitle, 0, -1));
		dstSubtitle.processAction(new InsertLinesAction(dstSubtitle, lines, 0));
		dstSubtitle.endCompositeAction();

		beginCompositeAction(i18n("Split Subtitles"));
		if(splitsLine) {
			at(splitIndex)->setHideTime(splitTime);
			splitIndex++;
			if(splitIndex < count())
				processAction(new RemoveLinesAction(*this, splitIndex));
		} else
			processAction(new RemoveLinesAction(*this, splitIndex));
		endCompositeAction();
	}
}

void
Subtitle::setStyleFlags(const RangeList &ranges, int styleFlags)
{
	beginCompositeAction(i18n("Set Lines Style"));

	for(SubtitleIterator it(*this, ranges); it.current(); ++it) {
		qDebug() << it.current()->primaryText().string();

		it.current()->setPrimaryText(SString(it.current()->primaryText()).setStyleFlags(0, -1, styleFlags));
		it.current()->setSecondaryText(SString(it.current()->secondaryText()).setStyleFlags(0, -1, styleFlags));
	}

	endCompositeAction();
}

void
Subtitle::setStyleFlags(const RangeList &ranges, int styleFlags, bool on)
{
	beginCompositeAction(i18n("Set Lines Style"));

	for(SubtitleIterator it(*this, ranges); it.current(); ++it) {
		it.current()->setPrimaryText(SString(it.current()->primaryText()).setStyleFlags(0, -1, styleFlags, on));
		it.current()->setSecondaryText(SString(it.current()->secondaryText()).setStyleFlags(0, -1, styleFlags, on));
	}

	endCompositeAction();
}

void
Subtitle::toggleStyleFlag(const RangeList &ranges, SString::StyleFlag styleFlag)
{
	SubtitleIterator it(*this, ranges);
	if(!it.current())
		return;

	beginCompositeAction(i18n("Toggle Lines Style"));

	bool isOn = it.current()->primaryText().hasStyleFlags(styleFlag) || it.current()->secondaryText().hasStyleFlags(styleFlag);

	setStyleFlags(ranges, styleFlag, !isOn);

	endCompositeAction();
}

void
Subtitle::changeTextColor(const RangeList &ranges, QRgb color)
{
	beginCompositeAction(i18n("Change Lines Text Color"));

	for(SubtitleIterator it(*this, ranges); it.current(); ++it) {
		it.current()->setPrimaryText(SString(it.current()->primaryText()).setStyleColor(0, -1, color));
		it.current()->setSecondaryText(SString(it.current()->secondaryText()).setStyleColor(0, -1, color));
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
Subtitle::processAction(QUndoCommand *action)
{
	if(app()->subtitle() == this)
		app()->undoStack()->push(action);
	else
		action->redo();
}

void
Subtitle::beginCompositeAction(const QString &title)
{
	QUndoStack *undoStack = app()->undoStack();
	if(app()->subtitle() == this) {
		undoStack->beginMacro(title);
		undoStack->push(new CompositeActionStart(*this));
	} else {
		CompositeActionStart(*this).redo();
	}
}

void
Subtitle::endCompositeAction()
{
	QUndoStack *undoStack = app()->undoStack();
	if(app()->subtitle() == this) {
		undoStack->push(new CompositeActionEnd(*this));
		undoStack->endMacro();
	} else {
		CompositeActionEnd(*this).redo();
	}
}

void
Subtitle::updateState()
{
	auto updatePrimary = [&](const int index){
		m_primaryState = index;
		if(m_primaryState == m_primaryCleanState)
			emit primaryDirtyStateChanged(false);
		else
			emit primaryDirtyStateChanged(true);
		emit primaryChanged();
	};
	auto updateSecondary = [&](const int index){
		m_secondaryState = index;
		if(m_secondaryState == m_secondaryCleanState)
			emit secondaryDirtyStateChanged(false);
		else
			emit secondaryDirtyStateChanged(true);
		emit secondaryChanged();
	};

	const QUndoStack *undoStack = app()->undoStack();
	const int index = undoStack->index();
	const UndoAction *action = index > 0 ? dynamic_cast<const UndoAction *>(undoStack->command(index - 1)) : nullptr;
	const UndoAction::DirtyMode dirtyMode = action != nullptr ? action->m_dirtyMode : SubtitleAction::Both;

	switch(dirtyMode) {
	case SubtitleAction::Primary:
		updatePrimary(index);
		break;

	case SubtitleAction::Secondary:
		updateSecondary(index);
		break;

	case SubtitleAction::Both:
		updatePrimary(index);
		updateSecondary(index);
		break;

	case SubtitleAction::None:
		break;
	}
}

/// SUBTITLECOMPOSITEACTIONEXECUTOR

SubtitleCompositeActionExecutor::SubtitleCompositeActionExecutor(Subtitle &subtitle, const QString &title, bool interactive)
	: m_subtitle(subtitle),
	  m_interactive(interactive)
{
	if(m_interactive)
		app()->linesWidget()->disableModelReset(true);

	m_subtitle.beginCompositeAction(title);
}

SubtitleCompositeActionExecutor::~SubtitleCompositeActionExecutor()
{
	m_subtitle.endCompositeAction();

	if(m_interactive)
		app()->linesWidget()->disableModelReset(false);
}
