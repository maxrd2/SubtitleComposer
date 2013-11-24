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

#include "finder.h"
#include "../../core/subtitleiterator.h"

#include <QtGui/QGroupBox>
#include <QtGui/QRadioButton>
#include <QtGui/QGridLayout>

#include <KDebug>
#include <KLocale>

#include <KMessageBox>

#include <KFind>
#include <KFindDialog>

using namespace SubtitleComposer;

Finder::Finder(QWidget *parent) :
	QObject(parent),
	m_subtitle(0),
	m_translationMode(false),
	m_feedingPrimary(false),
	m_find(0),
	m_iterator(0),
	m_dataLine(0)
{
	m_dialog = new KFindDialog(parent);
	m_dialog->setHasSelection(true);
	m_dialog->setHasCursor(true);
	m_dialog->setOptions(m_dialog->options() | KFind::FromCursor);

	QWidget *mainWidget = m_dialog->mainWidget();
	QLayout *mainLayout = mainWidget->layout();

	m_targetGroupBox = new QGroupBox(mainWidget);
	m_targetGroupBox->setTitle(i18n("Find In"));

	QGridLayout *targetLayout = new QGridLayout(m_targetGroupBox);
	targetLayout->setAlignment(Qt::AlignTop);
	targetLayout->setSpacing(5);

	m_targetRadioButtons[SubtitleLine::Both] = new QRadioButton(m_targetGroupBox);
	m_targetRadioButtons[SubtitleLine::Both]->setChecked(true);
	m_targetRadioButtons[SubtitleLine::Both]->setText(i18n("Both subtitles"));
	m_targetRadioButtons[SubtitleLine::Primary] = new QRadioButton(m_targetGroupBox);
	m_targetRadioButtons[SubtitleLine::Primary]->setText(i18n("Primary subtitle"));
	m_targetRadioButtons[SubtitleLine::Secondary] = new QRadioButton(m_targetGroupBox);
	m_targetRadioButtons[SubtitleLine::Secondary]->setText(i18n("Translation subtitle"));

	targetLayout->addWidget(m_targetRadioButtons[SubtitleLine::Both], 0, 0);
	targetLayout->addWidget(m_targetRadioButtons[SubtitleLine::Primary], 1, 0);
	targetLayout->addWidget(m_targetRadioButtons[SubtitleLine::Secondary], 2, 0);

	mainLayout->addWidget(m_targetGroupBox);
	m_targetGroupBox->hide();
}

Finder::~Finder()
{
	delete m_dialog;

	invalidate();
}

void
Finder::invalidate()
{
	delete m_find;
	m_find = 0;

	delete m_iterator;
	m_iterator = 0;

	m_feedingPrimary = false;

	// NOTE a line shoudn't be deleted before being removed from the subtitle
	// so there's no risk of being left with an invalid referece as this
	// method is called when a line is removed from the subtitle and before
	// anyone has a chance to delete the line.
	if(m_dataLine) {
		disconnect(m_dataLine, SIGNAL(primaryTextChanged(const SString &)), this, SLOT(onLinePrimaryTextChanged(const SString &)));
		disconnect(m_dataLine, SIGNAL(secondaryTextChanged(const SString &)), this, SLOT(onLineSecondaryTextChanged(const SString &)));
		m_dataLine = 0;
	}
}

QWidget *
Finder::parentWidget()
{
	return static_cast<QWidget *>(parent());
}

void
Finder::setSubtitle(Subtitle *subtitle)
{
	m_subtitle = subtitle;

	invalidate();
}

void
Finder::setTranslationMode(bool enabled)
{
	if(m_translationMode != enabled) {
		m_translationMode = enabled;

		if(m_translationMode)
			m_targetGroupBox->show();
		else
			m_targetGroupBox->hide();

		invalidate();
	}
}

void
Finder::find(const RangeList &selectionRanges, int currentIndex, const QString &text, bool findBackwards)
{
	if(!m_subtitle || !m_subtitle->linesCount())
		return;

	invalidate();

	m_dialog->setOptions(findBackwards ? m_dialog->options() | KFind::FindBackwards : m_dialog->options() & ~KFind::FindBackwards);

	if(!text.isEmpty()) {
		QStringList history = m_dialog->findHistory();
		history.removeAll(text);
		history.prepend(text);
		m_dialog->setFindHistory(history);
	}

	if(m_dialog->exec() != QDialog::Accepted)
		return;

	m_find = new KFind(m_dialog->pattern(), m_dialog->options(), 0);
	m_find->closeFindNextDialog();

	connect(m_find, SIGNAL(highlight(const QString &, int, int)), this, SLOT(onHighlight(const QString &, int, int)));

	m_iterator = new SubtitleIterator(*m_subtitle, m_dialog->options() & KFind::SelectedText ? selectionRanges : Range::full());
	if(m_iterator->index() == SubtitleIterator::Invalid) {
		invalidate();
		return;
	}
	m_iterator->setAutoSync(true);

	connect(m_iterator, SIGNAL(syncronized(int, int, bool)), this, SLOT(onIteratorSynchronized(int, int, bool)));

	if(m_dialog->options() & KFind::FromCursor)
		m_iterator->toIndex(currentIndex < 0 ? 0 : currentIndex);

	m_allSearchedIndex = m_iterator->index();

	m_find->setPattern(m_dialog->pattern());
	m_find->setOptions(m_dialog->options());

	m_instancesFound = false;

	advance();
}

bool
Finder::findNext()
{
	if(!m_iterator)
		return false;

	m_find->setOptions(m_find->options() & ~KFind::FindBackwards);

	advance();
	return true;
}

bool
Finder::findPrevious()
{
	if(!m_iterator)
		return false;

	m_find->setOptions(m_find->options() | KFind::FindBackwards);

	advance();
	return true;
}

void
Finder::advance()
{
	KFind::Result res = KFind::NoMatch;

	bool selection = m_find->options() & KFind::SelectedText;
	bool backwards = m_find->options() & KFind::FindBackwards;

	do {
		if(m_find->needData()) {
			if(m_dataLine) {
				disconnect(m_dataLine, SIGNAL(primaryTextChanged(const SString &)), this, SLOT(onLinePrimaryTextChanged(const SString &)));
				disconnect(m_dataLine, SIGNAL(secondaryTextChanged(const SString &)), this, SLOT(onLineSecondaryTextChanged(const SString &)));
			}

			m_dataLine = m_iterator->current();

			if(m_dataLine) {
				if(!m_translationMode || m_targetRadioButtons[SubtitleLine::Primary]->isChecked()) {
					m_feedingPrimary = true;
					m_find->setData(m_dataLine->primaryText().string());
				} else if(m_targetRadioButtons[SubtitleLine::Secondary]->isChecked()) {
					m_feedingPrimary = false;
					m_find->setData(m_dataLine->secondaryText().string());
				} else {                // m_translationMode && m_targetRadioButtons[SubtitleLine::Both]->isChecked()
					m_feedingPrimary = !m_feedingPrimary;   // we alternate the source of data
					m_find->setData((m_feedingPrimary ? m_dataLine->primaryText() : m_dataLine->secondaryText()).string());
				}

				connect(m_dataLine, SIGNAL(primaryTextChanged(const SString &)), this, SLOT(onLinePrimaryTextChanged(const SString &)));

				connect(m_dataLine, SIGNAL(secondaryTextChanged(const SString &)), this, SLOT(onLineSecondaryTextChanged(const SString &)));
			}
		}

		res = m_find->find();

		if(res == KFind::NoMatch && (!m_translationMode || !m_targetRadioButtons[SubtitleLine::Both]->isChecked() || !m_feedingPrimary)) {
			if(backwards)
				--(*m_iterator);
			else
				++(*m_iterator);

			// special case: we searched all lines and didn't found any matches
			if(!m_instancesFound && (m_allSearchedIndex == m_iterator->index() || (backwards ? (m_allSearchedIndex == m_iterator->lastIndex() && m_iterator->index() == SubtitleIterator::BehindFirst) : (m_allSearchedIndex == m_iterator->firstIndex() && m_iterator->index() == SubtitleIterator::AfterLast))
			                         ))
			{
				KMessageBox::sorry(parentWidget(), i18n("No instances of '%1' found!", m_find->pattern()), i18n("Find")
				                   );

				invalidate();

				break;
			}

			if(m_iterator->index() < 0) {
				if(backwards)
					m_iterator->toLast();
				else
					m_iterator->toFirst();

				if(KMessageBox::warningContinueCancel(parentWidget(), backwards ? (selection ? i18n("Beginning of selection reached.\nContinue from the end?") : i18n("Beginning of subtitle reached.\nContinue from the end?")) : (selection ? i18n("End of selection reached.\nContinue from the beginning?") : i18n("End of subtitle reached.\nContinue from the beginning?")), i18n("Find")
				                                      ) != KMessageBox::Continue)
					break;
			}
		}
	} while(res == KFind::NoMatch);
}

void
Finder::onHighlight(const QString &, int matchingIndex, int matchedLength)
{
	m_instancesFound = true;

	emit found(m_iterator->current(), m_feedingPrimary, matchingIndex, matchingIndex + matchedLength - 1);
}

void
Finder::onLinePrimaryTextChanged(const SString &text)
{
	if(m_feedingPrimary)
		m_find->setData(text.string());
}

void
Finder::onLineSecondaryTextChanged(const SString &text)
{
	if(!m_feedingPrimary)
		m_find->setData(text.string());
}

void
Finder::onIteratorSynchronized(int firstIndex, int lastIndex, bool inserted)
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
		if(m_dataLine->index() < 0) {   // m_dataLine was removed
			// work around missing "invalidateData" method in KFind
			long options = m_find->options();
			QString pattern = m_find->pattern();
			delete m_find;
			m_find = new KFind(pattern, options, 0);
			m_find->closeFindNextDialog();
			connect(m_find, SIGNAL(highlight(const QString &, int, int)), this, SLOT(onHighlight(const QString &, int, int)));
		}

		if(m_allSearchedIndex > lastIndex)
			m_allSearchedIndex -= linesCount;
		else if(m_allSearchedIndex >= firstIndex && m_allSearchedIndex <= lastIndex) // was one of the removed lines
			m_allSearchedIndex = m_iterator->index();
	}
}

#include "finder.moc"
