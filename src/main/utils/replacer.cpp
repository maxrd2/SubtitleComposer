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

#include "replacer.h"
#include "../../core/subtitleiterator.h"
#include "kreplace.h"

#include <QtGui/QGroupBox>
#include <QtGui/QRadioButton>
#include <QtGui/QGridLayout>

#include <KDebug>
#include <KLocale>
#include <KDialog>
#include <KMessageBox>
#include <KReplaceDialog>

using namespace SubtitleComposer;

Replacer::Replacer( QWidget* parent ):
	QObject( parent ),
	m_subtitle( 0 ),
	m_translationMode( false ),
	m_feedingPrimary( false ),
	m_replace( 0 ),
	m_iterator( 0 )
{
	m_dialog = new KReplaceDialog( parent );
	m_dialog->setHasSelection( true );
	m_dialog->setHasCursor( true );
	m_dialog->setOptions( m_dialog->options() | KFind::FromCursor );

	QWidget* mainWidget = m_dialog->mainWidget();
	QLayout* mainLayout = mainWidget->layout();

	m_targetGroupBox = new QGroupBox( mainWidget );
	m_targetGroupBox->setTitle( i18n( "Find/Replace In" ) );

	QGridLayout* targetLayout = new QGridLayout( m_targetGroupBox );
	targetLayout->setAlignment( Qt::AlignTop );
	targetLayout->setSpacing( 5 );

	m_targetRadioButtons[SubtitleLine::Both] = new QRadioButton( m_targetGroupBox );
	m_targetRadioButtons[SubtitleLine::Both]->setChecked( true );
	m_targetRadioButtons[SubtitleLine::Both]->setText( i18n( "Both subtitles" ) );
	m_targetRadioButtons[SubtitleLine::Primary] = new QRadioButton( m_targetGroupBox );
	m_targetRadioButtons[SubtitleLine::Primary]->setText( i18n( "Primary subtitle" ) );
	m_targetRadioButtons[SubtitleLine::Secondary] = new QRadioButton( m_targetGroupBox );
	m_targetRadioButtons[SubtitleLine::Secondary]->setText( i18n( "Translation subtitle" ) );

	targetLayout->addWidget( m_targetRadioButtons[SubtitleLine::Both], 0, 0 );
	targetLayout->addWidget( m_targetRadioButtons[SubtitleLine::Primary], 1, 0 );
	targetLayout->addWidget( m_targetRadioButtons[SubtitleLine::Secondary], 2, 0 );

	mainLayout->addWidget( m_targetGroupBox );
	m_targetGroupBox->hide();
}

Replacer::~Replacer()
{
	delete m_dialog;

	setSubtitle( 0 );
}

void Replacer::invalidate()
{
	delete m_replace;
	m_replace = 0;

	delete m_iterator;
	m_iterator = 0;

	m_feedingPrimary = false;
}

QWidget* Replacer::parentWidget()
{
	return static_cast<QWidget*>( parent() );
}

void Replacer::setSubtitle( Subtitle* subtitle )
{
	m_subtitle = subtitle;

	invalidate();
}

void Replacer::setTranslationMode( bool enabled )
{
	if ( m_translationMode != enabled )
	{
		m_translationMode = enabled;

		if ( m_translationMode )
			m_targetGroupBox->show();
		else
			m_targetGroupBox->hide();

		invalidate();
	}
}

void Replacer::replace( const RangeList& selectionRanges, int currentIndex, const QString& text )
{
	invalidate();

	if ( ! m_subtitle || ! m_subtitle->linesCount() )
		return;

	if ( ! text.isEmpty() )
	{
		QStringList history = m_dialog->findHistory();
		history.removeAll( text );
		history.prepend( text );
		m_dialog->setFindHistory( history );
	}

	if ( m_dialog->exec() != QDialog::Accepted )
		return;

	m_replace = new KReplace( m_dialog->pattern(), m_dialog->replacement(), m_dialog->options(), 0 );

	// Connect findNext signal - called when pressing the button in the dialog
	connect( m_replace, SIGNAL( findNext() ),
			 this, SLOT(advance()) );

	// Connect signals to code which handles highlighting of found text, and on-the-fly replacement.
	connect( m_replace, SIGNAL( highlight( const QString&, int, int ) ),
			 this, SLOT( onHighlight( const QString&, int, int ) ) );

	// Connect replace signal - called when doing a replacement
	connect( m_replace, SIGNAL( replace(const QString&, int, int, int) ),
			 this, SLOT( onReplace(const QString&, int, int, int) ) );

	if ( m_dialog->options() & KFind::SelectedText )
	{
		m_iterator = new SubtitleIterator( *m_subtitle, selectionRanges );
		if ( m_iterator->index() < 0 ) // Invalid index means no lines in selectionRanges
			return;
	}
	else
		m_iterator = new SubtitleIterator( *m_subtitle );

	if ( m_dialog->options() & KFind::FromCursor )
		m_iterator->toIndex( currentIndex < 0 ? 0 : currentIndex );

	m_firstIndex = m_iterator->index();
	m_instancesFound = false;

	advance();
}

void Replacer::advance()
{
	SubtitleCompositeActionExecutor executor( *m_subtitle, i18n( "Replace" ) );

	KFind::Result res = KFind::NoMatch;

	bool selection = m_replace->options() & KFind::SelectedText;
	bool backwards = m_replace->options() & KFind::FindBackwards;

	KDialog* replaceNextDialog = this->replaceNextDialog(); // creates the dialog if it didn't existed before

	do
	{
		if ( m_replace->needData() )
		{
			SubtitleLine* dataLine = m_iterator->current();

			if ( dataLine )
			{
				if ( ! m_translationMode || m_targetRadioButtons[SubtitleLine::Primary]->isChecked() )
				{
					m_feedingPrimary = true;
					m_replace->setData( dataLine->primaryText().string() );
				}
				else if ( m_targetRadioButtons[SubtitleLine::Secondary]->isChecked() )
				{
					m_feedingPrimary = false;
					m_replace->setData( dataLine->secondaryText().string() );
				}
				else // m_translationMode && m_targetRadioButtons[SubtitleLine::Both]->isChecked()
				{
					m_feedingPrimary = ! m_feedingPrimary; // we alternate the source of data
					m_replace->setData( (m_feedingPrimary ? dataLine->primaryText() : dataLine->secondaryText()).string() );
				}
			}
		}

		res = m_replace->replace();

		if ( res == KFind::NoMatch &&
			(! m_translationMode || ! m_targetRadioButtons[SubtitleLine::Both]->isChecked() || ! m_feedingPrimary) )
		{
			if ( backwards )
				--(*m_iterator);
			else
				++(*m_iterator);

			if ( m_firstIndex == m_iterator->index() ||
				 (backwards ?
					(m_firstIndex == m_iterator->lastIndex() && m_iterator->index() == SubtitleIterator::BehindFirst) :
					(m_firstIndex == m_iterator->firstIndex() && m_iterator->index() == SubtitleIterator::AfterLast)) )
			{
				if ( replaceNextDialog )
					replaceNextDialog->hide();

				if ( m_instancesFound && m_replace->numReplacements() )
					KMessageBox::information(
						parentWidget(),
						i18np( "1 replacement done.", "%1 replacements done.", m_replace->numReplacements() ),
						i18n( "Replace" )
					);
				else // special case
					KMessageBox::sorry(
						parentWidget(),
						i18n( "No instances of '%1' found!", m_replace->pattern() ),
						i18n( "Replace" )
					);

				m_replace->resetCounts();
				break;
			}

			if ( m_iterator->index() < 0 )
			{
				if ( backwards )
					m_iterator->toLast();
				else
					m_iterator->toFirst();

				int numReplacements = m_replace->numReplacements();

				m_replace->resetCounts();

				if ( KMessageBox::warningContinueCancel(
						parentWidget(),
						i18np( "1 replacement done.", "%1 replacements done.", numReplacements ) + "\n\n" +
						(backwards ?
							(selection ?
								i18n( "Beginning of selection reached.\nContinue from the end?" ) :
								i18n( "Beginning of subtitle reached.\nContinue from the end?" )) :
							(selection ?
								i18n( "End of selection reached.\nContinue from the beginning?" ) :
								i18n( "End of subtitle reached.\nContinue from the beginning?" ))),
						i18n( "Replace" )
						) != KMessageBox::Continue )
				{
					if ( replaceNextDialog )
						replaceNextDialog->hide();
					break;
				}
			}
		}
	}
	while ( res != KFind::Match );
}

KDialog* Replacer::replaceNextDialog()
{
	if ( ! m_replace )
		return 0;

	KDialog* replaceNextDialog = m_replace->replaceNextDialog( false );

	if ( ! replaceNextDialog && m_replace->options() & KReplaceDialog::PromptOnReplace )
	{
		replaceNextDialog = m_replace->replaceNextDialog( true );
		replaceNextDialog->setModal( true );
	}

	return replaceNextDialog;
}

void Replacer::onHighlight( const QString&, int matchingIndex, int matchedLength )
{
	m_instancesFound = true;

	emit found( m_iterator->current(), m_feedingPrimary, matchingIndex, matchingIndex + matchedLength - 1 );
}

void Replacer::onReplace( const QString& text, int replacementIndex, int replacedLength, int matchedLength )
{
	m_instancesFound = true;

	if ( m_feedingPrimary )
	{
		SString stext = m_iterator->current()->primaryText();
		stext.replace( replacementIndex, matchedLength, text.mid( replacementIndex, replacedLength ) );
		m_iterator->current()->setPrimaryText( stext );
	}
	else
	{
		SString stext = m_iterator->current()->secondaryText();
		stext.replace( replacementIndex, matchedLength, text.mid( replacementIndex, replacedLength ) );
		m_iterator->current()->setSecondaryText( stext );
	}
}

#include "replacer.moc"
