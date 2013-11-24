#ifndef SCRIPTING_SUBTITLELINE_H
#define SCRIPTING_SUBTITLELINE_H

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../../core/subtitleline.h"

#include <QtCore/QObject>
#include <QtCore/QString>

namespace SubtitleComposer {
namespace Scripting {
class Subtitle;

class SubtitleLine : public QObject
{
	Q_OBJECT

public slots:
/// NOTE: target defaults to an invalid value, which means using and operation
/// defined default value, generaly dependant on translationMode value.
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

	int check(int errorFlagsToCheck, int minDuration, int maxDuration, int minDurationPerChar, int maxDurationPerChar, int maxChars, int maxLines, bool update = true);

private:
	friend class Subtitle;

	SubtitleLine(SubtitleComposer::SubtitleLine *backend, QObject *parent = 0);

	SubtitleComposer::SubtitleLine *m_backend;
};
}
}
#endif
