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

#include "syncsubtitlesdialog.h"

#include <QtGui/QGroupBox>
#include <QtGui/QRadioButton>
#include <QtGui/QGridLayout>

#include <KLocale>


using namespace SubtitleComposer;

SyncSubtitlesDialog::SyncSubtitlesDialog( const QString& defaultEncoding, QWidget* parent ):
	SelectableSubtitleDialog( defaultEncoding, i18n( "Synchronize with Subtitle" ), parent )
{
	createSubtitleGroupBox( i18n( "Reference Subtitle" ) );

	QGroupBox* syncModeGroupBox = createGroupBox( i18nc( "@title:group", "Synchronization Mode" ));

	m_adjustRadioButton = new QRadioButton( syncModeGroupBox );
	m_adjustRadioButton->setText( i18n( "Adjust to reference's first and last lines" ) );
	m_adjustRadioButton->setChecked( true );

	m_synchronizeRadioButton = new QRadioButton( syncModeGroupBox );
	m_synchronizeRadioButton->setText( i18n( "Copy timing information from reference line by line" ) );

	QGridLayout* syncModeLayout = createLayout( syncModeGroupBox );
	syncModeLayout->addWidget( m_adjustRadioButton, 0, 0 );
	syncModeLayout->addWidget( m_synchronizeRadioButton, 1, 0 );
}

bool SyncSubtitlesDialog::adjustToReferenceSubtitle() const
{
	return m_adjustRadioButton->isChecked();
}

bool SyncSubtitlesDialog::synchronizeToReferenceTimes() const
{
	return m_synchronizeRadioButton->isChecked();
}

#include "syncsubtitlesdialog.moc"
