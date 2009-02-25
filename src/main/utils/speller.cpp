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

#include "speller.h"
#include "../application.h"
#include "../configs/spellingconfig.h"
#include "../../core/subtitleiterator.h"

#include <KDebug>
#include <KLocale>
#include <KMessageBox>
#include <sonnet/dialog.h>
#include <sonnet/backgroundchecker.h>

using namespace SubtitleComposer;

Speller::Speller( QWidget* parent ):
	QObject( parent ),
	m_subtitle( 0 ),
	m_translationMode( false ),
	m_feedPrimaryNext( true ),
	m_sonnetDialog( 0 ),
	m_iterator( 0 )
{
	connect( app()->spellingConfig(), SIGNAL(optionChanged(const QString&,const QString&)),
			 this, SLOT(onSpellingOptionChanged(const QString&,const QString&)) );
}

Speller::~Speller()
{
	setSubtitle( 0 );
}

void Speller::invalidate()
{
	delete m_iterator;
	m_iterator = 0;

	m_feedPrimaryNext = true;
}

QWidget* Speller::parentWidget()
{
	return static_cast<QWidget*>( parent() );
}

void Speller::setSubtitle( Subtitle* subtitle )
{
	m_subtitle = subtitle;

	invalidate();
}

void Speller::setTranslationMode( bool enabled )
{
	m_translationMode = enabled;

	invalidate();
}

void Speller::spellCheck( int currentIndex )
{
	invalidate();

	if ( ! m_subtitle || ! m_subtitle->linesCount() )
		return;

	m_iterator = new SubtitleIterator( *m_subtitle );
	m_iterator->toIndex( currentIndex < 0 ? 0 : currentIndex );
	m_firstIndex = m_iterator->index();

	if ( ! m_sonnetDialog )
	{
		m_sonnetDialog = new Sonnet::Dialog( new Sonnet::BackgroundChecker( this ), parentWidget() );

		connect( m_sonnetDialog, SIGNAL( done(const QString&) ), this, SLOT( onBufferDone() ) );

		connect( m_sonnetDialog, SIGNAL( replace(const QString&,int,const QString&) ),
				this, SLOT( onCorrected(const QString&,int,const QString&) ) );

		connect( m_sonnetDialog, SIGNAL( misspelling(const QString&,int) ),
				this, SLOT( onMisspelling(const QString&,int) ) );
	}

	m_sonnetDialog->setBuffer( m_iterator->current()->primaryText().string() );
	m_sonnetDialog->show();

	m_feedPrimaryNext = ! m_feedPrimaryNext;
}

void Speller::onBufferDone()
{
	// NOTE: not setting the buffer in this slots closes the dialog

	if ( advance() )
	{
		if ( m_translationMode )
		{
			m_feedPrimaryNext = ! m_feedPrimaryNext;
			m_sonnetDialog->setBuffer(
				m_feedPrimaryNext ?
					m_iterator->current()->secondaryText().string() :
					m_iterator->current()->primaryText().string()
			);
		}
		else
			m_sonnetDialog->setBuffer( m_iterator->current()->primaryText().string() );
	}
}

bool Speller::advance()
{
	if ( ! m_translationMode || m_feedPrimaryNext )
	{
		++(*m_iterator);

		if ( m_firstIndex == m_iterator->index() ||
			m_firstIndex == m_iterator->firstIndex() && m_iterator->index() == SubtitleIterator::AfterLast )
			return false;

		if ( m_iterator->index() < 0 )
		{
			m_iterator->toFirst();

			if ( KMessageBox::warningContinueCancel(
					parentWidget(),
					i18n( "End of subtitle reached.\nContinue from the beginning?" ),
					i18n( "Spell Checking" )
					) != KMessageBox::Continue )
				return false;
		}
	}

	return true;
}

void Speller::onMisspelling( const QString& before, int pos )
{
	bool primary = ! m_translationMode || ! m_feedPrimaryNext;

	emit misspelled( m_iterator->current(), primary, pos, pos + before.length() - 1 );
}

void Speller::onCorrected( const QString& before, int pos, const QString& after )
{
	if ( m_translationMode && m_feedPrimaryNext )
		m_iterator->current()->setSecondaryText(
			SString( m_iterator->current()->secondaryText() ).replace( pos, before.length(), after )
		);
	else
		m_iterator->current()->setPrimaryText(
			SString( m_iterator->current()->primaryText() ).replace( pos, before.length(), after )
		);
}

void Speller::onSpellingOptionChanged( const QString& option, const QString& /*value*/ )
{
	if ( option == SpellingConfig::keyDefaultLanguage() )
	{
		if ( m_sonnetDialog )
		{
			invalidate();

			m_sonnetDialog->deleteLater();
			m_sonnetDialog = 0;
		}
	}
}

#include "speller.moc"
