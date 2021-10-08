/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCRIPTING_SUBTITLE_H
#define SCRIPTING_SUBTITLE_H

#include "scripting_subtitleline.h"
#include "core/subtitle.h"
#include "core/rangelist.h"

#include <QExplicitlySharedDataPointer>
#include <QObject>

namespace SubtitleComposer {
class Subtitle;

namespace Scripting {
class Subtitle : public QObject
{
	Q_OBJECT

public slots:
/// NOTE: target defaults to an invalid value, which means using and operation
/// defined default value, generally dependent on translationMode value.
	double framesPerSecond() const;
	void setFramesPerSecond(double framesPerSecond);

	bool isEmpty() const;
	int linesCount() const;
	int lastIndex() const;

	QObject * firstLine();
	QObject * lastLine();
	QObject * line(int index);

	void changeFramesPerSecond(double toFramesPerSecond, double fromFramesPerSecond = -1.0);

	SubtitleLine * insertNewLine(int index, bool timeAfter, int target = -1);
	void removeLine(int index, int target = -1);
	void removeLines(QObject *ranges, int target = -1);

	void swapTexts(QObject *ranges);

	void splitLines(QObject *ranges);
	void joinLines(QObject *ranges);

	void shiftLines(QObject *ranges, int msecs);
	void adjustLines(QObject *range, int firstTime, int lastTime);
	void sortLines(QObject *range);

	void applyDurationLimits(QObject *ranges, int minDuration, int maxDuration, bool canOverlap);
	void setMaximumDurations(QObject *ranges);
	void setAutoDurations(QObject *ranges, int msecsPerChar, int msecsPerWord, int msecsPerLine, bool canOverlap, int calculationTarget = -1);

	void fixOverlappingLines(QObject *ranges, int minInterval = 100);

	void fixPunctuation(QObject *ranges, bool spaces, bool quotes, bool englishI, bool ellipsis, int target = -1);

	void lowerCase(QObject *ranges, int target = -1);
	void upperCase(QObject *ranges, int target = -1);
	void titleCase(QObject *ranges, bool lowerFirst, int target = -1);
	void sentenceCase(QObject *ranges, bool lowerFirst, int target = -1);

	void breakLines(QObject *ranges, int minLengthForLineBreak, int target = -1);
	void unbreakTexts(QObject *ranges, int target = -1);
	void simplifyTextWhiteSpace(QObject *ranges, int target = -1);

	void setMarked(QObject *ranges, bool value);

	void clearErrors(QObject *ranges, int errorFlags);

	void checkErrors(QObject *ranges, int errorFlags);

	void recheckErrors(QObject *ranges);

private:
	static SubtitleComposer::RangeList toRangesList(const QObject *object);

	friend class SubtitleModule;

	Subtitle(SubtitleComposer::Subtitle *backend, QObject *parent);

	QExplicitlySharedDataPointer<SubtitleComposer::Subtitle> m_backend;
};
}
}
#endif
