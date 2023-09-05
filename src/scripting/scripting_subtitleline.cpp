/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "scripting_subtitleline.h"

#include "appglobal.h"
#include "application.h"
#include "core/richtext/richdocument.h"
#include "core/subtitletarget.h"
#include "scripting/scripting_richstring.h"

#include <QDebug>

using namespace SubtitleComposer;

static SubtitleTarget
toTextTarget(int value, int opDefault)
{
	if(value < Primary || value >= SubtitleTargetSize)
		return SubtitleTarget(opDefault);
	return SubtitleTarget(value);
}

Scripting::SubtitleLine::SubtitleLine(SubtitleComposer::SubtitleLine *backend, QObject *parent) :
	QObject(parent),
	m_backend(backend)
{}

int
Scripting::SubtitleLine::number() const
{
	return m_backend->number();
}

int
Scripting::SubtitleLine::index() const
{
	return m_backend->index();
}

QObject *
Scripting::SubtitleLine::prevLine() const
{
	if(!m_backend->prevLine())
		return 0;
	return new Scripting::SubtitleLine(m_backend->prevLine(), const_cast<Scripting::SubtitleLine *>(this));
}

QObject *
Scripting::SubtitleLine::nextLine() const
{
	if(!m_backend->nextLine())
		return 0;
	return new Scripting::SubtitleLine(m_backend->nextLine(), const_cast<Scripting::SubtitleLine *>(this));
}

int
Scripting::SubtitleLine::primaryCharacters() const
{
	return m_backend->primaryCharacters();
}

int
Scripting::SubtitleLine::primaryWords() const
{
	return m_backend->primaryWords();
}

int
Scripting::SubtitleLine::primaryLines() const
{
	return m_backend->primaryLines();
}

QObject *
Scripting::SubtitleLine::primaryText() const
{
	return new Scripting::RichString(m_backend->primaryDoc()->toRichText(), const_cast<Scripting::SubtitleLine *>(this));
}

void
Scripting::SubtitleLine::setPrimaryText(QObject *object)
{
	const Scripting::RichString *string = qobject_cast<const Scripting::RichString *>(object);
	if(string)
		m_backend->primaryDoc()->setRichText(string->m_backend);
}

QString
Scripting::SubtitleLine::plainPrimaryText() const
{
	return m_backend->primaryDoc()->toPlainText();
}

void
Scripting::SubtitleLine::setPlainPrimaryText(const QString &plainText)
{
	m_backend->primaryDoc()->setPlainText(plainText);
}

QString
Scripting::SubtitleLine::richPrimaryText() const
{
	return m_backend->primaryDoc()->toRichText().richString();
}

void
Scripting::SubtitleLine::setRichPrimaryText(const QString &richText)
{
	SubtitleComposer::RichString text;
	text.setRichString(richText);
	m_backend->primaryDoc()->setRichText(text);
}

int
Scripting::SubtitleLine::secondaryCharacters() const
{
	return m_backend->secondaryCharacters();
}

int
Scripting::SubtitleLine::secondaryWords() const
{
	return m_backend->secondaryWords();
}

int
Scripting::SubtitleLine::secondaryLines() const
{
	return m_backend->secondaryLines();
}

QObject *
Scripting::SubtitleLine::secondaryText() const
{
	return new Scripting::RichString(m_backend->secondaryDoc()->toRichText(), const_cast<Scripting::SubtitleLine *>(this));
}

void
Scripting::SubtitleLine::setSecondaryText(QObject *object)
{
	if(app()->translationMode()) {
		const Scripting::RichString *string = qobject_cast<const Scripting::RichString *>(object);
		if(string)
			m_backend->secondaryDoc()->setRichText(string->m_backend);
	}
}

QString
Scripting::SubtitleLine::plainSecondaryText() const
{
	return m_backend->secondaryDoc()->toPlainText();
}

void
Scripting::SubtitleLine::setPlainSecondaryText(const QString &plainText)
{
	if(app()->translationMode())
		m_backend->secondaryDoc()->setPlainText(plainText);
}

QString
Scripting::SubtitleLine::richSecondaryText() const
{
	return m_backend->secondaryDoc()->toRichText();
}

void
Scripting::SubtitleLine::setRichSecondaryText(const QString &richText)
{
	if(app()->translationMode()) {
		SubtitleComposer::RichString text;
		text.setRichString(richText);
		m_backend->secondaryDoc()->setRichText(text);
	}
}

void
Scripting::SubtitleLine::breakText(int minLengthForBreak, int target)
{
	const int opDefault = app()->translationMode() ? Both : Primary;

	m_backend->breakText(minLengthForBreak, toTextTarget(target, opDefault));
}

void
Scripting::SubtitleLine::unbreakText(int target)
{
	const int opDefault = app()->translationMode() ? Both : Primary;

	m_backend->unbreakText(toTextTarget(target, opDefault));
}

void
Scripting::SubtitleLine::simplifyTextWhiteSpace(int target)
{
	const int opDefault = app()->translationMode() ? Both : Primary;

	m_backend->simplifyTextWhiteSpace(toTextTarget(target, opDefault));
}

int
Scripting::SubtitleLine::showTime() const
{
	return m_backend->showTime().toMillis();
}

void
Scripting::SubtitleLine::setShowTime(int showTime)
{
	m_backend->setShowTime(showTime);
}

int
Scripting::SubtitleLine::hideTime() const
{
	return m_backend->hideTime().toMillis();
}

void
Scripting::SubtitleLine::setHideTime(int hideTime)
{
	m_backend->setHideTime(hideTime);
}

int
Scripting::SubtitleLine::durationTime() const
{
	return m_backend->durationTime().toMillis();
}

void
Scripting::SubtitleLine::setDurationTime(int durationTime)
{
	m_backend->setDurationTime(durationTime);
}

int
Scripting::SubtitleLine::autoDuration(int msecsPerChar, int msecsPerWord, int msecsPerLine, int calculationTarget)
{
	const int opDefault = app()->translationMode() ? Both : Primary;

	return m_backend->autoDuration(msecsPerChar, msecsPerWord, msecsPerLine, toTextTarget(calculationTarget, opDefault)).toMillis();
}

void
Scripting::SubtitleLine::shiftTimes(int mseconds)
{
	m_backend->shiftTimes(mseconds);
}

void
Scripting::SubtitleLine::adjustTimes(int shiftMseconds, double scaleFactor)
{
	m_backend->adjustTimes(shiftMseconds, scaleFactor);
}

int
Scripting::SubtitleLine::errorCount() const
{
	return m_backend->errorCount();
}

int
Scripting::SubtitleLine::errorFlags() const
{
	return m_backend->errorFlags();
}

void
Scripting::SubtitleLine::setErrorFlags(int errorFlags)
{
	m_backend->setErrorFlags(errorFlags);
}

void
Scripting::SubtitleLine::setErrorFlags(int errorFlags, bool value)
{
	m_backend->setErrorFlags(errorFlags, value);
}

bool
Scripting::SubtitleLine::checkEmptyPrimaryText(bool update)
{
	return m_backend->checkEmptyPrimaryText(update);
}

bool
Scripting::SubtitleLine::checkEmptySecondaryText(bool update)
{
	return m_backend->checkEmptySecondaryText(update);
}

bool
Scripting::SubtitleLine::checkUntranslatedText(bool update)
{
	return m_backend->checkUntranslatedText(update);
}

bool
Scripting::SubtitleLine::checkOverlapsWithNext(bool update)
{
	return m_backend->checkOverlapsWithNext(update);
}

bool
Scripting::SubtitleLine::checkMinDuration(int minMsecs, bool update)
{
	return m_backend->checkMinDuration(minMsecs, update);
}

bool
Scripting::SubtitleLine::checkMaxDuration(int maxMsecs, bool update)
{
	return m_backend->checkMaxDuration(maxMsecs, update);
}

bool
Scripting::SubtitleLine::checkMinDurationPerPrimaryChar(int minMsecsPerChar, bool update)
{
	return m_backend->checkMinDurationPerPrimaryChar(minMsecsPerChar, update);
}

bool
Scripting::SubtitleLine::checkMinDurationPerSecondaryChar(int minMsecsPerChar, bool update)
{
	return m_backend->checkMinDurationPerSecondaryChar(minMsecsPerChar, update);
}

bool
Scripting::SubtitleLine::checkMaxDurationPerPrimaryChar(int maxMsecsPerChar, bool update)
{
	return m_backend->checkMaxDurationPerPrimaryChar(maxMsecsPerChar, update);
}

bool
Scripting::SubtitleLine::checkMaxDurationPerSecondaryChar(int maxMsecsPerChar, bool update)
{
	return m_backend->checkMaxDurationPerSecondaryChar(maxMsecsPerChar, update);
}

bool
Scripting::SubtitleLine::checkMaxPrimaryChars(int maxCharacters, bool update)
{
	return m_backend->checkMaxPrimaryChars(maxCharacters, update);
}

bool
Scripting::SubtitleLine::checkMaxSecondaryChars(int maxCharacters, bool update)
{
	return m_backend->checkMaxSecondaryChars(maxCharacters, update);
}

bool
Scripting::SubtitleLine::checkMaxPrimaryLines(int maxLines, bool update)
{
	return m_backend->checkMaxPrimaryLines(maxLines, update);
}

bool
Scripting::SubtitleLine::checkMaxSecondaryLines(int maxLines, bool update)
{
	return m_backend->checkMaxSecondaryLines(maxLines, update);
}

bool
Scripting::SubtitleLine::checkPrimaryUnneededSpaces(bool update)
{
	return m_backend->checkPrimaryUnneededSpaces(update);
}

bool
Scripting::SubtitleLine::checkSecondaryUnneededSpaces(bool update)
{
	return m_backend->checkSecondaryUnneededSpaces(update);
}

bool
Scripting::SubtitleLine::checkPrimaryCapitalAfterEllipsis(bool update)
{
	return m_backend->checkPrimaryCapitalAfterEllipsis(update);
}

bool
Scripting::SubtitleLine::checkSecondaryCapitalAfterEllipsis(bool update)
{
	return m_backend->checkSecondaryCapitalAfterEllipsis(update);
}

bool
Scripting::SubtitleLine::checkPrimaryUnneededDash(bool update)
{
	return m_backend->checkPrimaryUnneededDash(update);
}

bool
Scripting::SubtitleLine::checkSecondaryUnneededDash(bool update)
{
	return m_backend->checkSecondaryUnneededDash(update);
}

int
Scripting::SubtitleLine::check(int errorFlagsToCheck, bool update)
{
	return m_backend->check(errorFlagsToCheck, update);
}

bool
Scripting::SubtitleLine::isRightToLeft() const
{
	return m_backend->primaryDoc()->toPlainText().isRightToLeft();
}
