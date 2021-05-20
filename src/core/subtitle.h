#ifndef SUBTITLE_H
#define SUBTITLE_H

/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2018 Mladen Milinkovic <max@smoothware.net>
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

#include "core/range.h"
#include "core/rangelist.h"
#include "core/time.h"
#include "core/sstring.h"
#include "core/subtitletarget.h"
#include "helpers/objectref.h"
#include "formatdata.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>

QT_FORWARD_DECLARE_CLASS(QUndoCommand)

namespace SubtitleComposer {
class RichDocument;
class SubtitleLine;
class UndoAction;

class Subtitle : public QObject
{
	Q_OBJECT

	friend class UndoStack;

	friend class SubtitleLine;
	friend class SubtitleIterator;

	friend class UndoAction;

	friend class SubtitleAction;
	friend class SetFramesPerSecondAction;
	friend class InsertLinesAction;
	friend class RemoveLinesAction;
	friend class MoveLineAction;

	friend class SubtitleLineAction;
	friend class SetLinePrimaryTextAction;
	friend class SetLineSecondaryTextAction;
	friend class SetLineTextsAction;
	friend class SetLineShowTimeAction;
	friend class SetLineHideTimeAction;
	friend class SetLineTimesAction;
	friend class SetLineErrorsAction;
	friend class ToggleLineMarkedAction;

	friend class SubtitleCompositeActionExecutor;

	friend class Format;
	friend class InputFormat;

public:
	static double defaultFramesPerSecond();
	static void setDefaultFramesPerSecond(double framesPerSecond);

	Subtitle(double framesPerSecond = defaultFramesPerSecond());
	virtual ~Subtitle();

/// primary data includes primary text, timing information, format data and all errors except secondary only errors
	void setPrimaryData(const Subtitle &from, bool usePrimaryData);
	void clearPrimaryTextData();

/// secondary data includes secondary text and secondary only errors
	void setSecondaryData(const Subtitle &from, bool usePrimaryData);
	void clearSecondaryTextData();

	bool isPrimaryDirty() const;
	void clearPrimaryDirty();

	bool isSecondaryDirty() const;
	void clearSecondaryDirty();

	double framesPerSecond() const;
	void setFramesPerSecond(double framesPerSecond);
	void changeFramesPerSecond(double toFramesPerSecond, double fromFramesPerSecond = -1.0);

	bool isEmpty() const { return m_lines.empty(); }
	int linesCount() const { return m_lines.count(); }
	int lastIndex() const { return m_lines.count() - 1; }

	SubtitleLine * line(int index);
	const SubtitleLine * line(int index) const;

	inline SubtitleLine * firstLine() { return m_lines.isEmpty() ? nullptr : m_lines.first().obj(); }
	inline const SubtitleLine * firstLine() const { return m_lines.isEmpty() ? nullptr : m_lines.first().obj(); }

	inline SubtitleLine * lastLine() { return m_lines.isEmpty() ? nullptr : m_lines.last().obj(); }
	inline const SubtitleLine * lastLine() const { return m_lines.isEmpty() ? nullptr : m_lines.last().obj(); }

	inline int count() const { return m_lines.size(); }
	inline const SubtitleLine * at(const int i) const { return m_lines.at(i).obj(); }
	inline SubtitleLine * at(const int i) { return m_lines.at(i).obj(); }
	inline const SubtitleLine * operator[](const int i) const { return m_lines.at(i).obj(); }
	inline SubtitleLine * operator[](const int i) { return m_lines.at(i).obj(); }

//	inline const QVector<ObjectRef<SubtitleLine>> & allLines() const { return m_lines; }

//	inline const QList<const SubtitleLine *> & anchoredLines() const { return m_anchoredLines; }

	bool hasAnchors() const;
	bool isLineAnchored(int index) const;
	bool isLineAnchored(const SubtitleLine *line) const;
	void toggleLineAnchor(int index);
	void toggleLineAnchor(const SubtitleLine *line);
	void removeAllAnchors();

	void insertLine(SubtitleLine *line, int index = -1);
	void insertLines(const QList<SubtitleLine *> &lines, int index = -1);
	SubtitleLine * insertNewLine(int index, bool timeAfter, SubtitleTarget target);
	void removeLines(const RangeList &ranges, SubtitleTarget target);

	void swapTexts(const RangeList &ranges);

	void splitLines(const RangeList &ranges);
	void joinLines(const RangeList &ranges);

	void shiftAnchoredLine(SubtitleLine *anchoredLine, const Time &newShowTime);

	void shiftLines(const RangeList &ranges, long msecs);
	void adjustLines(const Range &range, long firstTime, long lastTime);
	void sortLines(const Range &range);

	void applyDurationLimits(const RangeList &ranges, const Time &minDuration, const Time &maxDuration, bool canOverlap);
	void setMaximumDurations(const RangeList &ranges);
	void setAutoDurations(const RangeList &ranges, int msecsPerChar, int msecsPerWord, int msecsPerLine, bool canOverlap, SubtitleTarget calculationTarget);

	void fixOverlappingLines(const RangeList &ranges, const Time &minInterval = 100);

	void fixPunctuation(const RangeList &ranges, bool spaces, bool quotes, bool englishI, bool ellipsis, SubtitleTarget target);

	void lowerCase(const RangeList &ranges, SubtitleTarget target);
	void upperCase(const RangeList &ranges, SubtitleTarget target);
	void titleCase(const RangeList &ranges, bool lowerFirst, SubtitleTarget target);
	void sentenceCase(const RangeList &ranges, bool lowerFirst, SubtitleTarget target);

	void breakLines(const RangeList &ranges, unsigned minLengthForLineBreak, SubtitleTarget target);
	void unbreakTexts(const RangeList &ranges, SubtitleTarget target);
	void simplifyTextWhiteSpace(const RangeList &ranges, SubtitleTarget target);

	void syncWithSubtitle(const Subtitle &refSubtitle);
	void appendSubtitle(const Subtitle &srcSubtitle, double shiftMsecsBeforeAppend);
	void splitSubtitle(Subtitle &dstSubtitle, const Time &splitTime, bool shiftSplitLines);

	void toggleStyleFlag(const RangeList &ranges, SString::StyleFlag styleFlag);
	void changeTextColor(const RangeList &ranges, QRgb color);

	void setMarked(const RangeList &ranges, bool value);
	void toggleMarked(const RangeList &ranges);

	void clearErrors(const RangeList &ranges, int errorFlags);
	void checkErrors(const RangeList &ranges, int errorFlags);
	void recheckErrors(const RangeList &ranges);

signals:
	void primaryChanged();
	void secondaryChanged();

	void primaryDirtyStateChanged(bool dirty);
	void secondaryDirtyStateChanged(bool dirty);

	void framesPerSecondChanged(double fps);
	void linesAboutToBeInserted(int firstIndex, int lastIndex);
	void linesInserted(int firstIndex, int lastIndex);
	void linesAboutToBeRemoved(int firstIndex, int lastIndex);
	void linesRemoved(int firstIndex, int lastIndex);

	void compositeActionStart();
	void compositeActionEnd();

	void lineAnchorChanged(const SubtitleLine *line, bool anchored);

/// forwarded line signals
	void linePrimaryTextChanged(SubtitleLine *line);
	void lineSecondaryTextChanged(SubtitleLine *line);
	void lineShowTimeChanged(SubtitleLine *line);
	void lineHideTimeChanged(SubtitleLine *line);
	void lineErrorFlagsChanged(SubtitleLine *line);
	void lineMarkChanged(SubtitleLine *line);

private:
	FormatData * formatData() const;
	void setFormatData(const FormatData *formatData);

	void beginCompositeAction(const QString &title);
	void endCompositeAction();
	void processAction(UndoAction *action);

	void updateState();

	inline int normalizeRangeIndex(int index) const { return index >= m_lines.count() ? m_lines.count() - 1 : index; }

	void setLastValidCachedIndex(int lastValidCachedIndex);

	inline SubtitleLine * takeAt(const int i) { SubtitleLine *s = m_lines.at(i).obj(); m_lines.remove(i); return s; }

private:
	int m_primaryState;
	int m_primaryCleanState;
	int m_secondaryState;
	int m_secondaryCleanState;

	double m_framesPerSecond;
	mutable QVector<ObjectRef<SubtitleLine>> m_lines;
	QList<const SubtitleLine *> m_anchoredLines;

	FormatData *m_formatData;

	static double s_defaultFramesPerSecond;
};

class SubtitleCompositeActionExecutor
{
public:
	SubtitleCompositeActionExecutor(Subtitle &subtitle, const QString &title);
	~SubtitleCompositeActionExecutor();

private:
	Subtitle &m_subtitle;
};
}

#include "subtitleline.h"

#endif
