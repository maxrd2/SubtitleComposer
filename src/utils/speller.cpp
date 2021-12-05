/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "speller.h"
#include "application.h"
#include "core/richtext/richdocument.h"
#include "core/subtitleiterator.h"

#include <QDebug>

#include <KMessageBox>
#include <KLocalizedString>

#include <sonnet_version.h>
#include <sonnet/dialog.h>
#include <sonnet/backgroundchecker.h>

using namespace SubtitleComposer;

Speller::Speller(QWidget *parent) :
	QObject(parent),
	m_subtitle(0),
	m_translationMode(false),
	m_useTranslation(false),
	m_sonnetDialog(0),
	m_iterator(0)
{
	connect(SCConfig::self(), &KCoreConfigSkeleton::configChanged, this, &Speller::onConfigChanged);
}

Speller::~Speller()
{
	setSubtitle(0);
}

void
Speller::invalidate()
{
	delete m_iterator;
	m_iterator = 0;
}

QWidget *
Speller::parentWidget()
{
	return static_cast<QWidget *>(parent());
}

void
Speller::setSubtitle(Subtitle *subtitle)
{
	m_subtitle = subtitle;

	invalidate();
}

void
Speller::setTranslationMode(bool enabled)
{
	m_translationMode = enabled;

	if(!enabled)
		m_useTranslation = false;
}

void
Speller::setUseTranslation(bool useTranslation)
{
	if(m_useTranslation != useTranslation) {
		m_useTranslation = useTranslation;
		updateBuffer();
	}
}

void
Speller::updateBuffer()
{
	if(m_iterator)
		m_sonnetDialog->setBuffer((m_useTranslation ? m_iterator->current()->secondaryDoc() : m_iterator->current()->primaryDoc())->toPlainText());
}

void
Speller::spellCheck(int currentIndex)
{
	invalidate();

	if(!m_subtitle || !m_subtitle->linesCount())
		return;

	m_iterator = new SubtitleIterator(*m_subtitle);
	m_iterator->toIndex(currentIndex < 0 ? 0 : currentIndex);
	m_firstIndex = m_iterator->index();

	if(!m_sonnetDialog) {
		m_sonnetDialog = new Sonnet::Dialog(new Sonnet::BackgroundChecker(this), parentWidget());

#if SONNET_VERSION < QT_VERSION_CHECK(5, 65, 0)
		connect(m_sonnetDialog, QOverload<const QString &>::of(&Sonnet::Dialog::done), this, &Speller::onBufferDone);
#else
		connect(m_sonnetDialog, &Sonnet::Dialog::spellCheckDone, this, &Speller::onBufferDone);
#endif

		connect(m_sonnetDialog, &Sonnet::Dialog::replace, this, &Speller::onCorrected);

		connect(m_sonnetDialog, &Sonnet::Dialog::misspelling, this, &Speller::onMisspelling);
	}

	updateBuffer();
	m_sonnetDialog->show();
}

void
Speller::onBufferDone()
{
	// NOTE: not setting the buffer in this slots closes the dialog
	if(advance())
		updateBuffer();
}

bool
Speller::advance()
{
	++(*m_iterator);

	if((m_firstIndex == m_iterator->index()) || (m_firstIndex == m_iterator->firstIndex() && m_iterator->index() == SubtitleIterator::AfterLast))
		return false;

	if(m_iterator->index() < 0) {
		m_iterator->toFirst();

		if(KMessageBox::Continue != KMessageBox::warningContinueCancel(parentWidget(), i18n("End of subtitle reached.\nContinue from the beginning?"), i18n("Spell Checking")))
			return false;
	}

	return true;
}

void
Speller::onMisspelling(const QString &before, int pos)
{
	emit misspelled(m_iterator->current(), !m_useTranslation, pos, pos + before.length() - 1);
}

void
Speller::onCorrected(const QString &before, int pos, const QString &after)
{
	if(before == after)
		return;

	if(m_useTranslation)
		m_iterator->current()->secondaryDoc()->replace(pos, before.length(), after);
	else
		m_iterator->current()->primaryDoc()->replace(pos, before.length(), after);
}

void
Speller::onConfigChanged()
{
	if(m_sonnetDialog) {
		invalidate();

		m_sonnetDialog->deleteLater();
		m_sonnetDialog = nullptr;
	}
}


