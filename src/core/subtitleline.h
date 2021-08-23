/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SUBTITLELINE_H
#define SUBTITLELINE_H

#include "core/time.h"
#include "core/formatdata.h"
#include "core/subtitletarget.h"
#include "helpers/objectref.h"

#include <QExplicitlySharedDataPointer>
#include <QObject>
#include <QString>

class QUndoCommand;

namespace SubtitleComposer {
class Subtitle;
class SString;
class RichDocument;
class UndoAction;

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
	friend class ObjectRef<SubtitleLine>;

public:
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

	SubtitleLine();
	SubtitleLine(const Time &showTime, const Time &hideTime);
	SubtitleLine(const SubtitleLine &line) = delete;
	SubtitleLine & operator=(const SubtitleLine &line) = delete;
	virtual ~SubtitleLine();

	int number() const;
	int index() const;

	inline Subtitle * subtitle() { return m_subtitle.data(); }
	inline const Subtitle * subtitle() const { return m_subtitle.constData(); }

	inline SubtitleLine * prevLine() const;
	inline SubtitleLine * nextLine() const;

	inline RichDocument * primaryDoc() const { return m_primaryDoc; }
	inline RichDocument * secondaryDoc() const { return m_secondaryDoc; }

	void breakText(int minBreakLength, SubtitleTarget target);
	void unbreakText(SubtitleTarget target);
	void simplifyTextWhiteSpace(SubtitleTarget target);

	inline Time showTime() const { return m_showTime; }
	void setShowTime(const Time &showTime);

	inline Time hideTime() const { return m_hideTime; }
	void setHideTime(const Time &hideTime);

	inline double duration() const { return m_hideTime.toMillis() - m_showTime.toMillis(); }
	inline Time durationTime() const { return Time(duration()); }
	inline void setDurationTime(const Time &durationTime) { setHideTime(m_showTime + durationTime); }
	QColor durationColor(const QColor &textColor, bool usePrimary=true);

	inline Time pauseTime() const { const SubtitleLine *p = prevLine(); return Time(showTime().toMillis() - (p ? p->hideTime().toMillis() : 0.)); }

	void setTimes(const Time &showTime, const Time &hideTime);

	inline bool containsTime(const Time &time) const { return m_showTime <= time && time <= m_hideTime; }
	inline bool intersectsTimespan(const Time &start, const Time &end) const { return m_showTime <= end && start <= m_hideTime; }

	int primaryCharacters() const;
	int primaryWords() const;
	int primaryLines() const;

	int secondaryCharacters() const;
	int secondaryWords() const;
	int secondaryLines() const;

	static Time autoDuration(const QString &text, int msecsPerChar, int msecsPerWord, int msecsPerLine);
	Time autoDuration(int msecsPerChar, int msecsPerWord, int msecsPerLine, SubtitleTarget calculationTarget);

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

	int check(int errorFlagsToCheck, bool update = true);

signals:
	void primaryTextChanged();
	void secondaryTextChanged();
	void showTimeChanged(const Time &showTime);
	void hideTimeChanged(const Time &hideTime);
	void errorFlagsChanged(int errorFlags);

private:
	FormatData * formatData() const;
	void setFormatData(const FormatData *formatData);

	void processAction(UndoAction *action);
	void processShowTimeSort(const Time &showTime);

	void setPrimaryDoc(RichDocument *doc);
	void setSecondaryDoc(RichDocument *doc);
	void setTexts(RichDocument *pText, RichDocument *sText);
	void primaryDocumentChanged();
	void secondaryDocumentChanged();

	void setupSignals();

	inline bool ignoreDocChanges(bool ignore) {
		bool r = m_ignoreDocChanges;
		m_ignoreDocChanges = ignore;
		return r;
	}

private:
	QExplicitlySharedDataPointer<Subtitle> m_subtitle;
	RichDocument *m_primaryDoc;
	RichDocument *m_secondaryDoc;
	Time m_showTime;
	Time m_hideTime;
	int m_errorFlags;
	bool m_ignoreDocChanges = false;

	FormatData *m_formatData;

	mutable ObjectRef<SubtitleLine> *m_ref = nullptr;
	const QVector<ObjectRef<SubtitleLine>> * refContainer();
};
}

#include "core/subtitle.h"

namespace SubtitleComposer {

inline SubtitleLine *
SubtitleLine::prevLine() const
{
	return m_subtitle ? m_subtitle->line(index() - 1) : nullptr;
}

inline SubtitleLine *
SubtitleLine::nextLine() const
{
	return m_subtitle ? m_subtitle->line(index() + 1) : nullptr;
}

}

#endif
