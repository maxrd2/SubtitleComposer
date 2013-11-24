/***************************************************************************
 *   Copyright (C) 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#include "subtitle.h"
#include "subtitleline.h"
#include "subtitleiterator.h"
#include "subtitleactions.h"
#include "compositeaction.h"

#include <KLocale>

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

Subtitle::Subtitle(double framesPerSecond) :
	m_primaryState(0),
	m_primaryCleanState(0),
	m_secondaryState(0),
	m_secondaryCleanState(0),
	m_compositeAction(0),
	m_compositeActionDepth(0),
	m_framesPerSecond(framesPerSecond),
	m_lastValidCachedIndex(-1),
	m_formatData(0)
{}

Subtitle::~Subtitle()
{
	qDeleteAll(m_lines);

	delete m_compositeAction;

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

	SubtitleIterator fromIt(from, Range::full());
	SubtitleIterator thisIt(*this, Range::full());

	// the errors that we are going to take from 'from':
	const int fromErrors = usePrimaryData ? SubtitleLine::PrimaryOnlyErrors : SubtitleLine::SecondaryOnlyErrors;
	// the errors that we are going to keep:
	const int thisErrors = SubtitleLine::PrimaryOnlyErrors | SubtitleLine::SharedErrors;

	for(SubtitleLine *fromLine = fromIt.current(), *thisLine = thisIt.current(); fromLine && thisLine; ++fromIt, ++thisIt, fromLine = fromIt.current(), thisLine = thisIt.current()) {
		thisLine->setSecondaryText(usePrimaryData ? fromLine->primaryText() : fromLine->secondaryText());
		thisLine->setErrorFlags((thisLine->errorFlags() & thisErrors) | (fromLine->errorFlags() & fromErrors));
	}

	if(fromIt.current()) {          // from subtitle had more lines than *this
		QList<SubtitleLine *> lines;
		for(; fromIt.current(); ++fromIt) {
			SubtitleLine *thisLine = new SubtitleLine(*fromIt.current());
			if(usePrimaryData)
				thisLine->setSecondaryText(thisLine->primaryText());
			thisLine->setPrimaryText(SString());
			thisLine->setErrorFlags(SubtitleLine::PrimaryOnlyErrors, false);
			lines.append(thisLine);
		}
		processAction(new InsertLinesAction(*this, lines));
	} else if(thisIt.current()) {   // *this had more lines than from subtitle
		for(SubtitleLine *thisLine = thisIt.current(); thisLine; ++thisIt, thisLine = thisIt.current()) {
			thisLine->setSecondaryText(SString());
			thisLine->setErrorFlags(SubtitleLine::SecondaryOnlyErrors, false);
		}
	}

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

ActionManager &
Subtitle::actionManager()
{
	return m_actionManager;
}

const ActionManager &
Subtitle::actionManager() const
{
	return m_actionManager;
}

double
Subtitle::framesPerSecond() const
{
	return m_framesPerSecond;
}

void
Subtitle::setFramesPerSecond(double framesPerSecond)
{
	if(m_framesPerSecond != framesPerSecond)
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

			it.current()->setTimes(showTime, hideTime);
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
//      return (index < m_lines.count() ? m_lines.at( index ) : 0);
	return index < 0 ? 0 : (index < m_lines.count() ? m_lines.at(index) : 0);
}

const SubtitleLine *
Subtitle::line(int index) const
{
//      return (index < m_lines.count() ? m_lines.at( index ) : 0);
	return index < 0 ? 0 : (index < m_lines.count() ? m_lines.at(index) : 0);
}

SubtitleLine *
Subtitle::firstLine()
{
	return m_lines.isEmpty() ? 0 : m_lines.first();
}

const SubtitleLine *
Subtitle::firstLine() const
{
	return m_lines.isEmpty() ? 0 : m_lines.first();
}

SubtitleLine *
Subtitle::lastLine()
{
	return m_lines.isEmpty() ? 0 : m_lines.last();
}

const SubtitleLine *
Subtitle::lastLine() const
{
	return m_lines.isEmpty() ? 0 : m_lines.last();
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
Subtitle::insertNewLine(int index, bool timeAfter, TextTarget target)
{
	Q_ASSERT(index <= m_lines.count());

	if(index < 0)
		index = m_lines.count();

	SubtitleLine *newLine = new SubtitleLine();
	int newLineIndex = (target == Secondary) ? m_lines.count() : index;

	if(timeAfter) {
		if(newLineIndex) {              // there is a previous line
			SubtitleLine *prevLine = m_lines.value(newLineIndex - 1);
			newLine->setTimes(prevLine->hideTime() + 100, prevLine->hideTime() + 1000);
		} else if(newLineIndex < m_lines.count()) {     // there is a next line
			SubtitleLine *nextLine = m_lines.value(newLineIndex);
			newLine->setTimes(nextLine->showTime() - 1100, nextLine->showTime() - 100);
		} else
			newLine->setHideTime(1000);
	} else {                                        // ! timeAfter
		if(newLineIndex < m_lines.count()) {    // there is a next line
			SubtitleLine *nextLine = m_lines.at(newLineIndex);
			newLine->setTimes(nextLine->showTime() - 1100, nextLine->showTime() - 100);
		} else if(newLineIndex) {       // there is a previous line
			SubtitleLine *prevLine = m_lines.at(newLineIndex - 1);
			newLine->setTimes(prevLine->hideTime() + 100, prevLine->hideTime() + 1000);
		} else
			newLine->setHideTime(1000);
	}

	if(target == Both || index == m_lines.count()) {
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
	} else {                                        // if target == Primary
		beginCompositeAction(i18n("Remove Lines"), true, false);

		RangeList mutableRanges(ranges);
		mutableRanges.trimToIndex(m_lines.count() - 1);

		// first, we need to append as many empty lines as we're to remove
		// we insert them with a greater time than the one of the last (non deleted) line

		int linesCount = m_lines.count();

		Range lastRange = mutableRanges.last();
		int lastIndex = lastRange.end() == linesCount - 1 ? lastRange.start() - 1 : linesCount - 1;
		SubtitleLine *lastLine = lastIndex < linesCount ? m_lines.at(lastIndex) : 0;
		Time showTime(lastLine ? lastLine->hideTime() + 100 : 0);
		Time hideTime(showTime + 1000);

		QList<SubtitleLine *> lines;
		for(int index = 0, size = ranges.indexesCount(); index < size; ++index) {
			lines.append(new SubtitleLine(SString(), SString(), showTime, hideTime));
			showTime.shift(1100);
			hideTime.shift(1100);
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
	beginCompositeAction(i18n("Split Lines"));

	for(SubtitleIterator it(*this, ranges, true); it.current(); --it) {
		SubtitleLine *line = it.current();
		if(line->primaryLines() > 1) {
			line->simplifyTextWhiteSpace(SubtitleLine::Primary);

			long autoDurationsSum = 0;
			QList<int> autoDurations;
			SStringList primaryLines = line->primaryText().split('\n');
			for(SStringList::ConstIterator ptIt = primaryLines.begin(), ptEnd = primaryLines.end(); ptIt != ptEnd; ++ptIt) {
				const Time &autoDuration = SubtitleLine::autoDuration((*ptIt).string(), 60, 50,
																	  50);
				autoDurations.append(autoDuration.toMillis());
				autoDurationsSum += autoDuration.toMillis();
			}

			double factor = (double)line->durationTime().toMillis() / autoDurationsSum;

			SStringList secondaryLines = line->secondaryText().split('\n');
			while(secondaryLines.count() < primaryLines.count())
				secondaryLines.append(SString());
			while(secondaryLines.count() > primaryLines.count()) {
				SString lastLine = secondaryLines.last();
				secondaryLines.pop_back();
				secondaryLines.last() += '\n';
				secondaryLines.last() += lastLine;
			}

			int subLineIndex = it.index(), splitLineIndex = 0;
			for(SStringList::ConstIterator ptIt = primaryLines.begin(), ptEnd = secondaryLines.end(), stIt = secondaryLines.begin(), stEnd = secondaryLines.end(); ptIt != ptEnd && stIt != stEnd; ++ptIt, ++stIt, ++subLineIndex, ++splitLineIndex) {
				if(splitLineIndex) {
					SubtitleLine *newLine = new SubtitleLine();
					newLine->setShowTime(line->hideTime() + 1);
					insertLine(newLine, subLineIndex);
					line = newLine;
				}

				line->setTexts(*ptIt, *stIt);
				line->setDurationTime(Time((long)
										   (factor * autoDurations[splitLineIndex]) - 1));
			}
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

		SubtitleLine *firstLine = m_lines.at(rangeStart);
		SubtitleLine *lastLine = m_lines.at(rangeEnd);

		SString primaryText, secondaryText;

		for(SubtitleIterator it(*this, Range(rangeStart, rangeEnd - 1)); it.current(); ++it) {
			if(!it.current()->primaryText().isEmpty()) {
				primaryText.append(it.current()->primaryText());
				primaryText.append("\n");
			}

			if(!it.current()->secondaryText().isEmpty()) {
				secondaryText.append(it.current()->secondaryText());
				secondaryText.append("\n");
			}
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
Subtitle::shiftLines(const RangeList &ranges, long msecs)
{
	if(msecs == 0)
		return;

	beginCompositeAction(i18n("Shift Lines"));

	for(SubtitleIterator it(*this, ranges); it.current(); ++it)
		it.current()->shiftTimes(msecs);

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

	long oldFirstTime = m_lines.at(firstIndex)->showTime().toMillis();
	long oldLastTime = m_lines.at(lastIndex)->showTime().toMillis();
	long oldDeltaTime = oldLastTime - oldFirstTime;

	long newDeltaTime = newLastTime - newFirstTime;

	// special case in which we can't procede as there's no way to
	// linearly transform the same time into two different ones...
	if(!oldDeltaTime && newDeltaTime)
		return;

	double shiftMseconds;
	double scaleFactor;

	if(oldDeltaTime) {
		shiftMseconds = newFirstTime - ((double)newDeltaTime / oldDeltaTime) * oldFirstTime;
		scaleFactor = (double)newDeltaTime / oldDeltaTime;
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
	beginCompositeAction(i18n("Sort"), true, false);

	SubtitleIterator it(*this, range);
	SubtitleLine *line = it.current();
	++it;
	int itPositions = 1;
	for(SubtitleLine *nextLine = it.current(); nextLine; ++it, line = nextLine, nextLine = it.current(), ++itPositions) {
		if(nextLine->showTime() < line->showTime()) {   // nextLine is not at the right position, we'll move it backwards
			int fromIndex = it.index();     // nextLine current index

			// We set the first target index candidate. Note that we can't tell is this is the
			// correct target index as nextLine my be misplaced by more than one position.
			// We do now, however, that the new index must be one before fromIndex.
			int toIndex = (--it).index();

			// we iterate backwards finding the correct position knowing that all previous lines ARE sorted
			for(; it.current(); --it) {
				if(it.current()->showTime() < nextLine->showTime()) {
					toIndex = (++it).index();
					break;
				}
			}

//                      kDebug() << "moving from" << fromIndex << "to" << toIndex;
			processAction(new MoveLineAction(*this, fromIndex, toIndex));

			// moving the lines invalidates the iterator so we recreate it and advance it to where it was
			it = SubtitleIterator(*this, range);
			it += itPositions;
			nextLine = it.current();
		}
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
				SubtitleLine::fixPunctuation(m_lines.at(it.index() - 1)->primaryText(), spaces, quotes, engI, ellipsis, &primaryContinues);

			if(target == Secondary || target == Both)
				// Initialize the value of secondaryContinues
				SubtitleLine::fixPunctuation(m_lines.at(it.index() - 1)->secondaryText(), spaces, quotes, engI, ellipsis, &secondaryContinues);
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
				m_lines.at(it.index() - 1)->primaryText().toSentenceCase(lowerFirst, &pCont);
			if(target == Secondary || target == Both) // Initialize sCont
				m_lines.at(it.index() - 1)->secondaryText().toSentenceCase(lowerFirst, &sCont);
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
	if(!srcSubtitle.m_lines.count())
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
		if(dstSubtitle.m_lines.count())
			dstSubtitle.processAction(new RemoveLinesAction(dstSubtitle, 0, -1));
		dstSubtitle.processAction(new InsertLinesAction(dstSubtitle, lines, 0));
		dstSubtitle.endCompositeAction();

		beginCompositeAction(i18n("Split Subtitles"));
		if(splitsLine) {
			m_lines.at(splitIndex)->setHideTime(splitTime);
			splitIndex++;
			if(splitIndex < m_lines.size())
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
		kDebug() << it.current()->primaryText().string();

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
	beginCompositeAction(i18n("Clear Lines Errors"));

	for(SubtitleIterator it(*this, ranges); it.current(); ++it)
		it.current()->setErrorFlags(errorFlags, false);

	endCompositeAction();
}

void
Subtitle::checkErrors(const RangeList &ranges, int errorFlags, int minDurationMsecs, int maxDurationMsecs, int minMsecsPerChar, int maxMsecsPerChar, int maxChars, int maxLines)
{
	beginCompositeAction(i18n("Check Lines Errors"));

	for(SubtitleIterator it(*this, ranges); it.current(); ++it) {
		it.current()->check(errorFlags, minDurationMsecs, maxDurationMsecs, minMsecsPerChar, maxMsecsPerChar, maxChars, maxLines);
	}

	endCompositeAction();
}

void
Subtitle::recheckErrors(const RangeList &ranges, int minDurationMsecs, int maxDurationMsecs, int minMsecsPerChar, int maxMsecsPerChar, int maxChars, int maxLines)
{
	beginCompositeAction(i18n("Check Lines Errors"));

	for(SubtitleIterator it(*this, ranges); it.current(); ++it) {
		it.current()->check(it.current()->errorFlags(), minDurationMsecs, maxDurationMsecs, minMsecsPerChar, maxMsecsPerChar, maxChars, maxLines);
	}

	endCompositeAction();
}

void
Subtitle::processAction(Action *action)
{
	if(m_compositeAction)
		m_compositeAction->appendAction(action);
	else
		m_actionManager.execAndStore(action);
}

void
Subtitle::beginCompositeAction(const QString &title, bool immediateExecution, bool delaySignals)
{
	m_compositeActionDepth++;

	if(!m_compositeAction)
		m_compositeAction = new CompositeAction(title, immediateExecution, delaySignals);
}

void
Subtitle::endCompositeAction()
{
	m_compositeActionDepth--;

	if(!m_compositeActionDepth) {
		// NOTE: it is EXTREMELY IMPORTANT to mark the composite action as finished (setting it to 0)
		// before performing m_actionManager.execAndStore() because that call may spawn other composite
		// actions that won't be correctly initialized if m_compositeAction was not set to 0.
		CompositeAction *compositeAction = m_compositeAction;
		m_compositeAction = 0;

		if(compositeAction->count() > 1)
			m_actionManager.execAndStore(compositeAction);
		else {
			if(compositeAction->count()) // compositeAction->count() == 1
				m_actionManager.execAndStore(compositeAction->detachContainedAction());
			delete compositeAction;
		}
	}
}

void
Subtitle::incrementState(int dirtyMode)
{
	switch(dirtyMode) {
	case SubtitleAction::Primary:
		m_primaryState++;
		if(m_primaryState == (m_primaryCleanState + 1))
			emit primaryDirtyStateChanged(true);
		else if(m_primaryState == m_primaryCleanState)
			emit primaryDirtyStateChanged(false);
		break;
	case SubtitleAction::Secondary:
		m_secondaryState++;
		if(m_secondaryState == (m_secondaryCleanState + 1))
			emit secondaryDirtyStateChanged(true);
		else if(m_secondaryState == m_secondaryCleanState)
			emit secondaryDirtyStateChanged(false);
		break;
	case SubtitleAction::Both:
		m_primaryState++;
		m_secondaryState++;
		if(m_primaryState == (m_primaryCleanState + 1))
			emit primaryDirtyStateChanged(true);
		else if(m_primaryState == m_primaryCleanState)
			emit primaryDirtyStateChanged(false);
		if(m_secondaryState == (m_secondaryCleanState + 1))
			emit secondaryDirtyStateChanged(true);
		else if(m_secondaryState == m_secondaryCleanState)
			emit secondaryDirtyStateChanged(false);
		break;
	default:
		break;
	}
}

void
Subtitle::decrementState(int dirtyMode)
{
	switch(dirtyMode) {
	case SubtitleAction::Primary:
		m_primaryState--;
		if(m_primaryState == m_primaryCleanState)
			emit primaryDirtyStateChanged(false);
		else if(m_primaryState == (m_primaryCleanState - 1))
			emit primaryDirtyStateChanged(true);
		break;
	case SubtitleAction::Secondary:
		m_secondaryState--;
		if(m_secondaryState == m_secondaryCleanState)
			emit secondaryDirtyStateChanged(false);
		else if(m_secondaryState == (m_secondaryCleanState - 1))
			emit secondaryDirtyStateChanged(true);
		break;
	case SubtitleAction::Both:
		m_primaryState--;
		m_secondaryState--;
		if(m_primaryState == m_primaryCleanState)
			emit primaryDirtyStateChanged(false);
		else if(m_primaryState == (m_primaryCleanState - 1))
			emit primaryDirtyStateChanged(true);
		if(m_secondaryState == m_secondaryCleanState)
			emit secondaryDirtyStateChanged(false);
		else if(m_secondaryState == (m_secondaryCleanState - 1))
			emit secondaryDirtyStateChanged(true);
		break;
	}
}

void
Subtitle::setLastValidCachedIndex(int lastValidCachedIndex)
{
	if(lastValidCachedIndex > m_lastValidCachedIndex) {
		// It's possible for incrementing m_lastValidCachedIndex to result in some lines having
		// an m_cachedIndex value lower than lastValidCachedIndex which though previously regarded
		// as correct, would be incorrect once we change m_lastValidCachedIndex.
		// We have to invalidate such lines' m_cachedIndex.

		SubtitleLine *line;
		for(int lineIndex = lastValidCachedIndex + 1, linesCount = m_lines.count(); lineIndex < linesCount; ++linesCount) {
			line = m_lines.at(lineIndex);
			if(line->m_cachedIndex < 0 || line->m_cachedIndex > lastValidCachedIndex)
				break;
			line->m_cachedIndex = -1;
		}
	} else if(lastValidCachedIndex < m_lastValidCachedIndex) {
		if(m_lines.count() > lastValidCachedIndex + 1)
			m_lines.at(lastValidCachedIndex + 1)->m_cachedIndex = -1;
	} else
		return;

	m_lastValidCachedIndex = lastValidCachedIndex;

	/*
	   kDebug() << "last valid cached index" << m_lastValidCachedIndex;

	   QStringList cacheIndexList, indexList;
	   for ( int lineIndex = 0, linesCount = m_lines.count(); lineIndex < linesCount; ++lineIndex )
	   cacheIndexList << QString::number( m_lines.at( lineIndex )->m_cachedIndex );
	   kDebug() << "cached indexes:" << cacheIndexList.join( " " );

	   for ( int lineIndex = 0, linesCount = m_lines.count(); lineIndex < linesCount; ++lineIndex )
	   indexList << QString::number( m_lines.at( lineIndex )->index() );
	   kDebug() << "indexes:" << indexList.join( " " );
	 */
}

/// SUBTITLECOMPOSITEACTIONEXECUTOR

SubtitleCompositeActionExecutor::SubtitleCompositeActionExecutor(Subtitle &subtitle, const QString &title) :
	m_subtitle(subtitle)
{
	m_subtitle.beginCompositeAction(title);
}

SubtitleCompositeActionExecutor::~SubtitleCompositeActionExecutor()
{
	m_subtitle.endCompositeAction();
}

#include "subtitle.moc"
