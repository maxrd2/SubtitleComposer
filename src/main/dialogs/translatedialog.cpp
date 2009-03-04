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

#include "translatedialog.h"

#include <QtCore/QFile>
#include <QtGui/QLabel>
#include <QtGui/QComboBox>
#include <QtGui/QGroupBox>
#include <QtGui/QGridLayout>

using namespace SubtitleComposer;

TranslateDialog::TranslateDialog( QWidget* parent ):
	ActionWithTargetDialog( i18n( "Translate" ), parent )
{
	QGroupBox* settingsGroupBox = createGroupBox( i18nc( "@title:group", "Settings" ) );

	m_inputLanguageComboBox = new QComboBox( settingsGroupBox );
	m_inputLanguageComboBox->setEditable( false );
	m_inputLanguageComboBox->setIconSize( QSize( 21, 13 ) );
	int index = 0;
	for ( QList<Language::Value>::ConstIterator it = Language::input().begin(), end = Language::input().end(); it != end; ++it )
	{
		m_inputLanguageComboBox->addItem( Language::name( *it ) );
		QString flagPath = Language::flagPath( *it );
		if ( ! flagPath.isEmpty() )
			m_inputLanguageComboBox->setItemIcon( index, KIcon( flagPath ) );
		index++;
	}

	QLabel* inputLanguageLabel = new QLabel( settingsGroupBox );
	inputLanguageLabel->setText( i18n( "Input language:" ) );
	inputLanguageLabel->setBuddy( m_inputLanguageComboBox );

	m_outputLanguageComboBox = new QComboBox( settingsGroupBox );
	m_outputLanguageComboBox->setEditable( false );
	m_outputLanguageComboBox->setIconSize( QSize( 21, 13 ) );
	index = 0;
	for ( QList<Language::Value>::ConstIterator it = Language::output().begin(), end = Language::output().end(); it != end; ++it )
	{
		m_outputLanguageComboBox->addItem( Language::name( *it ) );
		QString flagPath = Language::flagPath( *it );
		if ( ! flagPath.isEmpty() )
			m_outputLanguageComboBox->setItemIcon( index, KIcon( flagPath ) );
		index++;
	}

	QLabel* outputLanguageLabel = new QLabel( settingsGroupBox );
	outputLanguageLabel->setText( i18n( "Output language:" ) );
	outputLanguageLabel->setBuddy( m_outputLanguageComboBox );

	createLineTargetsButtonGroup();
	createTextTargetsButtonGroup();

	setTextsTargetEnabled( Subtitle::Both, false );

	QGridLayout* settingsLayout = createLayout( settingsGroupBox );
	settingsLayout->addWidget( inputLanguageLabel, 0, 0, Qt::AlignRight|Qt::AlignVCenter );
	settingsLayout->addWidget( m_inputLanguageComboBox, 0, 1 );
	settingsLayout->addWidget( outputLanguageLabel, 1, 0, Qt::AlignRight|Qt::AlignVCenter );
	settingsLayout->addWidget( m_outputLanguageComboBox, 1, 1 );
}

Language::Value TranslateDialog::inputLanguage() const
{
	return Language::input().at( m_inputLanguageComboBox->currentIndex() );
}

Language::Value TranslateDialog::outputLanguage() const
{
	return Language::output().at( m_outputLanguageComboBox->currentIndex() );
}

