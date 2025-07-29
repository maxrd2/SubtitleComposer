/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "core/undo/subtitleactions.h"
#include "core/subtitleiterator.h"
#include "core/subtitleline.h"
#include "core/richstring.h"

#include <QObject>
#include <QTextDocument>
#include <QTextEdit>

#include <KLocalizedString>

using namespace SubtitleComposer;

// *** SubtitleAction
SubtitleAction::SubtitleAction(Subtitle *subtitle, UndoStack::DirtyMode dirtyMode, const QString &description)
	: UndoAction(dirtyMode, subtitle, description)
{}

SubtitleAction::~SubtitleAction()
{}


// *** SetFramesPerSecondAction
SetFramesPerSecondAction::SetFramesPerSecondAction(Subtitle *subtitle, double framesPerSecond)
	: SubtitleAction(subtitle, UndoStack::Both, i18n("Set Frame Rate")),
	  m_framesPerSecond(framesPerSecond)
{}

SetFramesPerSecondAction::~SetFramesPerSecondAction()
{}

void
SetFramesPerSecondAction::redo()
{
	double tmp = m_subtitle->m_framesPerSecond;
	m_subtitle->m_framesPerSecond = m_framesPerSecond;
	m_framesPerSecond = tmp;

	emit m_subtitle->framesPerSecondChanged(m_subtitle->m_framesPerSecond);
}


// *** InsertLinesAction
InsertLinesAction::InsertLinesAction(Subtitle *subtitle, const QList<SubtitleLine *> &lines, int insertIndex)
	: SubtitleAction(subtitle, UndoStack::Both, i18n("Insert Lines")),
	  m_insertIndex(insertIndex < 0 ? subtitle->linesCount() : insertIndex),
	  m_lastIndex(m_insertIndex + lines.count() - 1),
	  m_lines(lines)
{
	Q_ASSERT(m_insertIndex >= 0 && m_insertIndex <= m_subtitle->linesCount());
	Q_ASSERT(m_lastIndex >= 0);
	Q_ASSERT(m_insertIndex <= m_lastIndex);
}

InsertLinesAction::~InsertLinesAction()
{
	qDeleteAll(m_lines);
}

bool
InsertLinesAction::mergeWith(const QUndoCommand *command)
{
	const InsertLinesAction *currentAction = static_cast<const InsertLinesAction *>(command);
	if(currentAction->m_subtitle != m_subtitle)
		return false;

	if(currentAction->m_insertIndex == m_lastIndex + 1 || (m_insertIndex <= currentAction->m_lastIndex && currentAction->m_insertIndex <= m_lastIndex)) {
		m_lastIndex += currentAction->m_lastIndex - currentAction->m_insertIndex + 1;
		if(m_insertIndex > currentAction->m_insertIndex) {
			m_lastIndex -= m_insertIndex - currentAction->m_insertIndex;
			m_insertIndex = currentAction->m_insertIndex;
		}
		return true;
	}

	return false;
}

void
InsertLinesAction::redo()
{
	emit m_subtitle->linesAboutToBeInserted(m_insertIndex, m_lastIndex);

	SubtitleLine *line;
	int insertOffset = 0;

	while(!m_lines.isEmpty()) {
		line = m_lines.takeFirst();
		auto pos = m_subtitle->m_lines.cbegin() + m_insertIndex + insertOffset++;
		setLineSubtitle(line);
		m_subtitle->m_lines.insert(pos, line);
	}

	emit m_subtitle->linesInserted(m_insertIndex, m_lastIndex);
}

void
InsertLinesAction::undo()
{
	emit m_subtitle->linesAboutToBeRemoved(m_insertIndex, m_lastIndex);

	for(int index = m_insertIndex; index <= m_lastIndex; ++index) {
		SubtitleLine *line = m_subtitle->takeAt(m_insertIndex);
		clearLineSubtitle(line);
		m_lines.append(line);
	}

	emit m_subtitle->linesRemoved(m_insertIndex, m_lastIndex);
}


// *** RemoveLinesAction
RemoveLinesAction::RemoveLinesAction(Subtitle *subtitle, int firstIndex, int lastIndex)
	: SubtitleAction(subtitle, UndoStack::Both, i18n("Remove Lines")),
	  m_firstIndex(firstIndex),
	  m_lastIndex(lastIndex < 0 ? subtitle->lastIndex() : lastIndex),
	  m_lines()
{
	Q_ASSERT(m_firstIndex >= 0);
	Q_ASSERT(m_firstIndex <= m_subtitle->linesCount());
	Q_ASSERT(m_lastIndex >= 0);
	Q_ASSERT(m_lastIndex <= m_subtitle->linesCount());
	Q_ASSERT(m_firstIndex <= m_lastIndex);
}

RemoveLinesAction::~RemoveLinesAction()
{
	qDeleteAll(m_lines);
}

bool
RemoveLinesAction::mergeWith(const QUndoCommand *command)
{
	const RemoveLinesAction *currentAction = static_cast<const RemoveLinesAction *>(command);
	if(&currentAction->m_subtitle != &m_subtitle)
		return false;

	if(m_lastIndex + 1 == currentAction->m_firstIndex) {
		// currentAction removed lines immediately after ours
		m_lastIndex = currentAction->m_lastIndex;
		while(!currentAction->m_lines.isEmpty())
			m_lines.append(const_cast<RemoveLinesAction *>(currentAction)->m_lines.takeFirst());
		return true;
	}

	if(currentAction->m_lastIndex + 1 == m_firstIndex) {
		// currentAction removed lines immediately before ours
		m_firstIndex = currentAction->m_firstIndex;
		while(!currentAction->m_lines.isEmpty())
			m_lines.prepend(const_cast<RemoveLinesAction *>(currentAction)->m_lines.takeLast());
		return true;
	}

	return false;
}

void
RemoveLinesAction::redo()
{
	emit m_subtitle->linesAboutToBeRemoved(m_firstIndex, m_lastIndex);

	for(int index = m_firstIndex; index <= m_lastIndex; ++index) {
		SubtitleLine *line = m_subtitle->takeAt(m_firstIndex);
		clearLineSubtitle(line);
		m_lines.append(line);
	}

	emit m_subtitle->linesRemoved(m_firstIndex, m_lastIndex);
}

void
RemoveLinesAction::undo()
{
	emit m_subtitle->linesAboutToBeInserted(m_firstIndex, m_lastIndex);

	int insertOffset = 0;

	while(!m_lines.isEmpty()) {
		SubtitleLine *line = m_lines.takeFirst();
		auto pos = m_subtitle->m_lines.cbegin() + m_firstIndex + insertOffset++;
		setLineSubtitle(line);
		m_subtitle->m_lines.insert(pos, line);
	}

	emit m_subtitle->linesInserted(m_firstIndex, m_lastIndex);
}


// *** MoveLineAction
MoveLineAction::MoveLineAction(Subtitle *subtitle, int fromIndex, int toIndex) :
	SubtitleAction(subtitle, UndoStack::Both, i18n("Move Line")),
	m_fromIndex(fromIndex),
	m_toIndex(toIndex < 0 ? subtitle->lastIndex() : toIndex)
{
	Q_ASSERT(m_fromIndex >= 0);
	Q_ASSERT(m_fromIndex <= m_subtitle->linesCount());
	Q_ASSERT(m_toIndex >= 0);
	Q_ASSERT(m_toIndex <= m_subtitle->linesCount());
	Q_ASSERT(m_fromIndex != m_toIndex);
}

MoveLineAction::~MoveLineAction()
{}

bool
MoveLineAction::mergeWith(const QUndoCommand *command)
{
	const MoveLineAction *currentAction = static_cast<const MoveLineAction *>(command);
	if(currentAction->m_subtitle != m_subtitle)
		return false;

	Q_ASSERT(command != this);

	// TODO: FIXME: this and currentAction were swapped in new Qt's implementation, so below code is not working
	// since move is used only when sorting - this will never be called
	if(currentAction->m_toIndex == m_fromIndex) {
		m_fromIndex = currentAction->m_fromIndex;
		return true;
	} else if(m_toIndex - m_fromIndex == 1 || m_fromIndex - m_toIndex == 1) {
		if(currentAction->m_toIndex == m_toIndex) {
			// when the distance between fromIndex and toIndex is 1, the action is the same as if the values were swapped
			m_toIndex = m_fromIndex;
			m_fromIndex = currentAction->m_fromIndex;
			return true;
		}
		if(currentAction->m_toIndex - currentAction->m_fromIndex == 1 || currentAction->m_fromIndex - currentAction->m_toIndex == 1) {
			// same as before, but now we consider inverting the previous action too
			if(currentAction->m_fromIndex == m_toIndex) {
				m_toIndex = m_fromIndex;
				m_fromIndex = currentAction->m_toIndex;
				return true;
			}
		}
	} else if(currentAction->m_toIndex - currentAction->m_fromIndex == 1 || currentAction->m_fromIndex - currentAction->m_toIndex == 1) {
		// again, same as before, but now we consider inverting only the previous action
		if(currentAction->m_fromIndex == m_fromIndex) {
			m_fromIndex = currentAction->m_toIndex;
			return true;
		}
	}

	return false;
}

void
MoveLineAction::redo()
{
	emit m_subtitle->linesAboutToBeRemoved(m_fromIndex, m_fromIndex);
	SubtitleLine *line = m_subtitle->takeAt(m_fromIndex);
	clearLineSubtitle(line);
	emit m_subtitle->linesRemoved(m_fromIndex, m_fromIndex);

	emit m_subtitle->linesAboutToBeInserted(m_toIndex, m_toIndex);
	setLineSubtitle(line);
	m_subtitle->m_lines.insert(m_subtitle->m_lines.cbegin() + m_toIndex, line);
	emit m_subtitle->linesInserted(m_toIndex, m_toIndex);
}

void
MoveLineAction::undo()
{
	emit m_subtitle->linesAboutToBeRemoved(m_toIndex, m_toIndex);
	SubtitleLine *line = m_subtitle->takeAt(m_toIndex);
	clearLineSubtitle(line);
	emit m_subtitle->linesRemoved(m_toIndex, m_toIndex);

	emit m_subtitle->linesAboutToBeInserted(m_fromIndex, m_fromIndex);
	setLineSubtitle(line);
	m_subtitle->m_lines.insert(m_subtitle->m_lines.cbegin() + m_fromIndex, line);
	emit m_subtitle->linesInserted(m_fromIndex, m_fromIndex);
}


// *** SwapLinesTextsAction
SwapLinesTextsAction::SwapLinesTextsAction(Subtitle *subtitle, const RangeList &ranges) :
	SubtitleAction(subtitle, UndoStack::Both, i18n("Swap Texts")),
	m_ranges(ranges)
{}

SwapLinesTextsAction::~SwapLinesTextsAction()
{}

void
SwapLinesTextsAction::redo()
{
	for(SubtitleIterator it(*m_subtitle, m_ranges); it.current(); ++it) {
		SubtitleLine *line = it.current();
		RichDocument *tmp = line->m_primaryDoc;
		line->m_primaryDoc = line->m_secondaryDoc;
		line->m_secondaryDoc = tmp;
		emit line->primaryTextChanged();
		emit line->secondaryTextChanged();
	}
}


// *** ChangeStylesheetAction
EditStylesheetAction::EditStylesheetAction(Subtitle *subtitle, QTextEdit *textEdit)
	: SubtitleAction(subtitle, UndoStack::Primary, i18n("Change stylesheet")),
	  m_stylesheetEdit(textEdit),
	  m_stylesheetDocState(m_stylesheetEdit->document()->availableUndoSteps())
{
}

EditStylesheetAction::~EditStylesheetAction()
{
}

bool
EditStylesheetAction::mergeWith(const QUndoCommand *command)
{
	const EditStylesheetAction *cur = static_cast<const EditStylesheetAction *>(command);
	Q_ASSERT(cur->m_stylesheetEdit == m_stylesheetEdit);
	return cur->m_stylesheetDocState == m_stylesheetDocState;
}

void
EditStylesheetAction::update(const QString &stylesheet)
{
	RichCSS *ss = m_subtitle->m_stylesheet;
	ss->blockSignals(true);
	ss->clear();
	ss->blockSignals(false);
	ss->parse(stylesheet);
}

void
EditStylesheetAction::undo()
{
	const bool prev = m_subtitle->ignoreDocChanges(true);
	while(m_stylesheetEdit->document()->isUndoAvailable() && m_stylesheetEdit->document()->availableUndoSteps() >= m_stylesheetDocState)
		m_stylesheetEdit->undo();
	update(m_stylesheetEdit->document()->toPlainText());
	m_subtitle->ignoreDocChanges(prev);
}

void
EditStylesheetAction::redo()
{
	const bool prev = m_subtitle->ignoreDocChanges(true);
	while(m_stylesheetEdit->document()->isRedoAvailable() && m_stylesheetEdit->document()->availableUndoSteps() < m_stylesheetDocState)
		m_stylesheetEdit->redo();
	update(m_stylesheetEdit->document()->toPlainText());
	m_subtitle->ignoreDocChanges(prev);
}
