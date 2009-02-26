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

#include "inputdialog.h"

#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QGridLayout>

#include <KPushButton>
#include <KNumInput>

using namespace SubtitleComposer;

/// TEXTINPUTDIALOG
/// ===============

TextInputDialog::TextInputDialog( const QString& caption, const QString& label, QWidget* parent ):
	KDialog( parent )
{
	init( caption, label, QString() );
}

TextInputDialog::TextInputDialog( const QString& caption, const QString& label, const QString& value, QWidget* parent ):
	KDialog( parent )
{
	init( caption, label, value );
}

void TextInputDialog::init( const QString& caption, const QString& labelText, const QString& value )
{
	setCaption( caption );
	setModal( true );
	setButtons( KDialog::Ok | KDialog::Cancel );

	setMainWidget( new QWidget( this ) );

	m_lineEdit = new QLineEdit( mainWidget() );
	m_lineEdit->setText( value );
	m_lineEdit->setFocus();

	QLabel* label = new QLabel( mainWidget() );
	label->setText( labelText );
	label->setBuddy( m_lineEdit );

	QGridLayout* mainLayout = new QGridLayout( mainWidget() );
	mainLayout->setAlignment( Qt::AlignTop );
	mainLayout->setContentsMargins( 0, 0, 0, 0 );
	mainLayout->setSpacing( 5 );
	mainLayout->addWidget( label, 0, 0, Qt::AlignRight|Qt::AlignVCenter );
	mainLayout->addWidget( m_lineEdit, 0, 1 );

	button( KDialog::Ok )->setEnabled( false );

	resize( 300, 10 );

	connect( m_lineEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( onLineEditTextChanged( const QString& ) ) );
}

const QString TextInputDialog::value() const
{
	return m_lineEdit->text();
}

void TextInputDialog::setValue( const QString& value )
{
	m_lineEdit->setText( value );
}

void TextInputDialog::onLineEditTextChanged( const QString& text )
{
	button( KDialog::Ok )->setEnabled( ! text.isEmpty() );
}



/// INTINPUTDIALOG
/// ==============

IntInputDialog::IntInputDialog( const QString& caption, const QString& label, QWidget* parent ):
	KDialog( parent )
{
	init( caption, label, 0, 100, 0 );
}

IntInputDialog::IntInputDialog( const QString& caption, const QString& label, int min, int max, QWidget* parent ):
	KDialog( parent )
{
	init( caption, label, min, max, min );
}

IntInputDialog::IntInputDialog( const QString& caption, const QString& label, int min, int max, int value, QWidget* parent ):
	KDialog( parent )
{
	init( caption, label, min, max, value );
}

void IntInputDialog::init( const QString& caption, const QString& labelText, int min, int max, int value )
{
	setCaption( caption );
	setModal( true );
	setButtons( KDialog::Ok | KDialog::Cancel );

	setMainWidget( new QWidget( this ) );

	m_intNumInput = new KIntNumInput( mainWidget() );
	m_intNumInput->setRange( min, max );
	m_intNumInput->setValue( value );
	m_intNumInput->setEditFocus( true );
	m_intNumInput->setSliderEnabled( true );
	m_intNumInput->setFocus();

	QLabel* label = new QLabel( mainWidget() );
	label->setText( labelText );
	label->setBuddy( m_intNumInput );

	QGridLayout* mainLayout = new QGridLayout( mainWidget() );
	mainLayout->setAlignment( Qt::AlignTop );
	mainLayout->setContentsMargins( 0, 0, 0, 0 );
	mainLayout->setSpacing( 5 );
	mainLayout->addWidget( label, 0, 0, Qt::AlignRight|Qt::AlignVCenter );
	mainLayout->addWidget( m_intNumInput, 0, 1 );
	mainLayout->setColumnStretch( 1, 2 );

	resize( 300, 10 );
}

int IntInputDialog::minimum() const
{
	return m_intNumInput->minimum();
}

void IntInputDialog::setMinimum( int minimum )
{
	m_intNumInput->setMinimum( minimum );
}

int IntInputDialog::maximum() const
{
	return m_intNumInput->maximum();
}

void IntInputDialog::setMaximum( int maximum )
{
	m_intNumInput->setMaximum( maximum );
}

int IntInputDialog::value() const
{
	return m_intNumInput->value();
}

void IntInputDialog::setValue( int value )
{
	m_intNumInput->setValue( value );
}

#include "inputdialog.moc"
