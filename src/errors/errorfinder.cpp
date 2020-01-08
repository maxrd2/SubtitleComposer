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

#include "errorfinder.h"
#include "application.h"
#include "core/subtitleiterator.h"
#include "errors/finderrorsdialog.h"
#include "lineswidget.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QPushButton>

#include <QDebug>

#include <KMessageBox>

using namespace SubtitleComposer;


ErrorFinder::ErrorFinder(QWidget *parent)
	: QObject(parent),
	  m_subtitle(nullptr),
	  m_dialog(new FindErrorsDialog(parent)),
	  m_translationMode(false),
	  m_iterator(nullptr)
{
}

ErrorFinder::~ErrorFinder()
{
	delete m_dialog;
	invalidate();
}

void
ErrorFinder::invalidate()
{
	delete m_iterator;
	m_iterator = nullptr;
}

QWidget *
ErrorFinder::parentWidget()
{
	return static_cast<QWidget *>(parent());
}

void
ErrorFinder::setSubtitle(Subtitle *subtitle)
{
	m_subtitle = subtitle;
	invalidate();
}

void
ErrorFinder::setTranslationMode(bool enabled)
{
	if(m_translationMode != enabled) {
		m_translationMode = enabled;
		invalidate();
	}
}

void
ErrorFinder::find(int searchFromIndex, bool findBackwards)
{
	if(!m_subtitle || m_subtitle->isEmpty())
		return;

	if(m_dialog->exec() != QDialog::Accepted)
		return;

	const RangeList targetRanges = app()->linesWidget()->targetRanges(m_dialog->selectedLinesTarget());

	{
		SubtitleCompositeActionExecutor executor(*m_subtitle, i18n("Check Lines Errors"));

		if(m_dialog->clearOtherErrors())
			m_subtitle->clearErrors(targetRanges, SubtitleLine::AllErrors & ~SubtitleLine::UserMark);

		if(m_dialog->clearMarks())
			m_subtitle->setMarked(targetRanges, false);

		m_subtitle->checkErrors(targetRanges, m_dialog->selectedErrorFlags());
	}


	invalidate();

	m_findBackwards = findBackwards;
	m_targetErrorFlags = m_dialog->selectedErrorFlags() | SubtitleLine::UserMark;

	m_iterator = new SubtitleIterator(*m_subtitle, targetRanges);
	if(m_iterator->index() == SubtitleIterator::Invalid) {
		invalidate();
		return;
	}
	m_iterator->toIndex(searchFromIndex < 0 ? 0 : searchFromIndex);

	advance(false);
}

bool
ErrorFinder::findNext(int fromIndex)
{
	if(!m_iterator)
		return false;

	m_findBackwards = false;
	if(fromIndex != -1)
		m_iterator->toIndex(fromIndex);

	advance(true);
	return true;
}

bool
ErrorFinder::findPrevious(int fromIndex)
{
	if(!m_iterator)
		return false;

	m_findBackwards = true;
	if(fromIndex != -1)
		m_iterator->toIndex(fromIndex);

	advance(true);
	return true;
}

void
ErrorFinder::advance(bool advanceIteratorOnFirstStep)
{
	const int startIndex = m_iterator->index();
	bool searched = false;

	for(;;) {
		if(advanceIteratorOnFirstStep) {
			if(m_findBackwards)
				--(*m_iterator);
			else
				++(*m_iterator);

			if(m_iterator->index() < 0) {
				if(m_findBackwards)
					m_iterator->toLast();
				else
					m_iterator->toFirst();

//				const QString text = m_findBackwards
//						? (m_selection ? i18n("Beginning of selection reached.\nContinue from the end?") : i18n("Beginning of subtitle reached.\nContinue from the end?"))
//						: (m_selection ? i18n("End of selection reached.\nContinue from the beginning?") : i18n("End of subtitle reached.\nContinue from the beginning?"));
//				if(KMessageBox::warningContinueCancel(parentWidget(), text, i18n("Find Error")) != KMessageBox::Continue)
//					break;
			}
		} else {
			advanceIteratorOnFirstStep = true;
		}

		if(m_iterator->current()->errorFlags() & m_targetErrorFlags) {
			emit found(m_iterator->current());
			break;
		} else if(searched && startIndex == m_iterator->index()) {
			// searched through all lines and found no errors
			KMessageBox::sorry(parentWidget(), i18n("No errors matching given criteria were found!"), i18n("Find Error"));
			invalidate();
			return;
		}

		searched = true;
	}
}
