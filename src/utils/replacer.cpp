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

#include "replacer.h"
#include "core/subtitleiterator.h"

#include <QGroupBox>
#include <QRadioButton>
#include <QGridLayout>
#include <QDialog>

#include <KFind>
#include <KReplace>
#include <KReplaceDialog>
#include <KLocalizedString>

using namespace SubtitleComposer;

Replacer::Replacer(QWidget *parent)
	: QObject(parent),
	  m_subtitle(nullptr),
	  m_translationMode(false),
	  m_feedingPrimary(false),
	  m_replace(nullptr),
	  m_iterator(nullptr)
{
	m_dialog = new KReplaceDialog(parent);
	m_dialog->setHasSelection(true);
	m_dialog->setHasCursor(true);
	m_dialog->setOptions(m_dialog->options() | KFind::FromCursor);

	QWidget *mainWidget = m_dialog;//->mainWidget();
	QLayout *mainLayout = mainWidget->layout();

	m_targetGroupBox = new QGroupBox(mainWidget);
	m_targetGroupBox->setTitle(i18n("Find/Replace In"));

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

Replacer::~Replacer()
{
	delete m_dialog;

	setSubtitle(nullptr);
}

void
Replacer::invalidate()
{
	delete m_replace;
	m_replace = nullptr;

	delete m_iterator;
	m_iterator = nullptr;

	m_feedingPrimary = false;
}

QWidget *
Replacer::parentWidget()
{
	return static_cast<QWidget *>(parent());
}

void
Replacer::setSubtitle(Subtitle *subtitle)
{
	m_subtitle = subtitle;

	invalidate();
}

void
Replacer::setTranslationMode(bool enabled)
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
Replacer::replace(const RangeList &selectionRanges, int currentIndex, const QString &text)
{
	invalidate();

	if(!m_subtitle || !m_subtitle->linesCount())
		return;

	if(!text.isEmpty()) {
		QStringList history = m_dialog->findHistory();
		history.removeAll(text);
		history.prepend(text);
		m_dialog->setFindHistory(history);
	}

	if(m_dialog->exec() != QDialog::Accepted)
		return;

	m_replace = new KReplace(m_dialog->pattern(), m_dialog->replacement(), m_dialog->options(), 0);

	// Connect findNext signal - called when pressing the button in the dialog
	connect(m_replace, &KReplace::findNext, this, &Replacer::onFindNext);

	// Connect signals to code which handles highlighting of found text, and on-the-fly replacement
	connect(m_replace, QOverload<const QString &,int,int>::of(&KReplace::highlight), this, &Replacer::onHighlight);

	// Connect replace signal - called when doing a replacement
	connect(m_replace, QOverload<const QString &,int,int,int>::of(&KReplace::replace), this, &Replacer::onReplace);

	if(m_dialog->options() & KFind::SelectedText) {
		m_iterator = new SubtitleIterator(*m_subtitle, selectionRanges);
		if(m_iterator->index() < 0) // Invalid index means no lines in selectionRanges
			return;
	} else {
		m_iterator = new SubtitleIterator(*m_subtitle);
	}

	if(m_dialog->options() & KFind::FromCursor)
		m_iterator->toIndex(currentIndex < 0 ? 0 : currentIndex);

	m_firstIndex = m_iterator->index();

	onFindNext();
}

void
Replacer::onFindNext()
{
	KFind::Result res = KFind::NoMatch;

	const bool backwards = m_replace->options() & KFind::FindBackwards;
	const int startIndex = backwards ? m_iterator->lastIndex() : m_iterator->firstIndex();
	const int finalIndex = backwards ? SubtitleIterator::BehindFirst : SubtitleIterator::AfterLast;

	QDialog *replaceNextDialog = this->replaceNextDialog(); // creates the dialog if it didn't exist before

	do {
		if(m_replace->needData()) {
			SubtitleLine *dataLine = m_iterator->current();

			if(dataLine) {
				if(!m_translationMode || m_targetRadioButtons[Primary]->isChecked()) {
					m_feedingPrimary = true;
					m_replace->setData(dataLine->primaryText().string());
				} else if(m_targetRadioButtons[Secondary]->isChecked()) {
					m_feedingPrimary = false;
					m_replace->setData(dataLine->secondaryText().string());
				} else { // m_translationMode && m_targetRadioButtons[SubtitleLine::Both]->isChecked()
					m_feedingPrimary = !m_feedingPrimary;   // alternate the data source
					m_replace->setData((m_feedingPrimary ? dataLine->primaryText() : dataLine->secondaryText()).string());
				}
			}
		}

		res = m_replace->replace();

		if(res == KFind::NoMatch && (!m_translationMode || !m_targetRadioButtons[Both]->isChecked() || !m_feedingPrimary)) {
			if(backwards)
				--(*m_iterator);
			else
				++(*m_iterator);

			if(m_firstIndex == m_iterator->index() || (m_firstIndex == startIndex && m_iterator->index() == finalIndex)) {
				if(replaceNextDialog)
					replaceNextDialog->hide();

				m_replace->displayFinalDialog();
				m_replace->resetCounts();
				break;
			}

			if(m_iterator->index() < 0) {
				if(backwards)
					m_iterator->toLast();
				else
					m_iterator->toFirst();

				if(!m_replace->shouldRestart()) {
					if(replaceNextDialog)
						replaceNextDialog->hide();

					m_replace->resetCounts();
					break;
				}
			}
		}
	} while(res != KFind::Match);
}

QDialog *
Replacer::replaceNextDialog()
{
	if(!m_replace)
		return nullptr;

	QDialog *dlg = m_replace->replaceNextDialog(false);

	if(!dlg && m_replace->options() & KReplaceDialog::PromptOnReplace) {
		dlg = m_replace->replaceNextDialog(true);
		dlg->setModal(true);
	}

	return dlg;
}

void
Replacer::onHighlight(const QString &, int matchingIndex, int matchedLength)
{
	emit found(m_iterator->current(), m_feedingPrimary, matchingIndex, matchingIndex + matchedLength - 1);
}

void
Replacer::onReplace(const QString &text, int replacementIndex, int replacedLength, int matchedLength)
{
	SubtitleCompositeActionExecutor executor(*m_subtitle, i18n("Replace"));

	if(m_feedingPrimary) {
		SString stext = m_iterator->current()->primaryText();
		stext.replace(replacementIndex, matchedLength, text.mid(replacementIndex, replacedLength));
		m_iterator->current()->setPrimaryText(stext);
	} else {
		SString stext = m_iterator->current()->secondaryText();
		stext.replace(replacementIndex, matchedLength, text.mid(replacementIndex, replacedLength));
		m_iterator->current()->setSecondaryText(stext);
	}
}


