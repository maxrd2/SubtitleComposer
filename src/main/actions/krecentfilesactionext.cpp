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

#include "krecentfilesactionext.h"

#include <QtCore/QFile>

#include <KLocale>
#include <KDebug>
#include <KStandardDirs>

KRecentFilesActionExt::KRecentFilesActionExt( QObject* parent ):
	KSelectAction( parent ),
	m_maxCount( 10 ),
	m_ignoreUrlQueryItems( false ),
	m_separatorAction( new QAction( QString(), this ) ),
	m_clearHistoryAction( new QAction( i18n( "Clear History" ), this ) )
{
	setMenuAccelsEnabled( false );

	m_separatorAction->setSeparator( true );

	connect( this, SIGNAL(triggered(int)), this, SLOT(onItemActivated(int)) );
	connect( m_clearHistoryAction, SIGNAL(triggered()), this, SLOT(clearUrls()) );
}


KRecentFilesActionExt::~KRecentFilesActionExt()
{
}

bool KRecentFilesActionExt::ignoreUrlQueryItems() const
{
	return m_ignoreUrlQueryItems;
}

void KRecentFilesActionExt::setIgnoreUrlQueryItems( bool value )
{
	if ( m_ignoreUrlQueryItems != value )
	{
		m_ignoreUrlQueryItems = value;

		if ( m_ignoreUrlQueryItems ) // to remove possible collitions
			setUrls( urls(), false );
	}
}

bool KRecentFilesActionExt::isEmpty() const
{
	return m_urlActions.isEmpty();
}

int KRecentFilesActionExt::count() const
{
	return m_urlActions.count();
}

int KRecentFilesActionExt::maxCount() const
{
	return m_maxCount;
}

void KRecentFilesActionExt::setMaxCount( int maxCount )
{
	if ( m_maxCount != maxCount && maxCount > 0 )
	{
		m_maxCount = maxCount;

		while ( count() > maxCount )
			removeAction( m_urlActions.last() );
	}
}

QString KRecentFilesActionExt::encodingForUrl( const KUrl& url ) const
{
	int index = indexOfUrl( url );
	return index < 0 ? QString() : m_urlActions.at( index )->data().toUrl().queryItemValue( "encoding" );
}

const KUrl::List KRecentFilesActionExt::urls() const
{
	KUrl::List urls;
	for ( QList<QAction*>::ConstIterator it = m_urlActions.begin(), end = m_urlActions.end(); it != end; ++it )
		urls << (*it)->data().toUrl();
	return urls;
}

void KRecentFilesActionExt::setUrls( const KUrl::List& urls, bool ignoreCollisions )
{
	removeAllActions();

	QString entryText( "%1 [%2]" );

	for ( KUrl::List::ConstIterator it = urls.begin(), end = urls.end(); it != end; ++it )
	{
		if ( (*it).isLocalFile() && ! KGlobal::dirs()->relativeLocation( "tmp", (*it).path() ).startsWith( '/' ) )
			continue; // don't store temporary paths

		if ( ! ignoreCollisions && m_ignoreUrlQueryItems && indexOfUrl( *it ) >= 0 )
			continue;

		QAction* action = new QAction(
			entryText.
				arg( (*it).fileName() ).
				arg(
					m_ignoreUrlQueryItems && (*it).isLocalFile() ?
						(*it).path() :
						(*it).pathOrUrl()
				),
			this
		);
		action->setData( QUrl( (*it).prettyUrl() ) );
		m_urlActions.append( action );
		addAction( action );
	}
	addAction( m_separatorAction );
	addAction( m_clearHistoryAction );

	setEnabled( ! isEmpty() );
}

void KRecentFilesActionExt::setUrls( const KUrl::List& urls )
{
	setUrls( urls, false );
}

void KRecentFilesActionExt::addUrl( const KUrl& url )
{
	removeUrl( url ); // avoid duplicates

	KUrl::List urls = this->urls();
	urls.prepend( url );

	setUrls( urls, true );
}

void KRecentFilesActionExt::removeUrl( const KUrl& url )
{
	int prevIndex;
	while ( (prevIndex = indexOfUrl( url )) != -1 )
		removeAction( m_urlActions.at( prevIndex ) );

	setEnabled( ! isEmpty() );
}

void KRecentFilesActionExt::clearUrls()
{
	removeAllActions();

	setEnabled( false );
}

void KRecentFilesActionExt::loadEntries( const KConfigGroup& group )
{
	KUrl::List urls;

	QString key( "File%1" );
	for ( int index = 0, size = qMin( group.readEntry<int>( "Files", m_maxCount ), m_maxCount ); index < size; ++index )
	{
		QString value = group.readPathEntry( key.arg( index ), QString() );
		if ( ! value.isEmpty() )
		{
			KUrl url( value );
			if ( url.isLocalFile() && ! QFile::exists( url.path() ) )
				continue; // Don't restore if file doesn't exist anymore
			urls.append( url );
		}
	}

	setUrls( urls, true );
}

void KRecentFilesActionExt::saveEntries( const KConfigGroup& g )
{
	KConfigGroup group( g );

	group.deleteGroup();

	int index = 0;
	QString key( "File%1" );
	for ( QList<QAction*>::ConstIterator it = m_urlActions.begin(), end = m_urlActions.end(); it != end; ++it )
		group.writePathEntry( key.arg( index++ ), KUrl( (*it)->data().toUrl() ).pathOrUrl() );

	group.writeEntry( "Files", m_urlActions.count() );
}

void KRecentFilesActionExt::onItemActivated( int index )
{
	// NOTE: emit a copy of the Url since the slot where it is connected might call add(),
	// remove() or clear() methods (potentially destroying the relevant m_urls item).

	if ( index >= 0 && index < m_urlActions.count() )
	{
		emit urlSelected( KUrl( m_urlActions.at( index )->data().toUrl() ) );
	}
}

QAction* KRecentFilesActionExt::removeAction( QAction* action )
{
	KSelectAction::removeAction( action );
	if ( m_urlActions.removeOne( action ) )
	{
		delete action;
		return 0;
	}
	else
		return action;
}

int KRecentFilesActionExt::indexOfUrl( const KUrl& url ) const
{
	if ( isEmpty() )
		return -1;

	KUrl refUrl( url );
	if ( m_ignoreUrlQueryItems )
		refUrl.setQuery( "" );

	for ( int index = 0, size = m_urlActions.count(); index < size; ++index )
	{
		KUrl curUrl( m_urlActions.at( index )->data().toUrl() );
		if ( m_ignoreUrlQueryItems )
			curUrl.setQuery( "" );
		if ( curUrl == refUrl )
			return index;
	}

	return -1;
}

#include "krecentfilesactionext.moc"
