/**
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2015 Mladen Milinkovic <max@smoothware.net>
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

#include "speller.h"
#include "application.h"
#include "core/subtitleiterator.h"

#include <QDebug>

#include <KMessageBox>
#include <KLocalizedString>

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
	connect(SCConfig::self(), SIGNAL(configChanged()), this, SLOT(onConfigChanged()));
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
		m_sonnetDialog->setBuffer((m_useTranslation ? m_iterator->current()->secondaryText() : m_iterator->current()->primaryText()).string());
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

		connect(m_sonnetDialog, SIGNAL(done(const QString &)), this, SLOT(onBufferDone()));

		connect(m_sonnetDialog, SIGNAL(replace(const QString &, int, const QString &)), this, SLOT(onCorrected(const QString &, int, const QString &)));

		connect(m_sonnetDialog, SIGNAL(misspelling(const QString &, int)), this, SLOT(onMisspelling(const QString &, int)));
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
		m_iterator->current()->setSecondaryText(SString(m_iterator->current()->secondaryText()).replace(pos, before.length(), after));
	else
		m_iterator->current()->setPrimaryText(SString(m_iterator->current()->primaryText()).replace(pos, before.length(), after));
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


