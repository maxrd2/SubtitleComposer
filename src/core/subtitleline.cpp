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

#include "core/richdocument.h"
#include "core/subtitleline.h"
#include "core/undo/subtitlelineactions.h"
#include "core/undo/subtitleactions.h"
#include "helpers/common.h"
#include "scconfig.h"

#include <QRegularExpression>

#include <KLocalizedString>

#include <math.h>

using namespace SubtitleComposer;


/// "magic" code taken from https://tekpool.wordpress.com/category/bit-count/
int
SubtitleLine::bitsCount(unsigned int u)
{
	unsigned int uCount;
	uCount = u - ((u >> 1) & 033333333333) - ((u >> 2) & 011111111111);
	return ((uCount + (uCount >> 3)) & 030707070707) % 63;
}

SubtitleLine::ErrorFlag
SubtitleLine::errorFlag(SubtitleLine::ErrorID id)
{
	if(id < 0 || id >= ErrorSIZE)
		return static_cast<ErrorFlag>(0);

	return static_cast<ErrorFlag>(0x1 << id);
}

SubtitleLine::ErrorID
SubtitleLine::errorID(SubtitleLine::ErrorFlag flag)
{
	if(flag < 1)
		return ErrorUNKNOWN;

	int id = static_cast<int>(log2(static_cast<int>(flag)));
	return id < ErrorSIZE ? static_cast<ErrorID>(id) : ErrorUNKNOWN;
}

/// ERRORS DESCRIPTIONS
/// ===================

const QString &
SubtitleLine::simpleErrorText(SubtitleLine::ErrorFlag errorFlag)
{
	return simpleErrorText(errorID(errorFlag));
}

const QString &
SubtitleLine::simpleErrorText(SubtitleLine::ErrorID errorID)
{
	static const QString empty;
	static QString texts[ErrorSIZE];

	if(texts[EmptyPrimaryTextID].isEmpty()) {
		texts[EmptyPrimaryTextID] = i18n("Empty text");
		texts[EmptySecondaryTextID] = texts[EmptySecondaryTextID - 1];
		texts[MaxPrimaryCharsID] = i18n("Too many characters");
		texts[MaxSecondaryCharsID] = texts[MaxSecondaryCharsID - 1];
		texts[MaxPrimaryLinesID] = i18n("Too many lines");
		texts[MaxSecondaryLinesID] = texts[MaxSecondaryLinesID - 1];
		texts[PrimaryUnneededSpacesID] = i18n("Unnecessary white space");
		texts[SecondaryUnneededSpacesID] = texts[SecondaryUnneededSpacesID - 1];
		texts[PrimaryUnneededDashID] = i18n("Unnecessary dash");
		texts[SecondaryUnneededDashID] = texts[SecondaryUnneededDashID - 1];
		texts[PrimaryCapitalAfterEllipsisID] = i18n("Capital letter after continuation ellipsis");
		texts[SecondaryCapitalAfterEllipsisID] = texts[SecondaryCapitalAfterEllipsisID - 1];
		texts[MaxDurationPerPrimaryCharID] = i18n("Too long duration per character");
		texts[MaxDurationPerSecondaryCharID] = texts[MaxDurationPerSecondaryCharID - 1];
		texts[MinDurationPerPrimaryCharID] = i18n("Too short duration per character");
		texts[MinDurationPerSecondaryCharID] = texts[MinDurationPerSecondaryCharID - 1];
		texts[MaxDurationID] = i18n("Too long duration");
		texts[MinDurationID] = i18n("Too short duration");
		texts[OverlapsWithNextID] = i18n("Overlapping times");
		texts[UntranslatedTextID] = i18n("Untranslated text");
		texts[UserMarkID] = i18n("User mark");
	}

	return (errorID < 0 || errorID >= ErrorSIZE) ? empty : texts[errorID];
}

QString
SubtitleLine::fullErrorText(SubtitleLine::ErrorFlag errorFlag) const
{
	return fullErrorText(errorID(errorFlag));
}

QString
SubtitleLine::fullErrorText(SubtitleLine::ErrorID errorID) const
{
	if(!(m_errorFlags & (0x1 << errorID)))
		return QString();

	switch(errorID) {
	case EmptyPrimaryTextID: return i18n("Has no primary text.");
	case EmptySecondaryTextID: return i18n("Has no translation text.");
	case MaxPrimaryCharsID: return i18np("Has too many characters in primary text (1 char).", "Has too many characters in primary text (%1 chars).", primaryCharacters());
	case MaxSecondaryCharsID: return i18np("Has too many characters in translation text (1 char).", "Has too many characters in translation text (%1 chars).", secondaryCharacters());
	case MaxPrimaryLinesID: return i18np("Has too many lines in primary text (1 line).", "Has too many lines in primary text (%1 lines).", primaryLines());
	case MaxSecondaryLinesID: return i18np("Has too many lines in translation text (1 line).", "Has too many lines in translation text (%1 lines).", secondaryLines());
	case PrimaryUnneededSpacesID: return i18n("Has unnecessary spaces in primary text.");
	case SecondaryUnneededSpacesID: return i18n("Has unnecessary spaces in translation text.");
	case PrimaryUnneededDashID: return i18n("Has unnecessary dash in primary text.");
	case SecondaryUnneededDashID: return i18n("Has unnecessary dash in translation text.");
	case PrimaryCapitalAfterEllipsisID: return i18n("Has capital letter after line continuation ellipsis in primary text.");
	case SecondaryCapitalAfterEllipsisID: return i18n("Has capital letter after line continuation ellipsis in translation text.");
	case MaxDurationPerPrimaryCharID: return i18np("Has too long duration per character in primary text (1 msec/char).", "Has too long duration per character in primary text (%1 msecs/char).", durationTime().toMillis() / primaryCharacters());
	case MaxDurationPerSecondaryCharID: return i18np("Has too long duration per character in translation text (1 msec/char).", "Has too long duration per character in translation text (%1 msecs/char).", durationTime().toMillis() / secondaryCharacters());
	case MinDurationPerPrimaryCharID: return i18np("Has too short duration per character in primary text (1 msec/char).", "Has too short duration per character in primary text (%1 msecs/char).", durationTime().toMillis() / primaryCharacters());
	case MinDurationPerSecondaryCharID: return i18np("Has too short duration per character in translation text (1 msec/char).", "Has too short duration per character in translation text (%1 msecs/char).", durationTime().toMillis() / secondaryCharacters());
	case MaxDurationID: return i18np("Has too long duration (1 msec).", "Has too long duration (%1 msecs).", durationTime().toMillis());
	case MinDurationID: return i18np("Has too short duration (1 msec).", "Has too short duration (%1 msecs).", durationTime().toMillis());
	case OverlapsWithNextID: return i18n("Overlaps with next line.");
	case UntranslatedTextID: return i18n("Has untranslated text.");
	case UserMarkID: return i18n("Has user mark.");
	default: return QString();
	}
}

void
SubtitleLine::setupSignals()
{
	connect(m_primaryDoc, &RichDocument::contentsChanged, this, &SubtitleLine::primaryDocumentChanged);
	connect(m_secondaryDoc, &RichDocument::contentsChanged, this, &SubtitleLine::secondaryDocumentChanged);

	QObject::connect(this, &SubtitleLine::primaryTextChanged, [this](){
		if(subtitle()) emit subtitle()->linePrimaryTextChanged(this);
	});
	QObject::connect(this, &SubtitleLine::secondaryTextChanged, [this](){
		if(subtitle()) emit subtitle()->lineSecondaryTextChanged(this);
	});
	QObject::connect(this, &SubtitleLine::showTimeChanged, [this](){
		if(subtitle()) emit subtitle()->lineShowTimeChanged(this);
	});
	QObject::connect(this, &SubtitleLine::hideTimeChanged, [this](){
		if(subtitle()) emit subtitle()->lineHideTimeChanged(this);
	});
}

SubtitleLine::SubtitleLine()
	: QObject(),
	  m_subtitle(nullptr),
	  m_primaryDoc(new RichDocument(this)),
	  m_secondaryDoc(new RichDocument(this)),
	  m_showTime(0.0),
	  m_hideTime(0.0),
	  m_errorFlags(0),
	  m_formatData(nullptr)
{
	setupSignals();
}

SubtitleLine::SubtitleLine(const Time &showTime, const Time &hideTime)
	: QObject(),
	  m_subtitle(nullptr),
	  m_primaryDoc(new RichDocument(this)),
	  m_secondaryDoc(new RichDocument(this)),
	  m_showTime(showTime),
	  m_hideTime(hideTime),
	  m_errorFlags(0),
	  m_formatData(nullptr)
{
	setupSignals();
}

//SubtitleLine::SubtitleLine(const SubtitleLine &line)
//	: QObject(),
//	  m_subtitle(nullptr),
//	  m_primaryText(line.m_primaryText),
//	  m_secondaryText(line.m_secondaryText),
//	  m_showTime(line.m_showTime),
//	  m_hideTime(line.m_hideTime),
//	  m_errorFlags(line.m_errorFlags),
//	  m_formatData(nullptr)
//{}

//SubtitleLine &
//SubtitleLine::operator=(const SubtitleLine &line)
//{
//	if(this == &line)
//		return *this;

//	m_primaryText = line.m_primaryText;
//	m_secondaryText = line.m_secondaryText;
//	m_showTime = line.m_showTime;
//	m_hideTime = line.m_hideTime;
//	m_errorFlags = line.m_errorFlags;

//	return *this;
//}

SubtitleLine::~SubtitleLine()
{
	delete m_formatData;
}

FormatData *
SubtitleLine::formatData() const
{
	return m_formatData;
}

void
SubtitleLine::setFormatData(const FormatData *formatData)
{
	delete m_formatData;

	m_formatData = formatData ? new FormatData(*formatData) : NULL;
}

int
SubtitleLine::number() const
{
	return index() + 1;
}

int
SubtitleLine::index() const
{
	if(!m_subtitle)
		return -1;

	Q_ASSERT(m_ref != nullptr);
	Q_ASSERT(m_ref->m_obj == this);

	const int index = m_ref - m_subtitle->m_lines.constData();

	if(index < 0 || index >= m_subtitle->count()) {
		// this should never happen... unless ObjectRef is declared as Q_MOVABLE_TYPE or QVector
		// decides to stop calling constructors
		qWarning() << "SubtitleLine::index() WARNING: m_ref doesn't not belong to cotainer.";
		for(int i = 0, n = m_subtitle->count(); i < n; i++) {
			if(this == m_subtitle->at(i)) {
				m_ref = &m_subtitle->m_lines[i];
				return i;
			}
		}
		qWarning() << "SubtitleLine::index() WARNING: container doesn't contain the line.";
		return -1;
	}

	return index;
}

void
SubtitleLine::primaryDocumentChanged()
{
	processAction(new SetLinePrimaryTextAction(*this, m_primaryDoc));
}

void
SubtitleLine::secondaryDocumentChanged()
{
	processAction(new SetLineSecondaryTextAction(*this, m_secondaryDoc));
}

void
SubtitleLine::setPrimaryDoc(RichDocument *doc)
{
	if(m_primaryDoc)
		disconnect(m_primaryDoc, &RichDocument::contentsChanged, this, &SubtitleLine::primaryDocumentChanged);
	m_primaryDoc = doc;
	m_primaryDoc->setParent(this);
	connect(m_primaryDoc, &RichDocument::contentsChanged, this, &SubtitleLine::primaryDocumentChanged);
}

void
SubtitleLine::setSecondaryDoc(RichDocument *doc)
{
	if(m_secondaryDoc)
		disconnect(m_secondaryDoc, &RichDocument::contentsChanged, this, &SubtitleLine::secondaryDocumentChanged);
	m_secondaryDoc = doc;
	m_secondaryDoc->setParent(this);
	connect(m_secondaryDoc, &RichDocument::contentsChanged, this, &SubtitleLine::secondaryDocumentChanged);
}

void
SubtitleLine::setTexts(RichDocument *pText, RichDocument *sText)
{
	setPrimaryDoc(pText);
	setSecondaryDoc(sText);
}

void
SubtitleLine::breakText(int minBreakLength, SubtitleTarget target)
{
	switch(target) {
	case Primary:
		m_primaryDoc->breakText(minBreakLength);
		break;
	case Secondary:
		m_secondaryDoc->breakText(minBreakLength);
		break;
	case Both:
		m_primaryDoc->breakText(minBreakLength);
		m_secondaryDoc->breakText(minBreakLength);
		break;
	default:
		break;
	}
}

void
SubtitleLine::unbreakText(SubtitleTarget target)
{
	switch(target) {
	case Primary:
		m_primaryDoc->replace('\n', ' ');
		break;
	case Secondary:
		m_secondaryDoc->replace('\n', ' ');
		break;
	case Both:
		m_primaryDoc->replace('\n', ' ');
		m_secondaryDoc->replace('\n', ' ');
		break;
	default:
		break;
	}
}

void
SubtitleLine::simplifyTextWhiteSpace(SubtitleTarget target)
{
	switch(target) {
	case Primary:
		m_primaryDoc->cleanupSpaces();
		break;
	case Secondary:
		m_secondaryDoc->cleanupSpaces();
		break;
	case Both:
		m_primaryDoc->cleanupSpaces();
		m_secondaryDoc->cleanupSpaces();
		break;
	default:
		break;
	}
}

void
SubtitleLine::processShowTimeSort(const Time &showTime)
{
	if(!m_subtitle)
		return;

	const int curIndex = index();
	const int maxIndex = m_subtitle->linesCount() - 1;
	int newIndex = curIndex;

	while(newIndex < maxIndex && m_subtitle->line(newIndex + 1)->m_showTime < showTime)
		newIndex++;

	while(newIndex > 0 && m_subtitle->line(newIndex - 1)->m_showTime > showTime)
		newIndex--;

	if(curIndex != newIndex)
		processAction(new MoveLineAction(*m_subtitle, curIndex, newIndex));
}

void
SubtitleLine::setShowTime(const Time &showTime, bool safe/*=false*/)
{
	if(m_showTime == showTime)
		return;

	if(m_subtitle)
		m_subtitle->beginCompositeAction(i18n("Set Line Show Time"));

	if(m_subtitle && m_subtitle->isLineAnchored(this)) {
		m_subtitle->shiftAnchoredLine(this, showTime);
	} else {
		if(safe && showTime > m_hideTime) {
			setTimes(showTime, showTime);
		} else {
			processShowTimeSort(showTime);
			processAction(new SetLineShowTimeAction(*this, showTime));
		}
	}

	if(m_subtitle)
		m_subtitle->endCompositeAction();
}

void
SubtitleLine::setHideTime(const Time &hideTime, bool safe/*=false*/)
{
	if(m_hideTime == hideTime)
		return;

	if(m_subtitle)
		m_subtitle->beginCompositeAction(i18n("Set Line Hide Time"));

	if(safe && m_showTime > hideTime)
		setTimes(hideTime, m_showTime);
	else
		processAction(new SetLineHideTimeAction(*this, hideTime));

	if(m_subtitle)
		m_subtitle->endCompositeAction();
}

QColor
SubtitleLine::durationColor(const QColor &textColor, bool usePrimary)
{
	const int textLen = (usePrimary ? primaryDoc() : secondaryDoc())->length();
	const int minD = textLen * SCConfig::minDurationPerCharacter();
	const int maxD = textLen * SCConfig::maxDurationPerCharacter();
	const int avgD = textLen * SCConfig::idealDurationPerCharacter();
	const int curD = durationTime().toMillis();
	if(curD < avgD) {
		// duration is too short - mix red
		const int r = qMin(255, 255 * (curD - avgD) / qMin(minD - avgD, -1));
		return QColor(
			textColor.red() * (255 - r) / 255 + r,
			textColor.green() * (255 - r) / 255,
			textColor.blue() * (255 - r) / 255,
			textColor.alpha());
	}
	if(curD > avgD) {
		// duration is too long - mix blue
		const int r = qMin(255, 255 * (curD - avgD) / qMax(maxD - avgD, 1));
		return QColor(
			textColor.red() * (255 - r) / 255,
			textColor.green() * (255 - r) / 255,
			textColor.blue() * (255 - r) / 255 + r,
			textColor.alpha());
	}
	return textColor;
}

void
SubtitleLine::setTimes(const Time &showTime, const Time &hideTime)
{
	if(m_showTime == showTime && m_hideTime == hideTime)
		return;

	if(m_subtitle)
		m_subtitle->beginCompositeAction(i18n("Set Line Times"));

	if(m_subtitle && m_subtitle->isLineAnchored(this)) {
		m_subtitle->shiftAnchoredLine(this, showTime);
	} else {
		processShowTimeSort(showTime);
		processAction(new SetLineTimesAction(*this, showTime, hideTime));
	}

	if(m_subtitle)
		m_subtitle->endCompositeAction();
}

int
SubtitleLine::primaryCharacters() const
{
	return m_primaryDoc->toPlainText().simplified().length();
}

int
SubtitleLine::primaryWords() const
{
	QString text(m_primaryDoc->toPlainText().simplified());
	return text.length() ? text.count(' ') + 1 : 0;
}

int
SubtitleLine::primaryLines() const
{
	SString text(m_primaryDoc->toPlainText());
	text.simplifyWhiteSpace();
	return text.isEmpty() ? 0 : text.count('\n') + 1;
}

int
SubtitleLine::secondaryCharacters() const
{
	return m_secondaryDoc->toPlainText().simplified().length();
}

int
SubtitleLine::secondaryWords() const
{
	QString text(m_secondaryDoc->toPlainText().simplified());
	return text.length() ? text.count(' ') + 1 : 0;
}

int
SubtitleLine::secondaryLines() const
{
	SString text(m_secondaryDoc->toPlainText());
	text.simplifyWhiteSpace();
	return text.isEmpty() ? 0 : text.count('\n') + 1;
}

Time
SubtitleLine::autoDuration(const QString &t, int msecsPerChar, int msecsPerWord, int msecsPerLine)
{
	Q_ASSERT(msecsPerChar >= 0);
	Q_ASSERT(msecsPerWord >= 0);
	Q_ASSERT(msecsPerLine >= 0);

	SString text(t);
	text.simplifyWhiteSpace();
	if(text.isEmpty())
		return 0;

	int chars = text.length();
	int lines = text.count('\n') + 1;
	int words = text.count(' ') + lines;

	return chars * msecsPerChar + words * msecsPerWord + lines * msecsPerLine;
}

Time
SubtitleLine::autoDuration(int msecsPerChar, int msecsPerWord, int msecsPerLine, SubtitleTarget calculationTarget)
{
	switch(calculationTarget) {
	case Secondary:
		return autoDuration(m_secondaryDoc->toPlainText(), msecsPerChar, msecsPerWord, msecsPerLine);
	case Both: {
		Time primary = autoDuration(m_primaryDoc->toPlainText(), msecsPerChar, msecsPerWord, msecsPerLine);
		Time secondary = autoDuration(m_secondaryDoc->toPlainText(), msecsPerChar, msecsPerWord, msecsPerLine);
		return primary > secondary ? primary : secondary;
	}
	case Primary:
	default:
		return autoDuration(m_primaryDoc->toPlainText(), msecsPerChar, msecsPerWord, msecsPerLine);
	}
}

void
SubtitleLine::shiftTimes(long mseconds)
{
	if(!mseconds)
		return;

	processAction(new SetLineTimesAction(*this, m_showTime.shifted(mseconds), m_hideTime.shifted(mseconds), i18n("Shift Line Times")));
}

void
SubtitleLine::adjustTimes(double shiftMseconds, double scaleFactor)
{
	if(shiftMseconds || scaleFactor != 1.0)
		processAction(new SetLineTimesAction(*this, m_showTime.adjusted(shiftMseconds, scaleFactor), m_hideTime.adjusted(shiftMseconds, scaleFactor), i18n("Adjust Line Times")));
}

/// ERRORS

int
SubtitleLine::errorFlags() const
{
	return m_errorFlags;
}

int
SubtitleLine::errorCount() const
{
	return bitsCount(m_errorFlags);
}

void
SubtitleLine::setErrorFlags(int errorFlags)
{
	if(m_errorFlags != errorFlags)
		processAction(new SetLineErrorsAction(*this, errorFlags));
}

void
SubtitleLine::setErrorFlags(int errorFlags, bool value)
{
	setErrorFlags(value ? (m_errorFlags | errorFlags) : (m_errorFlags & ~errorFlags));
}

bool
SubtitleLine::checkEmptyPrimaryText(bool update)
{
	static const QRegExp emptyTextRegExp("^\\s*$");

	bool error = m_primaryDoc->isEmpty() || m_primaryDoc->toPlainText().indexOf(emptyTextRegExp) != -1;

	if(update)
		setErrorFlags(EmptyPrimaryText, error);

	return error;
}

bool
SubtitleLine::checkEmptySecondaryText(bool update)
{
	static const QRegExp emptyTextRegExp("^\\s*$");

	bool error = m_secondaryDoc->isEmpty() || m_secondaryDoc->toPlainText().indexOf(emptyTextRegExp) != -1;

	if(update)
		setErrorFlags(EmptySecondaryText, error);

	return error;
}

bool
SubtitleLine::checkUntranslatedText(bool update)
{
	bool error = m_primaryDoc->toPlainText() == m_secondaryDoc->toPlainText();

	if(update)
		setErrorFlags(UntranslatedText, error);

	return error;
}

bool
SubtitleLine::checkOverlapsWithNext(bool update)
{
	SubtitleLine *nextLine = this->nextLine();
	bool error = nextLine && nextLine->m_showTime <= m_hideTime;

	if(update)
		setErrorFlags(OverlapsWithNext, error);

	return error;
}

bool
SubtitleLine::checkMinDuration(int minMsecs, bool update)
{
	Q_ASSERT(minMsecs >= 0);

	bool error = durationTime().toMillis() < minMsecs;

	if(update)
		setErrorFlags(MinDuration, error);

	return error;
}

bool
SubtitleLine::checkMaxDuration(int maxMsecs, bool update)
{
	Q_ASSERT(maxMsecs >= 0);

	bool error = durationTime().toMillis() > maxMsecs;

	if(update)
		setErrorFlags(MaxDuration, error);

	return error;
}

bool
SubtitleLine::checkMinDurationPerPrimaryChar(int minMsecsPerChar, bool update)
{
	Q_ASSERT(minMsecsPerChar >= 0);

	int characters = primaryCharacters();
	bool error = characters ? (durationTime().toMillis() / characters < minMsecsPerChar) : false;

	if(update)
		setErrorFlags(MinDurationPerPrimaryChar, error);

	return error;
}

bool
SubtitleLine::checkMinDurationPerSecondaryChar(int minMsecsPerChar, bool update)
{
	Q_ASSERT(minMsecsPerChar >= 0);

	int characters = secondaryCharacters();
	bool error = characters ? (durationTime().toMillis() / characters < minMsecsPerChar) : false;

	if(update)
		setErrorFlags(MinDurationPerSecondaryChar, error);

	return error;
}

bool
SubtitleLine::checkMaxDurationPerPrimaryChar(int maxMsecsPerChar, bool update)
{
	Q_ASSERT(maxMsecsPerChar >= 0);

	int characters = primaryCharacters();
	bool error = characters ? (durationTime().toMillis() / characters > maxMsecsPerChar) : false;

	if(update)
		setErrorFlags(MaxDurationPerPrimaryChar, error);

	return error;
}

bool
SubtitleLine::checkMaxDurationPerSecondaryChar(int maxMsecsPerChar, bool update)
{
	Q_ASSERT(maxMsecsPerChar >= 0);

	int characters = secondaryCharacters();
	bool error = characters ? (durationTime().toMillis() / characters > maxMsecsPerChar) : false;

	if(update)
		setErrorFlags(MaxDurationPerSecondaryChar, error);

	return error;
}

bool
SubtitleLine::checkMaxPrimaryChars(int maxCharacters, bool update)
{
	Q_ASSERT(maxCharacters >= 0);

	bool error = primaryCharacters() > maxCharacters;

	if(update)
		setErrorFlags(MaxPrimaryChars, error);

	return error;
}

bool
SubtitleLine::checkMaxSecondaryChars(int maxCharacters, bool update)
{
	Q_ASSERT(maxCharacters >= 0);

	bool error = secondaryCharacters() > maxCharacters;

	if(update)
		setErrorFlags(MaxSecondaryChars, error);

	return error;
}

bool
SubtitleLine::checkMaxPrimaryLines(int maxLines, bool update)
{
	Q_ASSERT(maxLines >= 0);

	bool error = primaryLines() > maxLines;

	if(update)
		setErrorFlags(MaxPrimaryLines, error);

	return error;
}

bool
SubtitleLine::checkMaxSecondaryLines(int maxLines, bool update)
{
	Q_ASSERT(maxLines >= 0);

	bool error = secondaryLines() > maxLines;

	if(update)
		setErrorFlags(MaxSecondaryLines, error);

	return error;
}

bool
SubtitleLine::checkPrimaryUnneededSpaces(bool update)
{
	static const QRegularExpression unneededSpaceRegExp("(^\\s|\\s$|¿\\s|¡\\s|\\s\\s|\\s!|\\s\\?|\\s:|\\s;|\\s,|\\s\\.)");

	bool error = m_primaryDoc->indexOf(unneededSpaceRegExp) != -1;

	if(update)
		setErrorFlags(PrimaryUnneededSpaces, error);

	return error;
}

bool
SubtitleLine::checkSecondaryUnneededSpaces(bool update)
{
	static const QRegularExpression unneededSpaceRegExp("(^\\s|\\s$|¿\\s|¡\\s|\\s\\s|\\s!|\\s\\?|\\s:|\\s;|\\s,|\\s\\.)");

	bool error = m_secondaryDoc->indexOf(unneededSpaceRegExp) != -1;

	if(update)
		setErrorFlags(SecondaryUnneededSpaces, error);

	return error;
}

bool
SubtitleLine::checkPrimaryCapitalAfterEllipsis(bool update)
{
	static const QRegExp capitalAfterEllipsisRegExp("^\\s*\\.\\.\\.[¡¿\\.,;\\(\\[\\{\"'\\s]*");

	const QString text = m_primaryDoc->toPlainText();
	bool error = capitalAfterEllipsisRegExp.indexIn(text, QRegExp::CaretAtZero) != -1;
	if(error) {
		QChar chr = text.at(capitalAfterEllipsisRegExp.matchedLength());
		error = chr.isLetter() && chr == chr.toUpper();
	}

	if(update)
		setErrorFlags(PrimaryCapitalAfterEllipsis, error);

	return error;
}

bool
SubtitleLine::checkSecondaryCapitalAfterEllipsis(bool update)
{
	static const QRegExp capitalAfterEllipsisRegExp("^\\s*\\.\\.\\.[¡¿\\.,;\\(\\[\\{\"'\\s]*");

	const QString text = m_secondaryDoc->toPlainText();
	bool error = capitalAfterEllipsisRegExp.indexIn(text, QRegExp::CaretAtZero) != -1;
	if(error) {
		QChar chr = text.at(capitalAfterEllipsisRegExp.matchedLength());
		error = chr.isLetter() && chr == chr.toUpper();
	}

	if(update)
		setErrorFlags(SecondaryCapitalAfterEllipsis, error);

	return error;
}

bool
SubtitleLine::checkPrimaryUnneededDash(bool update)
{
	static const QRegExp unneededDashRegExp("(^|\n)\\s*-[^-]");

	bool error = m_primaryDoc->toPlainText().count(unneededDashRegExp) == 1;

	if(update)
		setErrorFlags(PrimaryUnneededDash, error);

	return error;
}

bool
SubtitleLine::checkSecondaryUnneededDash(bool update)
{
	static const QRegExp unneededDashRegExp("(^|\n)\\s*-[^-]");

	bool error = m_secondaryDoc->toPlainText().count(unneededDashRegExp) == 1;

	if(update)
		setErrorFlags(SecondaryUnneededDash, error);

	return error;
}

int
SubtitleLine::check(int errorFlagsToCheck, bool update)
{
	int lineErrorFlags = m_errorFlags & ~errorFlagsToCheck; // clear the flags we're going to (re)check

	if(errorFlagsToCheck & EmptyPrimaryText)
		if(checkEmptyPrimaryText(false))
			lineErrorFlags |= EmptyPrimaryText;

	if(errorFlagsToCheck & EmptySecondaryText)
		if(checkEmptySecondaryText(false))
			lineErrorFlags |= EmptySecondaryText;

	if(errorFlagsToCheck & OverlapsWithNext)
		if(checkOverlapsWithNext(false))
			lineErrorFlags |= OverlapsWithNext;

	if(errorFlagsToCheck & UntranslatedText)
		if(checkUntranslatedText(false))
			lineErrorFlags |= UntranslatedText;

	if(errorFlagsToCheck & MaxDuration)
		if(checkMaxDuration(SCConfig::maxDuration(), false))
			lineErrorFlags |= MaxDuration;

	if(errorFlagsToCheck & MinDuration)
		if(checkMinDuration(SCConfig::minDuration(), false))
			lineErrorFlags |= MinDuration;

	if(errorFlagsToCheck & MaxDurationPerPrimaryChar)
		if(checkMaxDurationPerPrimaryChar(SCConfig::maxDurationPerCharacter(), false))
			lineErrorFlags |= MaxDurationPerPrimaryChar;

	if(errorFlagsToCheck & MaxDurationPerSecondaryChar)
		if(checkMaxDurationPerSecondaryChar(SCConfig::maxDurationPerCharacter(), false))
			lineErrorFlags |= MaxDurationPerSecondaryChar;

	if(errorFlagsToCheck & MinDurationPerPrimaryChar)
		if(checkMinDurationPerPrimaryChar(SCConfig::minDurationPerCharacter(), false))
			lineErrorFlags |= MinDurationPerPrimaryChar;

	if(errorFlagsToCheck & MinDurationPerSecondaryChar)
		if(checkMinDurationPerSecondaryChar(SCConfig::minDurationPerCharacter(), false))
			lineErrorFlags |= MinDurationPerSecondaryChar;

	if(errorFlagsToCheck & MaxPrimaryChars)
		if(checkMaxPrimaryChars(SCConfig::maxCharacters(), false))
			lineErrorFlags |= MaxPrimaryChars;

	if(errorFlagsToCheck & MaxSecondaryChars)
		if(checkMaxSecondaryChars(SCConfig::maxCharacters(), false))
			lineErrorFlags |= MaxSecondaryChars;

	if(errorFlagsToCheck & MaxPrimaryLines)
		if(checkMaxPrimaryLines(SCConfig::maxLines(), false))
			lineErrorFlags |= MaxPrimaryLines;

	if(errorFlagsToCheck & MaxSecondaryLines)
		if(checkMaxSecondaryLines(SCConfig::maxLines(), false))
			lineErrorFlags |= MaxSecondaryLines;

	if(errorFlagsToCheck & PrimaryUnneededSpaces)
		if(checkPrimaryUnneededSpaces(false))
			lineErrorFlags |= PrimaryUnneededSpaces;

	if(errorFlagsToCheck & SecondaryUnneededSpaces)
		if(checkSecondaryUnneededSpaces(false))
			lineErrorFlags |= SecondaryUnneededSpaces;

	if(errorFlagsToCheck & PrimaryCapitalAfterEllipsis)
		if(checkPrimaryCapitalAfterEllipsis(false))
			lineErrorFlags |= PrimaryCapitalAfterEllipsis;

	if(errorFlagsToCheck & SecondaryCapitalAfterEllipsis)
		if(checkSecondaryCapitalAfterEllipsis(false))
			lineErrorFlags |= SecondaryCapitalAfterEllipsis;

	if(errorFlagsToCheck & PrimaryUnneededDash)
		if(checkPrimaryUnneededDash(false))
			lineErrorFlags |= PrimaryUnneededDash;

	if(errorFlagsToCheck & SecondaryUnneededDash)
		if(checkSecondaryUnneededDash(false))
			lineErrorFlags |= SecondaryUnneededDash;

	if(update)
		setErrorFlags(lineErrorFlags);

	return lineErrorFlags;
}

void
SubtitleLine::processAction(QUndoCommand *action)
{
	if(m_subtitle) {
		m_subtitle->processAction(action);
	} else {
		action->redo();
		delete action;
	}
}

const QVector<ObjectRef<SubtitleLine>> *
SubtitleLine::refContainer()
{
	return m_subtitle ? &m_subtitle->m_lines : nullptr;
}
