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

#include "errorfinder.h"
#include "../../core/subtitleiterator.h"
#include "../dialogs/actionwitherrortargetsdialog.h"

#include <QtGui/QGroupBox>
#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>

#include <KDebug>
#include <KLocale>

#include <KMessageBox>

namespace SubtitleComposer {
class FindErrorsDialog : public ActionWithErrorTargetsDialog
{
public:
	FindErrorsDialog(QWidget *parent) : ActionWithErrorTargetsDialog(i18n("Find Error"), parent)
	{
		createErrorsGroupBox(i18n("Errors to Find"));
		createErrorsButtons(true, translationMode());

		QGroupBox *optionsGroupBox = createGroupBox(i18n("Options"));

		m_selectionCheckBox = new QCheckBox(optionsGroupBox);
		m_selectionCheckBox->setText(i18n("Selected lines"));

		m_fromCurrentCheckBox = new QCheckBox(optionsGroupBox);
		m_fromCurrentCheckBox->setText(i18n("From current line"));
		m_fromCurrentCheckBox->setChecked(true);

		m_findBackwards = new QCheckBox(optionsGroupBox);
		m_findBackwards->setText(i18n("Find backwards"));

		createTargetsGroupBox("Find In");
		createTextTargetsButtonGroup();

		QGridLayout *optionsLayout = createLayout(optionsGroupBox);
		optionsLayout->addWidget(m_selectionCheckBox, 0, 0);
		optionsLayout->addWidget(m_fromCurrentCheckBox, 1, 0);
		optionsLayout->addWidget(m_findBackwards, 0, 1);
	}

	bool findInSelection() const { return m_selectionCheckBox->isChecked(); }
	bool findFromCurrent() const { return m_fromCurrentCheckBox->isChecked(); }
	bool findBackwards() const { return m_findBackwards->isChecked(); }
	void setFindBackwards(bool value) { m_findBackwards->setChecked(value); }

protected:
	virtual void setTranslationMode(bool value)
	{
		ActionWithErrorTargetsDialog::setTranslationMode(value);
		createErrorsButtons(true, value);
	}

private:
	QCheckBox *m_selectionCheckBox;
	QCheckBox *m_fromCurrentCheckBox;
	QCheckBox *m_findBackwards;
};
}

using namespace SubtitleComposer;

ErrorFinder::ErrorFinder(QWidget *parent) :
	QObject(parent),
	m_subtitle(0),
	m_translationMode(false),
	m_iterator(0)
{
	m_dialog = new FindErrorsDialog(parent);
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
	m_iterator = 0;
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
ErrorFinder::find(const RangeList &selectionRanges, int currentIndex, bool findBackwards)
{
	if(!m_subtitle || !m_subtitle->linesCount())
		return;

	invalidate();

	m_dialog->setFindBackwards(findBackwards);

	if(m_dialog->exec() != QDialog::Accepted)
		return;

	m_selection = m_dialog->findInSelection();
	m_findBackwards = m_dialog->findBackwards();
	m_targetErrorFlags = m_dialog->selectedErrorFlags();

	m_iterator = new SubtitleIterator(*m_subtitle, m_selection ? selectionRanges : Range::full());
	if(m_iterator->index() == SubtitleIterator::Invalid) {
		invalidate();
		return;
	}
	m_iterator->setAutoSync(true);

	connect(m_iterator, SIGNAL(syncronized(int, int, bool)), this, SLOT(onIteratorSynchronized(int, int, bool)));

	if(m_dialog->findFromCurrent())
		m_iterator->toIndex(currentIndex < 0 ? 0 : currentIndex);

	m_allSearchedIndex = m_iterator->index();

	m_instancesFound = false;

	advance(false);
}

bool
ErrorFinder::findNext()
{
	if(!m_iterator)
		return false;

	m_findBackwards = false;

	advance(true);
	return true;
}

bool
ErrorFinder::findPrevious()
{
	if(!m_iterator)
		return false;

	m_findBackwards = true;

	advance(true);
	return true;
}

void
ErrorFinder::advance(bool advanceIteratorOnFirstStep)
{
	bool foundError = false;

	do {
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

				if(KMessageBox::warningContinueCancel(parentWidget(), m_findBackwards ? (m_selection ? i18n("Beginning of selection reached.\nContinue from the end?") : i18n("Beginning of subtitle reached.\nContinue from the end?")) : (m_selection ? i18n("End of selection reached.\nContinue from the beginning?") : i18n("End of subtitle reached.\nContinue from the beginning?")), i18n("Find Error")
													  ) != KMessageBox::Continue)
					break;
			}
		}

		advanceIteratorOnFirstStep = true;
		foundError = m_iterator->current()->errorFlags() & m_targetErrorFlags;

		if(!foundError) {
			// special case: we searched all lines and didn't found any matches
			if(!m_instancesFound && (m_allSearchedIndex == m_iterator->index() || (m_findBackwards ? (m_allSearchedIndex == m_iterator->lastIndex() && m_iterator->index() == SubtitleIterator::BehindFirst) : (m_allSearchedIndex == m_iterator->firstIndex() && m_iterator->index() == SubtitleIterator::AfterLast))
									 ))
			{
				KMessageBox::sorry(parentWidget(), i18n("No errors matching given criteria found!"), i18n("Find Error")
								   );
				invalidate();
				break;
			}
		}
	} while(!foundError);

	if(foundError) {
		m_instancesFound = true;
		emit found(m_iterator->current());
	}
}

void
ErrorFinder::onIteratorSynchronized(int firstIndex, int lastIndex, bool inserted)
{
	if(m_iterator->index() == SubtitleIterator::Invalid) {
		invalidate();
		return;
	}

	int linesCount = lastIndex - firstIndex + 1;
	if(inserted) {
		if(m_allSearchedIndex >= firstIndex)
			m_allSearchedIndex += linesCount;
	} else {
		if(m_allSearchedIndex > lastIndex)
			m_allSearchedIndex -= linesCount;
		else if(m_allSearchedIndex >= firstIndex && m_allSearchedIndex <= lastIndex) // was one of the removed lines
			m_allSearchedIndex = m_iterator->index();
	}
}

#include "errorfinder.moc"
