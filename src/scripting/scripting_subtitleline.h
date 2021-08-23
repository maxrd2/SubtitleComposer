/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCRIPTING_SUBTITLELINE_H
#define SCRIPTING_SUBTITLELINE_H

#include "core/subtitleline.h"

#include <QObject>
#include <QString>

namespace SubtitleComposer {
namespace Scripting {
class Subtitle;

class SubtitleLine : public QObject
{
	Q_OBJECT

public slots:
/// NOTE: target defaults to an invalid value, which means using and operation
/// defined default value, generally dependent on translationMode value.
/// Also, setSecondaryText is a noop is ! translationMode
	int number() const;
	int index() const;

	QObject * prevLine() const;
	QObject * nextLine() const;

	int primaryCharacters() const;
	int primaryWords() const;
	int primaryLines() const;

	QObject * primaryText() const;
	void setPrimaryText(const QObject *text);
	QString plainPrimaryText() const;
	void setPlainPrimaryText(const QString &plainText);
	QString richPrimaryText() const;
	void setRichPrimaryText(const QString &richText);

	int secondaryCharacters() const;
	int secondaryWords() const;
	int secondaryLines() const;

	QObject * secondaryText() const;
	void setSecondaryText(const QObject *text);
	QString plainSecondaryText() const;
	void setPlainSecondaryText(const QString &plainText);
	QString richSecondaryText() const;
	void setRichSecondaryText(const QString &richText);

	void breakText(int minLengthForBreak, int target = -1);
	void unbreakText(int target = -1);
	void simplifyTextWhiteSpace(int target = -1);

/// all times or durations are specified in milliseconds

	int showTime() const;
	void setShowTime(int showTime);

	int hideTime() const;
	void setHideTime(int hideTime);

	int durationTime() const;
	void setDurationTime(int durationTime);

	int autoDuration(int msecsPerChar, int msecsPerWord, int msecsPerLine, int calculationTarget = -1);

	void shiftTimes(int mseconds);
	void adjustTimes(int shiftMseconds, double scaleFactor);

	int errorCount() const;
	int errorFlags() const;
	void setErrorFlags(int errorFlags);
	void setErrorFlags(int errorFlags, bool value);

	bool checkEmptyPrimaryText(bool update = true);
	bool checkEmptySecondaryText(bool update = true);
	bool checkUntranslatedText(bool update = true);
	bool checkOverlapsWithNext(bool update = true);

	bool checkMinDuration(int minMsecs, bool update = true);
	bool checkMaxDuration(int maxMsecs, bool update = true);

	bool checkMinDurationPerPrimaryChar(int minMsecsPerChar, bool update = true);
	bool checkMinDurationPerSecondaryChar(int minMsecsPerChar, bool update = true);
	bool checkMaxDurationPerPrimaryChar(int maxMsecsPerChar, bool update = true);
	bool checkMaxDurationPerSecondaryChar(int maxMsecsPerChar, bool update = true);

	bool checkMaxPrimaryChars(int maxCharacters, bool update = true);
	bool checkMaxSecondaryChars(int maxCharacters, bool update = true);
	bool checkMaxPrimaryLines(int maxLines, bool update = true);
	bool checkMaxSecondaryLines(int maxLines, bool update = true);

	bool checkPrimaryUnneededSpaces(bool update = true);
	bool checkSecondaryUnneededSpaces(bool update = true);
	bool checkPrimaryCapitalAfterEllipsis(bool update = true);
	bool checkSecondaryCapitalAfterEllipsis(bool update = true);
	bool checkPrimaryUnneededDash(bool update = true);
	bool checkSecondaryUnneededDash(bool update = true);

	int check(int errorFlagsToCheck, bool update = true);

	bool isRightToLeft() const;

private:
	friend class Subtitle;

	SubtitleLine(SubtitleComposer::SubtitleLine *backend, QObject *parent = 0);

	SubtitleComposer::SubtitleLine *m_backend;
};
}
}
#endif
