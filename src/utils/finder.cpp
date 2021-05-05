/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2018 Mladen Milinkovic <max@smoothware.net>
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

#include "finder.h"
#include "core/richdocument.h"
#include "core/subtitleiterator.h"

#include <QGroupBox>
#include <QRadioButton>
#include <QGridLayout>
#include <QDebug>

#include <KMessageBox>
#include <KFind>
#include <KFindDialog>
#include <KLocalizedString>

#include <ktextwidgets_version.h>


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

	QWidget *mainWidget = m_dialog;//->mainWidget();
	QLayout *mainLayout = mainWidget->layout();

	m_targetGroupBox = new QGroupBox(mainWidget);
	m_targetGroupBox->setTitle(i18n("Find In"));

	QGridLayout *targetLayout = new QGridLayout(m_targetGroupBox);
	targetLayout->setAlignment(Qt::AlignTop);
	targetLayout->setSpacing(5);

	m_targetRadioButtons[Both] = new QRadioButton(m_targetGroupBox);
	m_targetRadioButtons[Both]->setChecked(true);
	m_targetRadioButtons[Both]->setText(i18n("Both subtitles"));
	m_targetRadioButtons[Primary] = new QRadioButton(m_targetGroupBox);
	m_targetRadioButtons[Primary]->setText(i18n("Primary subtitle"));
	m_targetRadioButtons[Secondary] = new QRadioButton(m_targetGroupBox);
	m_targetRadioButtons[Secondary]->setText(i18n("Translation subtitle"));

	targetLayout->addWidget(m_targetRadioButtons[Both], 0, 0);
	targetLayout->addWidget(m_targetRadioButtons[Primary], 1, 0);
	targetLayout->addWidget(m_targetRadioButtons[Secondary], 2, 0);

	mainLayout->addWidget(m_targetGroupBox);
	m_targetGroupBox->hide();
}

Finder::~Finder()
{
	invalidate();
}

void
Finder::invalidate()
{
	if(m_find) {
		delete m_find;
		m_find = 0;
	}

	if(m_iterator) {
		delete m_iterator;
		m_iterator = 0;
	}

	m_feedingPrimary = false;

	// NOTE a line shouldn't be deleted before being removed from the subtitle
	// so there's no risk of being left with an invalid reference as this
	// method is called when a line is removed from the subtitle and before
	// anyone has a chance to delete the line.
	if(m_dataLine) {
		disconnect(m_dataLine, &SubtitleLine::primaryTextChanged, this, &Finder::onLinePrimaryTextChanged);
		disconnect(m_dataLine, &SubtitleLine::secondaryTextChanged, this, &Finder::onLineSecondaryTextChanged);
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

#if KTEXTWIDGETS_VERSION < QT_VERSION_CHECK(5, 81, 0)
	connect(m_find, QOverload<const QString &, int, int>::of(&KFind::highlight), this, &Finder::onHighlight);
#else
	connect(m_find, &KFind::textFound, this, &Finder::onHighlight);
#endif

	m_iterator = new SubtitleIterator(*m_subtitle, m_dialog->options() & KFind::SelectedText ? selectionRanges : Range::full());
	if(m_iterator->index() == SubtitleIterator::Invalid) {
		invalidate();
		return;
	}

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
				disconnect(m_dataLine, &SubtitleLine::primaryTextChanged, this, &Finder::onLinePrimaryTextChanged);
				disconnect(m_dataLine, &SubtitleLine::secondaryTextChanged, this, &Finder::onLineSecondaryTextChanged);
			}

			m_dataLine = m_iterator->current();

			if(m_dataLine) {
				if(!m_translationMode || m_targetRadioButtons[Primary]->isChecked()) {
					m_feedingPrimary = true;
					m_find->setData(m_dataLine->primaryDoc()->toPlainText());
				} else if(m_targetRadioButtons[Secondary]->isChecked()) {
					m_feedingPrimary = false;
					m_find->setData(m_dataLine->secondaryDoc()->toPlainText());
				} else {                // m_translationMode && m_targetRadioButtons[SubtitleLine::Both]->isChecked()
					m_feedingPrimary = !m_feedingPrimary;   // we alternate the source of data
					m_find->setData((m_feedingPrimary ? m_dataLine->primaryDoc() : m_dataLine->secondaryDoc())->toPlainText());
				}

				connect(m_dataLine, &SubtitleLine::primaryTextChanged, this, &Finder::onLinePrimaryTextChanged);

				connect(m_dataLine, &SubtitleLine::secondaryTextChanged, this, &Finder::onLineSecondaryTextChanged);
			}
		}

		res = m_find->find();

		if(res == KFind::NoMatch && (!m_translationMode || !m_targetRadioButtons[Both]->isChecked() || !m_feedingPrimary)) {
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
Finder::onLinePrimaryTextChanged()
{
	if(m_feedingPrimary)
		m_find->setData(m_dataLine->primaryDoc()->toPlainText());
}

void
Finder::onLineSecondaryTextChanged()
{
	if(!m_feedingPrimary)
		m_find->setData(m_dataLine->secondaryDoc()->toPlainText());
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
#if KTEXTWIDGETS_VERSION < QT_VERSION_CHECK(5, 81, 0)
			connect(m_find, QOverload<const QString &, int, int>::of(&KFind::highlight), this, &Finder::onHighlight);
#else
			connect(m_find, &KFind::textFound, this, &Finder::onHighlight);
#endif
		}

		if(m_allSearchedIndex > lastIndex)
			m_allSearchedIndex -= linesCount;
		else if(m_allSearchedIndex >= firstIndex && m_allSearchedIndex <= lastIndex) // was one of the removed lines
			m_allSearchedIndex = m_iterator->index();
	}
}


