/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2017 Mladen Milinkovic <max@smoothware.net>
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

#include "subtitleactions.h"
#include "subtitleiterator.h"
#include "subtitleline.h"
#include "sstring.h"

#include <QObject>

#include <KLocalizedString>

using namespace SubtitleComposer;

/// SUBTITLE ACTION
/// ===============

SubtitleAction::SubtitleAction(Subtitle &subtitle, SubtitleAction::DirtyMode dirtyMode, const QString &description) :
	Action(description),
	m_subtitle(subtitle),
	m_dirtyMode(dirtyMode)
{}

SubtitleAction::~SubtitleAction()
{}

void
SubtitleAction::internalPreRedo()
{
	m_subtitle.incrementState(m_dirtyMode);
}

void
SubtitleAction::internalPreUndo()
{
	m_subtitle.decrementState(m_dirtyMode);
}

/// SET FRAMES PER SECOND ACTION
/// ============================

SetFramesPerSecondAction::SetFramesPerSecondAction(Subtitle &subtitle, double framesPerSecond) : SubtitleAction(subtitle, SubtitleAction::Both, i18n("Set Frame Rate")),
	m_framesPerSecond(framesPerSecond)
{}

SetFramesPerSecondAction::~SetFramesPerSecondAction()
{}

void
SetFramesPerSecondAction::internalRedo()
{
	double aux = m_subtitle.m_framesPerSecond;
	m_subtitle.m_framesPerSecond = m_framesPerSecond;
	m_framesPerSecond = aux;
}

void
SetFramesPerSecondAction::internalUndo()
{
	internalRedo();
}

void
SetFramesPerSecondAction::internalEmitRedoSignals()
{
	emit m_subtitle.framesPerSecondChanged(m_subtitle.m_framesPerSecond);
}

void
SetFramesPerSecondAction::internalEmitUndoSignals()
{
	internalEmitRedoSignals();
}

/// INSERT LINES ACTION
/// ===================

InsertLinesAction::InsertLinesAction(Subtitle &subtitle, const QList<SubtitleLine *> &lines, int insertIndex) : SubtitleAction(subtitle, SubtitleAction::Both, i18n("Insert Lines")),
	m_insertIndex(insertIndex < 0 ? subtitle.linesCount() : insertIndex),
	m_lastIndex(m_insertIndex + lines.count() - 1),
	m_lines(lines)
{
	Q_ASSERT(m_insertIndex >= 0);
	Q_ASSERT(m_insertIndex <= m_subtitle.linesCount());
	Q_ASSERT(m_lastIndex >= 0);
	Q_ASSERT(m_insertIndex <= m_lastIndex);
}

InsertLinesAction::~InsertLinesAction()
{
	qDeleteAll(m_lines);
}

bool
InsertLinesAction::mergeWithPrevious(Action *pa)
{
	InsertLinesAction *prevAction = dynamic_cast<InsertLinesAction *>(pa);
	if(!prevAction || &prevAction->m_subtitle != &m_subtitle)
		return false;

	if(m_insertIndex == prevAction->m_insertIndex) {
		// this inserted lines immediately above those inserted by prevAction
		m_lastIndex += (prevAction->m_lastIndex - prevAction->m_insertIndex + 1);
		prevAction->internalPreUndo();
		return true;
	} else if(m_insertIndex == prevAction->m_lastIndex + 1) {
		// this inserted lines immediately below those inserted by prevAction
		m_insertIndex -= (prevAction->m_lastIndex - prevAction->m_insertIndex + 1);
		prevAction->internalPreUndo();
		return true;
	}

	return false;
}

void
InsertLinesAction::internalRedo()
{
	emit m_subtitle.linesAboutToBeInserted(m_insertIndex, m_lastIndex);

	SubtitleLine *line;
	int insertOffset = 0;
	int lineIndex = -1;

	while(!m_lines.isEmpty()) {
		line = m_lines.takeFirst();
		lineIndex = m_insertIndex + insertOffset++;
		m_subtitle.m_lines.insert(lineIndex, line);
		setLineSubtitle(line, lineIndex);
	}

	if(m_subtitle.m_lastValidCachedIndex >= m_insertIndex || m_insertIndex == 0)
		m_subtitle.setLastValidCachedIndex(lineIndex);

	emit m_subtitle.linesInserted(m_insertIndex, m_lastIndex);
}

void
InsertLinesAction::internalUndo()
{
	emit m_subtitle.linesAboutToBeRemoved(m_insertIndex, m_lastIndex);

	for(int index = m_insertIndex; index <= m_lastIndex; ++index) {
		SubtitleLine *line = m_subtitle.m_lines.takeAt(m_insertIndex);
		clearLineSubtitle(line);
		m_lines.append(line);
	}

	if(m_subtitle.m_lastValidCachedIndex >= m_insertIndex)
		m_subtitle.setLastValidCachedIndex(m_insertIndex - 1);

	emit m_subtitle.linesRemoved(m_insertIndex, m_lastIndex);
}

/// REMOVE LINES ACTION
/// ===================

RemoveLinesAction::RemoveLinesAction(Subtitle &subtitle, int firstIndex, int lastIndex) :
	SubtitleAction(subtitle, SubtitleAction::Both, i18n("Remove Lines")),
	m_firstIndex(firstIndex),
	m_lastIndex(lastIndex < 0 ? subtitle.lastIndex() : lastIndex),
	m_lines()
{
	Q_ASSERT(m_firstIndex >= 0);
	Q_ASSERT(m_firstIndex <= m_subtitle.linesCount());
	Q_ASSERT(m_lastIndex >= 0);
	Q_ASSERT(m_lastIndex <= m_subtitle.linesCount());
	Q_ASSERT(m_firstIndex <= m_lastIndex);
}

RemoveLinesAction::~RemoveLinesAction()
{
	qDeleteAll(m_lines);
}

bool
RemoveLinesAction::mergeWithPrevious(Action *pa)
{
	RemoveLinesAction *prevAction = dynamic_cast<RemoveLinesAction *>(pa);
	if(!prevAction || &prevAction->m_subtitle != &m_subtitle)
		return false;

	if(m_firstIndex == prevAction->m_firstIndex) {
		// this removed lines immediately below those removed by prevAction
		m_lastIndex += prevAction->m_lines.count();
		while(!prevAction->m_lines.isEmpty())
			m_lines.prepend(prevAction->m_lines.takeLast());
		prevAction->internalPreUndo();
		return true;
	} else if(m_lastIndex + 1 == prevAction->m_firstIndex) {
		// this removed lines immediately above those removed by prevAction
		m_lastIndex += prevAction->m_lines.count();
		while(!prevAction->m_lines.isEmpty())
			m_lines.append(prevAction->m_lines.takeFirst());

		prevAction->internalPreUndo();
		return true;
	}

	return false;
}

void
RemoveLinesAction::internalRedo()
{
	emit m_subtitle.linesAboutToBeRemoved(m_firstIndex, m_lastIndex);

	for(int index = m_firstIndex; index <= m_lastIndex; ++index) {
		SubtitleLine *line = m_subtitle.m_lines.takeAt(m_firstIndex);
		clearLineSubtitle(line);
		m_lines.append(line);
	}

	if(m_subtitle.m_lastValidCachedIndex >= m_firstIndex)
		m_subtitle.setLastValidCachedIndex(m_firstIndex - 1);

	emit m_subtitle.linesRemoved(m_firstIndex, m_lastIndex);
}

void
RemoveLinesAction::internalUndo()
{
	emit m_subtitle.linesAboutToBeInserted(m_firstIndex, m_lastIndex);

	int insertOffset = 0;
	int lineIndex = -1;

	while(!m_lines.isEmpty()) {
		SubtitleLine *line = m_lines.takeFirst();
		lineIndex = m_firstIndex + insertOffset++;
		m_subtitle.m_lines.insert(lineIndex, line);
		setLineSubtitle(line, lineIndex);
	}

	if(m_subtitle.m_lastValidCachedIndex >= m_firstIndex || m_firstIndex == 0)
		m_subtitle.setLastValidCachedIndex(lineIndex);

	emit m_subtitle.linesInserted(m_firstIndex, m_lastIndex);
}

/// MOVE LINE ACTION
/// ================

MoveLineAction::MoveLineAction(Subtitle &subtitle, int fromIndex, int toIndex) :
	SubtitleAction(subtitle, SubtitleAction::Both, i18n("Move Line")),
	m_fromIndex(fromIndex),
	m_toIndex(toIndex < 0 ? subtitle.lastIndex() : toIndex)
{
	Q_ASSERT(m_fromIndex >= 0);
	Q_ASSERT(m_fromIndex <= m_subtitle.linesCount());
	Q_ASSERT(m_toIndex >= 0);
	Q_ASSERT(m_toIndex <= m_subtitle.linesCount());
	Q_ASSERT(m_fromIndex != m_toIndex);
}

MoveLineAction::~MoveLineAction()
{}

bool
MoveLineAction::mergeWithPrevious(Action *pa)
{
	MoveLineAction *prevAction = dynamic_cast<MoveLineAction *>(pa);
	if(!prevAction || &prevAction->m_subtitle != &m_subtitle)
		return false;

	Q_ASSERT(pa != this);

	bool compressed = false;

	if(prevAction->m_toIndex == m_fromIndex) {
		m_fromIndex = prevAction->m_fromIndex;
		compressed = true;
	}
	// when the distance between fromIndex and toIndex is 1, the action is the same as if the values were swapped
	else if(m_toIndex - m_fromIndex == 1 || m_fromIndex - m_toIndex == 1) {
		if(prevAction->m_toIndex == m_toIndex) {
			m_toIndex = m_fromIndex;
			m_fromIndex = prevAction->m_fromIndex;
			compressed = true;
		}
		// same as before, but now we consider inverting the previous action too
		else if(prevAction->m_toIndex - prevAction->m_fromIndex == 1 || prevAction->m_fromIndex - prevAction->m_toIndex == 1) {
			if(prevAction->m_fromIndex == m_toIndex) {
				m_toIndex = m_fromIndex;
				m_fromIndex = prevAction->m_toIndex;
				compressed = true;
			}
		}
	}
	// again, same as before, but now we consider inverting only the previous action
	else if(prevAction->m_toIndex - prevAction->m_fromIndex == 1 || prevAction->m_fromIndex - prevAction->m_toIndex == 1) {
		if(prevAction->m_fromIndex == m_fromIndex) {
			m_fromIndex = prevAction->m_toIndex;
			compressed = true;
		}
	}

	if(compressed) {
		prevAction->internalPreUndo();
	}

	return compressed;
}

void
MoveLineAction::internalRedo()
{
	emit m_subtitle.linesAboutToBeRemoved(m_fromIndex, m_fromIndex);
	SubtitleLine *line = m_subtitle.m_lines.takeAt(m_fromIndex);
	clearLineSubtitle(line);
	if(m_subtitle.m_lastValidCachedIndex >= m_fromIndex)
		m_subtitle.setLastValidCachedIndex(m_fromIndex - 1);
	emit m_subtitle.linesRemoved(m_fromIndex, m_fromIndex);

	emit m_subtitle.linesAboutToBeInserted(m_toIndex, m_toIndex);
	m_subtitle.m_lines.insert(m_toIndex, line);
	setLineSubtitle(line, m_toIndex);
	if(m_subtitle.m_lastValidCachedIndex >= m_toIndex || m_toIndex == 0)
		m_subtitle.setLastValidCachedIndex(m_toIndex);
	emit m_subtitle.linesInserted(m_toIndex, m_toIndex);
}

void
MoveLineAction::internalUndo()
{
	emit m_subtitle.linesAboutToBeRemoved(m_toIndex, m_toIndex);
	SubtitleLine *line = m_subtitle.m_lines.takeAt(m_toIndex);
	clearLineSubtitle(line);
	if(m_subtitle.m_lastValidCachedIndex >= m_toIndex)
		m_subtitle.setLastValidCachedIndex(m_toIndex - 1);
	emit m_subtitle.linesRemoved(m_toIndex, m_toIndex);

	emit m_subtitle.linesAboutToBeInserted(m_fromIndex, m_fromIndex);
	m_subtitle.m_lines.insert(m_fromIndex, line);
	setLineSubtitle(line, m_fromIndex);
	if(m_subtitle.m_lastValidCachedIndex >= m_fromIndex || m_fromIndex == 0)
		m_subtitle.setLastValidCachedIndex(m_fromIndex);
	emit m_subtitle.linesInserted(m_fromIndex, m_fromIndex);
}

/// SWAP LINES TEXTS ACTION
/// =======================

SwapLinesTextsAction::SwapLinesTextsAction(Subtitle &subtitle, const RangeList &ranges) :
	SubtitleAction(subtitle, SubtitleAction::Both, i18n("Swap Texts")),
	m_ranges(ranges)
{}

SwapLinesTextsAction::~SwapLinesTextsAction()
{}

void
SwapLinesTextsAction::internalRedo()
{
	for(SubtitleIterator it(m_subtitle, m_ranges); it.current(); ++it) {
		SubtitleLine *line = it.current();
		SString aux = line->m_primaryText;
		line->m_secondaryText = line->m_primaryText;
		line->m_primaryText = aux;
	}
}

void
SwapLinesTextsAction::internalUndo()
{
	internalRedo();
}

void
SwapLinesTextsAction::internalEmitRedoSignals()
{
	for(SubtitleIterator it(m_subtitle, m_ranges); it.current(); ++it) {
		SubtitleLine *line = it.current();
		emit line->primaryTextChanged(line->m_primaryText);
		emit line->secondaryTextChanged(line->m_secondaryText);
	}
}

void
SwapLinesTextsAction::internalEmitUndoSignals()
{
	internalEmitRedoSignals();
}
