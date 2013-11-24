#ifndef SUBTITLELINE_H
#define SUBTITLELINE_H

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

#include "action.h"
#include "sstring.h"
#include "time.h"
#include "formatdata.h"

#include <QtCore/QObject>
#include <QtCore/QString>

namespace SubtitleComposer {
class Subtitle;

class SubtitleLine : public QObject
{
	Q_OBJECT

	friend class Subtitle;
	friend class SubtitleAction;
	friend class SwapLinesTextsAction;
	friend class SubtitleLineAction;
	friend class SetLinePrimaryTextAction;
	friend class SetLineSecondaryTextAction;
	friend class SetLineTextsAction;
	friend class SetLineShowTimeAction;
	friend class SetLineHideTimeAction;
	friend class SetLineTimesAction;
	friend class SetLineStyleFlagsAction;
	friend class SetLineErrorsAction;
	friend class ToggleLineMarkedAction;
	friend class Format;

public:
	typedef enum {
		Primary = 0,
		Secondary,
		Both,
		TextTargetSIZE
	} TextTarget;

	typedef enum {
		EmptyPrimaryTextID = 0,         // Empty primary text
		EmptySecondaryTextID,           // Empty secondary text
		MaxPrimaryCharsID,              // Too many characters in primary text
		MaxSecondaryCharsID,            // Too many characters in secondary text
		MaxPrimaryLinesID,              // Too many line breaks in primary text
		MaxSecondaryLinesID,            // Too many line breaks in secondary text
		PrimaryUnneededSpacesID,                // Unnecessary spaces in primary text
		SecondaryUnneededSpacesID,              // Unnecessary spaces in secondary text
		PrimaryUnneededDashID,          // Unnecessary dash (-) in primary text
		SecondaryUnneededDashID,                // Unnecessary dash (-) in secondary text
		PrimaryCapitalAfterEllipsisID,          // Capital letter after ellipsis in primary text
		SecondaryCapitalAfterEllipsisID,                // Capital letter after ellipsis in secondary text
		MinDurationPerPrimaryCharID,            // Too short duration per primary text character
		MinDurationPerSecondaryCharID,          // Too short duration per secondary text character
		MaxDurationPerPrimaryCharID,            // Too long duration per primary text character
		MaxDurationPerSecondaryCharID,          // Too long duration per secondary text character
		MinDurationID,                  // Too short duration
		MaxDurationID,                  // Too long duration
		OverlapsWithNextID,             // Overlaps with next line
		UntranslatedTextID,             // Primary and translation text are the same
		UserMarkID,
		ErrorSIZE,
		ErrorUNKNOWN
	} ErrorID;

	typedef enum {
		EmptyPrimaryText = 0x1 << EmptyPrimaryTextID,
		EmptySecondaryText = 0x1 << EmptySecondaryTextID,
		MaxPrimaryChars = 0x1 << MaxPrimaryCharsID,
		MaxSecondaryChars = 0x1 << MaxSecondaryCharsID,
		MaxPrimaryLines = 0x1 << MaxPrimaryLinesID,
		MaxSecondaryLines = 0x1 << MaxSecondaryLinesID,
		PrimaryUnneededSpaces = 0x1 << PrimaryUnneededSpacesID,
		SecondaryUnneededSpaces = 0x1 << SecondaryUnneededSpacesID,
		PrimaryUnneededDash = 0x1 << PrimaryUnneededDashID,
		SecondaryUnneededDash = 0x1 << SecondaryUnneededDashID,
		PrimaryCapitalAfterEllipsis = 0x1 << PrimaryCapitalAfterEllipsisID,
		SecondaryCapitalAfterEllipsis = 0x1 << SecondaryCapitalAfterEllipsisID,
		MinDurationPerPrimaryChar = 0x1 << MinDurationPerPrimaryCharID,
		MinDurationPerSecondaryChar = 0x1 << MinDurationPerSecondaryCharID,
		MaxDurationPerPrimaryChar = 0x1 << MaxDurationPerPrimaryCharID,
		MaxDurationPerSecondaryChar = 0x1 << MaxDurationPerSecondaryCharID,
		MinDuration = 0x1 << MinDurationID,
		MaxDuration = 0x1 << MaxDurationID,
		OverlapsWithNext = 0x1 << OverlapsWithNextID,
		UntranslatedText = 0x1 << UntranslatedTextID,
		UserMark = 0x1 << UserMarkID,

		PrimaryOnlyErrors = EmptyPrimaryText | MaxPrimaryChars | MaxPrimaryLines | PrimaryUnneededSpaces | PrimaryUnneededDash | PrimaryCapitalAfterEllipsis | MinDurationPerPrimaryChar | MaxDurationPerPrimaryChar,

		SecondaryOnlyErrors = EmptySecondaryText | MaxSecondaryChars | MaxSecondaryLines | SecondaryUnneededSpaces | SecondaryUnneededDash | SecondaryCapitalAfterEllipsis | MinDurationPerSecondaryChar | MaxDurationPerSecondaryChar | UntranslatedText,

		SharedErrors = MinDuration | MaxDuration | OverlapsWithNext | UserMark,

		AllErrors = PrimaryOnlyErrors | SecondaryOnlyErrors | SharedErrors,

		TimesErrors = MinDuration | MaxDuration | MinDurationPerPrimaryChar | MinDurationPerSecondaryChar | MaxDurationPerPrimaryChar | MaxDurationPerSecondaryChar | OverlapsWithNext,

		TextErrors = EmptyPrimaryText | EmptySecondaryText | MaxPrimaryChars | MaxSecondaryChars | MaxPrimaryLines | MaxSecondaryLines | PrimaryUnneededSpaces | SecondaryUnneededSpaces | PrimaryUnneededDash | SecondaryUnneededDash | PrimaryCapitalAfterEllipsis | SecondaryCapitalAfterEllipsis | MinDurationPerPrimaryChar | MinDurationPerSecondaryChar | MaxDurationPerPrimaryChar | MaxDurationPerSecondaryChar | UntranslatedText
	} ErrorFlag;

	static int bitsCount(unsigned int bitFlags);

	static ErrorFlag errorFlag(ErrorID id);
	static ErrorID errorID(ErrorFlag flag);

	static const QString & simpleErrorText(SubtitleLine::ErrorFlag errorFlag);
	static const QString & simpleErrorText(SubtitleLine::ErrorID errorID);

	QString fullErrorText(SubtitleLine::ErrorFlag errorFlag) const;
	QString fullErrorText(SubtitleLine::ErrorID errorID) const;

	explicit SubtitleLine(const SString &pText = SString(), const SString &sText = SString());
	SubtitleLine(const SString &pText, const Time &showTime, const Time &hideTime);
	SubtitleLine(const SString &pText, const SString &sText, const Time &showTime, const Time &hideTime);
	SubtitleLine(const SubtitleLine &line);
	SubtitleLine & operator=(const SubtitleLine &line);
	virtual ~SubtitleLine();

	int number() const;
	int index() const;

	Subtitle * subtitle();
	const Subtitle * subtitle() const;

	SubtitleLine * prevLine();
	SubtitleLine * nextLine();

	const SString & primaryText() const;
	void setPrimaryText(const SString &pText);

	const SString & secondaryText() const;
	void setSecondaryText(const SString &sText);

	void setTexts(const SString &pText, const SString &sText);

	static SString fixPunctuation(const SString &text, bool spaces, bool quotes, bool englishI, bool ellipsis, bool *cont);
	static SString breakText(const SString &text, int minLengthForBreak);
	static QString simplifyTextWhiteSpace(QString text);
	static SString simplifyTextWhiteSpace(SString text);

	void breakText(int minLengthForBreak, TextTarget target);
	void unbreakText(TextTarget target);
	void simplifyTextWhiteSpace(TextTarget target);

	Time showTime() const;
	void setShowTime(const Time &showTime);

	Time hideTime() const;
	void setHideTime(const Time &hideTime);

	Time durationTime() const;
	void setDurationTime(const Time &durationTime);

	void setTimes(const Time &showTime, const Time &hideTime);

	int primaryCharacters() const;
	int primaryWords() const;
	int primaryLines() const;

	int secondaryCharacters() const;
	int secondaryWords() const;
	int secondaryLines() const;

	static Time autoDuration(const QString &text, int msecsPerChar, int msecsPerWord, int msecsPerLine);
	Time autoDuration(int msecsPerChar, int msecsPerWord, int msecsPerLine, TextTarget calculationTarget);

	void shiftTimes(long mseconds);
	void adjustTimes(double shiftMseconds, double scaleFactor);

	int errorFlags() const;
	int errorCount() const;
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

	int check(int errorFlagsToCheck, int minDurationMsecs, int maxDurationMsecs, int minMsecsPerChar, int maxMsecsPerChar, int maxChars, int maxLines, bool update = true);

signals:
	void primaryTextChanged(const SString &text);
	void secondaryTextChanged(const SString &text);
	void showTimeChanged(const Time &showTime);
	void hideTimeChanged(const Time &hideTime);
	void errorFlagsChanged(int errorFlags);

private:
	FormatData * formatData() const;
	void setFormatData(const FormatData *formatData);

	void processAction(Action *action);

private:
	Subtitle *m_subtitle;
	SString m_primaryText;
	SString m_secondaryText;
	Time m_showTime;
	Time m_hideTime;
	int m_errorFlags;

	int m_cachedIndex;

	FormatData *m_formatData;
};
}
#endif
