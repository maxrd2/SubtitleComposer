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

#include "changetextscasedialog.h"

#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>
#include <QtGui/QButtonGroup>
#include <QtGui/QRadioButton>
#include <QtGui/QGridLayout>

#include <KLocale>

using namespace SubtitleComposer;

ChangeTextsCaseDialog::ChangeTextsCaseDialog( QWidget* parent ):
	ActionWithTargetDialog( i18n( "Change Case" ), parent )
{
	QGroupBox* caseGroupBox = createGroupBox( i18nc( "@title:group", "Change To" ) );
	QButtonGroup* caseButtonGroup = new QButtonGroup( this );

	m_lowerRadioButton = new QRadioButton( caseGroupBox );
	m_lowerRadioButton->setText( i18n( "Lower case" ) );
	m_lowerRadioButton->setChecked( true );
	caseButtonGroup->addButton( m_lowerRadioButton, 0 );

	m_upperRadioButton = new QRadioButton( caseGroupBox );
	m_upperRadioButton->setText( i18n( "Upper case" ) );
	caseButtonGroup->addButton( m_upperRadioButton, 1 );

	m_titleRadioButton = new QRadioButton( caseGroupBox );
	m_titleRadioButton->setText( i18n( "Title case" ) );
	caseButtonGroup->addButton( m_titleRadioButton, 2 );

	m_sentenceRadioButton = new QRadioButton( caseGroupBox );
	m_sentenceRadioButton->setText( i18n( "Sentence case" ) );
	caseButtonGroup->addButton( m_sentenceRadioButton, 3 );

	m_lowerFirstCheckBox = new QCheckBox( caseGroupBox );
	m_lowerFirstCheckBox->setText( i18n( "Convert to lower case first" ) );
	m_lowerFirstCheckBox->setEnabled( false );

	connect( caseButtonGroup, SIGNAL( buttonClicked(int) ), this, SLOT( onCaseButtonGroupClicked(int) ) );

	createLineTargetsButtonGroup();
	createTextTargetsButtonGroup();

	QHBoxLayout* lowerFirstLayout = new QHBoxLayout();
	lowerFirstLayout->addSpacing( 20 );
	lowerFirstLayout->addWidget( m_lowerFirstCheckBox );

	QGridLayout* caseLayout = createLayout( caseGroupBox );
	caseLayout->addWidget( m_lowerRadioButton, 0, 0 );
	caseLayout->addWidget( m_upperRadioButton, 1, 0 );
	caseLayout->addWidget( m_titleRadioButton, 2, 0 );
	caseLayout->addWidget( m_sentenceRadioButton, 3, 0 );
	caseLayout->addLayout( lowerFirstLayout, 4, 0 );
}

ChangeTextsCaseDialog::CaseOp ChangeTextsCaseDialog::caseOperation() const
{
	if ( m_lowerRadioButton->isChecked() )
		return ChangeTextsCaseDialog::Lower;
	else if ( m_upperRadioButton->isChecked() )
		return ChangeTextsCaseDialog::Upper;
	else if ( m_titleRadioButton->isChecked() )
		return ChangeTextsCaseDialog::Title;
	else // if ( m_sentenceRadioButton->isChecked() )
		return ChangeTextsCaseDialog::Sentence;
}

bool ChangeTextsCaseDialog::lowerFirst() const
{
	return m_lowerFirstCheckBox->isChecked();
}

void ChangeTextsCaseDialog::onCaseButtonGroupClicked( int id )
{
	if ( id < 2 )
		m_lowerFirstCheckBox->setChecked( false );
	m_lowerFirstCheckBox->setEnabled( id >= 2 );
}

#include "changetextscasedialog.moc"
