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

#include "playerwidget.h"
#include "application.h"
#include "configs/playerconfig.h"
#include "actions/useractionnames.h"
#include "../common/commondefs.h"
#include "../core/subtitleiterator.h"
#include "../player/player.h"
#include "../player/playerbackend.h"
#include "../widgets/layeredwidget.h"
#include "../widgets/textoverlaywidget.h"
#include "../widgets/attachablewidget.h"
#include "../widgets/pointingslider.h"
#include "../widgets/timeedit.h"

#include <QtCore/QEvent>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QKeyEvent>

#include <QtGui/QDesktopWidget>
#include <QtGui/QCursor>
#include <QtGui/QLabel>
#include <QtGui/QToolButton>
#include <QtGui/QGroupBox>
#include <QtGui/QGridLayout>

#include <KLocale>
#include <KConfigGroup>
#include <KMessageBox>
#include <KMenu>
#include <KPushButton>


using namespace SubtitleComposer;

// FIXME WTF is this!??
#define MAGIC_NUMBER -1

#define HIDE_MOUSE_MSECS 2000
#define UNKNOWN_LENGTH_STRING	(" / " + Time().toString() + ' ')

PlayerWidget::PlayerWidget( QWidget* parent ):
	QWidget( parent ),
	m_subtitle( 0 ),
	m_translationMode( false ),
	m_showTranslation( false ),
	m_overlayLine( 0 ),
	m_playingLine( 0 ),
	m_fullScreenMode( false ),
	m_player( Player::instance() ),
	m_lengthString( UNKNOWN_LENGTH_STRING ),
	m_updatePositionControls( 1 ),
	m_updateVideoPosition( false ),
	m_updateVolumeControls( true ),
	m_updatePlayerVolume( false ),
	m_showPositionTimeEdit( app()->playerConfig()->showPositionTimeEdit() )
{
	m_layeredWidget = new LayeredWidget( this );
	m_layeredWidget->setAcceptDrops( true );
	m_layeredWidget->installEventFilter( this );
	m_layeredWidget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

	m_player->initialize( m_layeredWidget, app()->playerConfig()->backend() );

	connect( m_player, SIGNAL(backendInitialized(PlayerBackend*)), this, SLOT( onPlayerBackendInitialized() ) );

	m_textOverlay = new TextOverlayWidget( m_layeredWidget );
	m_textOverlay->setRichTextMode( true );
	m_textOverlay->setAlignment( Qt::AlignHCenter|Qt::AlignBottom );

	m_seekSlider = new PointingSlider( Qt::Horizontal, this );
	m_seekSlider->setTickPosition( QSlider::NoTicks );
	m_seekSlider->setTracking( false );
	m_seekSlider->setMinimum( 0 );
	m_seekSlider->setMaximum( 1000 );
	m_seekSlider->setPageStep( 10 );
	m_seekSlider->setFocusPolicy( Qt::NoFocus );

	m_infoControlsGroupBox = new QGroupBox( this );
	m_infoControlsGroupBox->setAcceptDrops( true );
	m_infoControlsGroupBox->installEventFilter( this );

	QLabel* positionTagLabel = new QLabel( m_infoControlsGroupBox );
	positionTagLabel->setText( i18n( "<b>Position</b>" ) );
	positionTagLabel->installEventFilter( this );

	m_positionLabel = new QLabel( m_infoControlsGroupBox );
	m_positionEdit = new TimeEdit( m_infoControlsGroupBox );
	m_positionEdit->setFocusPolicy( Qt::NoFocus );

	QLabel* lengthTagLabel = new QLabel( m_infoControlsGroupBox );
	lengthTagLabel->setText( i18n( "<b>Length</b>") );
	lengthTagLabel->installEventFilter( this );
	m_lengthLabel = new QLabel( m_infoControlsGroupBox );

	QLabel* fpsTagLabel = new QLabel( m_infoControlsGroupBox );
	fpsTagLabel->setText( i18n( "<b>FPS</b>") );
	fpsTagLabel->installEventFilter( this );
	m_fpsLabel = new QLabel( m_infoControlsGroupBox );

	m_fpsLabel->setMinimumWidth( m_positionEdit->sizeHint().width() ); // sets the minimum width for the whole group

	m_volumeSlider = new PointingSlider( Qt::Vertical, this );
	m_volumeSlider->setFocusPolicy( Qt::NoFocus );
	m_volumeSlider->setTickPosition( QSlider::NoTicks );
// 	m_volumeSlider->setInvertedAppearance( true );
	m_volumeSlider->setTracking( true );
	m_volumeSlider->setPageStep( 5 );
	m_volumeSlider->setMinimum( 0 );
	m_volumeSlider->setMaximum( 100 );
	m_volumeSlider->setFocusPolicy( Qt::NoFocus );

	QGridLayout* videoControlsLayout = new QGridLayout();
	videoControlsLayout->setMargin( 0 );
	videoControlsLayout->setSpacing( 2 );
	videoControlsLayout->addWidget( createToolButton( this, ACT_PLAY_PAUSE, 16 ), 0, 0 );
	videoControlsLayout->addWidget( createToolButton( this, ACT_STOP, 16 ), 0, 1 );
	videoControlsLayout->addWidget( createToolButton( this, ACT_SEEK_BACKWARDS, 16 ), 0, 2 );
	videoControlsLayout->addWidget( createToolButton( this, ACT_SEEK_FORWARDS, 16 ), 0, 3 );
	videoControlsLayout->addItem( new QSpacerItem( 2, 2 ), 0, 4 );
	videoControlsLayout->addWidget( createToolButton( this, ACT_SEEK_TO_PREVIOUS_LINE, 16 ), 0, 5 );
	videoControlsLayout->addWidget( createToolButton( this, ACT_SEEK_TO_NEXT_LINE, 16 ), 0, 6 );
	videoControlsLayout->addItem( new QSpacerItem( 2, 2 ), 0, 7 );
	videoControlsLayout->addWidget( createToolButton( this, ACT_SET_CURRENT_LINE_SHOW_TIME, 16 ), 0, 8 );
	videoControlsLayout->addWidget( createToolButton( this, ACT_SET_CURRENT_LINE_HIDE_TIME, 16 ), 0, 9 );
	videoControlsLayout->addItem( new QSpacerItem( 2, 2 ), 0, 10 );
	videoControlsLayout->addWidget( createToolButton( this, ACT_CURRENT_LINE_FOLLOWS_VIDEO, 16 ), 0, 11 );
	videoControlsLayout->addWidget( m_seekSlider, 0, 12 );

	QGridLayout* audioControlsLayout = new QGridLayout();
	audioControlsLayout->setMargin( 0 );
	audioControlsLayout->addWidget( createToolButton( this, ACT_TOGGLE_MUTED, 16 ), 0, 0, Qt::AlignHCenter );
	audioControlsLayout->addWidget( m_volumeSlider, 1, 0, Qt::AlignHCenter );

	QGridLayout* infoControlsLayout = new QGridLayout( m_infoControlsGroupBox );
	infoControlsLayout->setSpacing( 5 );
	infoControlsLayout->addWidget( fpsTagLabel, 0, 0 );
	infoControlsLayout->addWidget( m_fpsLabel, 1, 0 );
	infoControlsLayout->addItem( new QSpacerItem( 1, 1, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding ), 2, 0 );
	infoControlsLayout->addWidget( lengthTagLabel, 3, 0 );
	infoControlsLayout->addWidget( m_lengthLabel, 4, 0 );
	infoControlsLayout->addItem( new QSpacerItem( 1, 1, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding ), 5, 0 );
	infoControlsLayout->addWidget( positionTagLabel, 6, 0 );
	infoControlsLayout->addWidget( m_positionLabel, 7, 0 );
	infoControlsLayout->addWidget( m_positionEdit, 8, 0 );

	m_mainLayout = new QGridLayout( this );
	m_mainLayout->setMargin( 0 );
	m_mainLayout->setSpacing( 5 );
	m_mainLayout->addWidget( m_infoControlsGroupBox, 0, 0, 2, 1 );
	m_mainLayout->addWidget( m_layeredWidget, 0, 1 );
	m_mainLayout->addLayout( audioControlsLayout, 0, 2 );
	m_mainLayout->addLayout( videoControlsLayout, 1, 1 );
	m_mainLayout->addWidget( createToolButton( this, ACT_TOGGLE_FULL_SCREEN, 16 ), 1, 2 );

	m_fullScreenControls = new AttachableWidget( AttachableWidget::Bottom, 4, m_layeredWidget );
	m_fullScreenControls->setAutoHide( true );
	m_layeredWidget->setWidgetMode( m_fullScreenControls, LayeredWidget::IgnoreResize );

	m_fsSeekSlider = new PointingSlider( Qt::Horizontal, m_fullScreenControls );
	m_fsSeekSlider->setTickPosition( QSlider::NoTicks );
	m_fsSeekSlider->setTracking( false );
	m_fsSeekSlider->setMinimum( 0 );
	m_fsSeekSlider->setMaximum( 1000 );
	m_fsSeekSlider->setPageStep( 10 );

	m_fsVolumeSlider = new PointingSlider( Qt::Horizontal, m_fullScreenControls );
	m_fsVolumeSlider->setFocusPolicy( Qt::NoFocus );
	m_fsVolumeSlider->setTickPosition( QSlider::NoTicks );
	m_fsVolumeSlider->setTracking( true );
	m_fsVolumeSlider->setPageStep( 5 );
	m_fsVolumeSlider->setMinimum( 0 );
	m_fsVolumeSlider->setMaximum( 100 );

	m_fsPositionLabel = new QLabel( m_fullScreenControls );
	QPalette fsPositionPalette;
	fsPositionPalette.setColor( m_fsPositionLabel->backgroundRole(), Qt::black );
	fsPositionPalette.setColor( m_fsPositionLabel->foregroundRole(), Qt::white );
	m_fsPositionLabel->setPalette( fsPositionPalette );
	m_fsPositionLabel->setAutoFillBackground( true );
    m_fsPositionLabel->setFrameShape( QFrame::Panel );
	m_fsPositionLabel->setText( Time().toString( false ) + " /  " + Time().toString( false ) );
	m_fsPositionLabel->setAlignment( Qt::AlignRight|Qt::AlignVCenter );
	m_fsPositionLabel->adjustSize();
	m_fsPositionLabel->setMinimumWidth( m_fsPositionLabel->width() );

	QHBoxLayout* fullScreenControlsLayout = new QHBoxLayout( m_fullScreenControls );
	fullScreenControlsLayout->setMargin( 0 );
	fullScreenControlsLayout->setSpacing( 0 );

	fullScreenControlsLayout->addWidget( createToolButton( m_fullScreenControls, ACT_PLAY_PAUSE, 32 ) );
	fullScreenControlsLayout->addWidget( createToolButton( m_fullScreenControls, ACT_STOP, 32 ) );
	fullScreenControlsLayout->addWidget( createToolButton( m_fullScreenControls, ACT_SEEK_BACKWARDS, 32 ) );
	fullScreenControlsLayout->addWidget( createToolButton( m_fullScreenControls, ACT_SEEK_FORWARDS, 32 ) );
	fullScreenControlsLayout->addSpacing( 3 );
	fullScreenControlsLayout->addWidget( createToolButton( m_fullScreenControls, ACT_SEEK_TO_PREVIOUS_LINE, 32 ) );
	fullScreenControlsLayout->addWidget( createToolButton( m_fullScreenControls, ACT_SEEK_TO_NEXT_LINE, 32 ) );
	fullScreenControlsLayout->addSpacing( 3 );
	fullScreenControlsLayout->addWidget( m_fsSeekSlider, 9 );
// 	fullScreenControlsLayout->addSpacing( 1 );
	fullScreenControlsLayout->addWidget( m_fsPositionLabel );
	fullScreenControlsLayout->addWidget( m_fsVolumeSlider, 2 );
	fullScreenControlsLayout->addWidget( createToolButton( m_fullScreenControls, ACT_TOGGLE_MUTED, 32 ) );
	fullScreenControlsLayout->addWidget( createToolButton( m_fullScreenControls, ACT_TOGGLE_FULL_SCREEN, 32 ) );
	m_fullScreenControls->adjustSize();

	connect( m_volumeSlider, SIGNAL( valueChanged(int) ), this, SLOT(onVolumeSliderValueChanged(int)) );
	connect( m_fsVolumeSlider, SIGNAL( valueChanged(int) ), this, SLOT(onVolumeSliderValueChanged(int)) );

	connect( m_seekSlider, SIGNAL( valueChanged(int) ), this, SLOT(onSeekSliderValueChanged(int)) );
	connect( m_seekSlider, SIGNAL( sliderMoved(int) ), this, SLOT(onSeekSliderMoved(int)) );
	connect( m_seekSlider, SIGNAL( sliderPressed() ), this, SLOT(onSeekSliderPressed()) );
	connect( m_seekSlider, SIGNAL( sliderReleased() ), this, SLOT(onSeekSliderReleased()) );
	connect( m_fsSeekSlider, SIGNAL( valueChanged(int) ), this, SLOT(onSeekSliderValueChanged(int)) );
	connect( m_fsSeekSlider, SIGNAL( sliderMoved(int) ), this, SLOT(onSeekSliderMoved(int)) );
	connect( m_fsSeekSlider, SIGNAL( sliderPressed() ), this, SLOT(onSeekSliderPressed()) );
	connect( m_fsSeekSlider, SIGNAL( sliderReleased() ), this, SLOT(onSeekSliderReleased()) );

	connect( m_positionEdit, SIGNAL( valueChanged(int) ), this, SLOT(onPositionEditValueChanged(int)) );
	connect( m_positionEdit, SIGNAL( valueEntered(int) ), this, SLOT(onPositionEditValueChanged(int)) );

	connect( app()->playerConfig(), SIGNAL(optionChanged(const QString&,const QString&)),
			 this, SLOT(onPlayerOptionChanged(const QString&,const QString&)) );

	// Connect PlayerWidget observer slots to Player signals
	connect( m_player, SIGNAL(fileOpened(const QString&)), this, SLOT(onPlayerFileOpened(const QString&)) );
	connect( m_player, SIGNAL(fileOpenError(const QString&)), this, SLOT(onPlayerFileOpenError(const QString&)) );
	connect( m_player, SIGNAL(fileClosed()), this, SLOT(onPlayerFileClosed()) );
	connect( m_player, SIGNAL(playbackError()), this, SLOT(onPlayerPlaybackError()) );
	connect( m_player, SIGNAL(playing()), this, SLOT(onPlayerPlaying()) );
	connect( m_player, SIGNAL(stopped()), this, SLOT(onPlayerStopped()) );
	connect( m_player, SIGNAL(positionChanged(double)), this, SLOT(onPlayerPositionChanged(double)) );
	connect( m_player, SIGNAL(lengthChanged(double)), this, SLOT(onPlayerLengthChanged(double)) );
	connect( m_player, SIGNAL(framesPerSecondChanged(double)), this, SLOT(onPlayerFramesPerSecondChanged(double)) );
	connect( m_player, SIGNAL(volumeChanged(double)), this, SLOT(onPlayerVolumeChanged(double)) );

	connect( m_player, SIGNAL(leftClicked(const QPoint&)), this, SLOT(onPlayerLeftClicked(const QPoint&)) );
	connect( m_player, SIGNAL(rightClicked(const QPoint&)), this, SLOT(onPlayerRightClicked(const QPoint&)) );
	connect( m_player, SIGNAL(doubleClicked(const QPoint&)), this, SLOT(onPlayerDoubleClicked(const QPoint&)) );

	setOverlayLine( 0 );
	onPlayerFileClosed();
	onPlayerOptionChanged( QString(), QString() ); // initializes the font
}

PlayerWidget::~PlayerWidget()
{
	m_player->finalize();
}

QToolButton* PlayerWidget::toolButton( QWidget* parent, const char* name )
{
	return parent->findChild<QToolButton*>( name );
}

QToolButton* PlayerWidget::createToolButton( QWidget* parent, const char* name, int size )
{
	QToolButton* toolButton = new QToolButton( parent );
	toolButton->setObjectName( name );
	toolButton->setMinimumSize( size, size );
	toolButton->setAutoRaise( true );
	toolButton->setFocusPolicy( Qt::NoFocus );
	return toolButton;
}

void PlayerWidget::loadConfig()
{
	onPlayerVolumeChanged( m_player->volume() );
}


void PlayerWidget::saveConfig()
{
}

bool PlayerWidget::fullScreenMode() const
{
	return m_fullScreenMode;
}

void PlayerWidget::setFullScreenMode( bool fullScreenMode )
{
	static int timerID = 0;

	if ( m_fullScreenMode != fullScreenMode )
	{
		m_fullScreenMode = fullScreenMode;

		if ( m_fullScreenMode )
		{
			increaseFontSize( 13 );

			window()->hide();
			m_layeredWidget->setParent( 0 ); // removes the widget from this window (and this m_mainLayout)
			m_layeredWidget->showFullScreen(); // krazy:exclude=c++/qmethods

			m_fullScreenControls->attach( m_layeredWidget );

			timerID = startTimer( HIDE_MOUSE_MSECS );
		}
		else
		{
			if ( timerID )
			{
				killTimer( timerID );
				timerID = 0;
			}

			decreaseFontSize( 13 );

			m_fullScreenControls->dettach();

			window()->show();
			m_layeredWidget->setParent( this );
			m_layeredWidget->showNormal(); // krazy:exclude=c++/qmethods

			m_mainLayout->addWidget( m_layeredWidget, 0, 1 );
		}
	}
}

SubtitleLine* PlayerWidget::playingLine()
{
	return m_playingLine;
}

SubtitleLine* PlayerWidget::overlayLine()
{
	return m_overlayLine;
}

void PlayerWidget::plugActions()
{
	toolButton( this, ACT_STOP )->setDefaultAction( app()->action( ACT_STOP ) );
	toolButton( this, ACT_PLAY_PAUSE )->setDefaultAction( app()->action( ACT_PLAY_PAUSE ) );
	toolButton( this, ACT_SEEK_BACKWARDS )->setDefaultAction( app()->action( ACT_SEEK_BACKWARDS ) );
	toolButton( this, ACT_SEEK_FORWARDS )->setDefaultAction( app()->action( ACT_SEEK_FORWARDS ) );
	toolButton( this, ACT_SEEK_TO_PREVIOUS_LINE )->setDefaultAction( app()->action( ACT_SEEK_TO_PREVIOUS_LINE ) );
	toolButton( this, ACT_SEEK_TO_NEXT_LINE )->setDefaultAction( app()->action( ACT_SEEK_TO_NEXT_LINE ) );
	toolButton( this, ACT_SET_CURRENT_LINE_SHOW_TIME )->setDefaultAction( app()->action( ACT_SET_CURRENT_LINE_SHOW_TIME ) );
	toolButton( this, ACT_SET_CURRENT_LINE_HIDE_TIME )->setDefaultAction( app()->action( ACT_SET_CURRENT_LINE_HIDE_TIME ) );
	toolButton( this, ACT_CURRENT_LINE_FOLLOWS_VIDEO )->setDefaultAction( app()->action( ACT_CURRENT_LINE_FOLLOWS_VIDEO ) );
	toolButton( this, ACT_TOGGLE_MUTED )->setDefaultAction( app()->action( ACT_TOGGLE_MUTED ) );
	toolButton( this, ACT_TOGGLE_FULL_SCREEN )->setDefaultAction( app()->action( ACT_TOGGLE_FULL_SCREEN ) );

	toolButton( m_fullScreenControls, ACT_STOP )->setDefaultAction( app()->action( ACT_STOP ) );
	toolButton( m_fullScreenControls, ACT_PLAY_PAUSE )->setDefaultAction( app()->action( ACT_PLAY_PAUSE ) );
	toolButton( m_fullScreenControls, ACT_SEEK_BACKWARDS )->setDefaultAction( app()->action( ACT_SEEK_BACKWARDS ) );
	toolButton( m_fullScreenControls, ACT_SEEK_FORWARDS )->setDefaultAction( app()->action( ACT_SEEK_FORWARDS ) );
	toolButton( m_fullScreenControls, ACT_SEEK_TO_PREVIOUS_LINE )->setDefaultAction( app()->action( ACT_SEEK_TO_PREVIOUS_LINE ) );
	toolButton( m_fullScreenControls, ACT_SEEK_TO_NEXT_LINE )->setDefaultAction( app()->action( ACT_SEEK_TO_NEXT_LINE ) );
	toolButton( m_fullScreenControls, ACT_TOGGLE_MUTED )->setDefaultAction( app()->action( ACT_TOGGLE_MUTED ) );
	toolButton( m_fullScreenControls, ACT_TOGGLE_FULL_SCREEN )->setDefaultAction( app()->action( ACT_TOGGLE_FULL_SCREEN ) );
}

void PlayerWidget::timerEvent( QTimerEvent* /*event*/ )
{
	if ( m_cursorPos != m_lastCursorPos )
		m_lastCursorPos = m_cursorPos;
	else if ( m_layeredWidget->cursor().shape() != Qt::BlankCursor )
		m_layeredWidget->setCursor( QCursor( Qt::BlankCursor ) );
}

bool PlayerWidget::eventFilter( QObject* object, QEvent* event )
{
	if ( object == m_layeredWidget )
	{
		if ( event->type() == QEvent::DragEnter )
		{
			QDragEnterEvent* dragEnterEvent = static_cast<QDragEnterEvent*>( event );
			KUrl::List urls = KUrl::List::fromMimeData( dragEnterEvent->mimeData() );
			if ( ! urls.isEmpty() )
				dragEnterEvent->accept();
			else
				dragEnterEvent->ignore();
			return true;
		}
		else if ( event->type() == QEvent::DragMove )
		{
			return true; // eat event
		}
		else if ( event->type() == QEvent::Drop )
		{
			QDropEvent* dropEvent = static_cast<QDropEvent*>( event );

			KUrl::List urls = KUrl::List::fromMimeData( dropEvent->mimeData() );
			if ( ! urls.isEmpty() )
			{
				for ( KUrl::List::ConstIterator it = urls.begin(), end = urls.end(); it != end; ++it )
				{
					const KUrl& url = *it;

					if ( url.protocol() != "file" )
						continue;

					app()->openVideo( url );
					break;
				}
			}

			return true; // eat event
		}
		else if ( event->type() == QEvent::KeyPress )
		{
			// NOTE: when on full screen mode, the keyboard input is received but
			// for some reason it doesn't trigger the correct actions automatically
			// so we process the event and handle the issue ourselves.

			QKeyEvent* keyEvent = static_cast<QKeyEvent*>( event );
			app()->triggerAction( QKeySequence( (keyEvent->modifiers() & ~Qt::KeypadModifier) + keyEvent->key() ) );
			return true; // eat event
		}
		else if ( event->type() == QEvent::MouseMove )
		{
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>( event );
			m_cursorPos = mouseEvent->pos();

			if ( m_layeredWidget->cursor().shape() == Qt::BlankCursor )
				m_layeredWidget->setCursor( Qt::ArrowCursor );
		}
	}
	else if ( object == m_infoControlsGroupBox || object->parent() == m_infoControlsGroupBox )
	{
		if ( event->type() != QEvent::MouseButtonRelease )
			return QWidget::eventFilter( object, event );

		QMouseEvent* mouseEvent = static_cast<QMouseEvent*>( event );

		if ( mouseEvent->button() != Qt::RightButton )
			return QWidget::eventFilter( object, event );

		KMenu menu;
		QAction* action = menu.addAction( i18n( "Show editable position control" ) );
		action->setCheckable( true );
		action->setChecked( app()->playerConfig()->showPositionTimeEdit() );

		if ( menu.exec( mouseEvent->globalPos() ) == action )
			app()->playerConfig()->toggleShowPositionTimeEdit();

		return true; // eat event
	}

	return QWidget::eventFilter( object, event );
}

void PlayerWidget::setSubtitle( Subtitle* subtitle )
{
	if ( m_subtitle )
	{
		disconnect( m_subtitle, SIGNAL(linesInserted(int,int)), this, SLOT(invalidateOverlayLine()) );
		disconnect( m_subtitle, SIGNAL(linesRemoved(int,int)), this, SLOT(invalidateOverlayLine()) );

		m_subtitle = 0;	// has to be set to 0 for invalidateOverlayLine

		invalidateOverlayLine();
		setPlayingLine( 0 );
	}

	m_subtitle = subtitle;

	if ( m_subtitle )
	{
		connect( m_subtitle, SIGNAL(linesInserted(int,int)), this, SLOT(invalidateOverlayLine()) );
		connect( m_subtitle, SIGNAL(linesRemoved(int,int)), this, SLOT(invalidateOverlayLine()) );
	}
}


void PlayerWidget::setTranslationMode( bool enabled )
{
	m_translationMode = enabled;

	if ( ! m_translationMode )
		setShowTranslation( false );
}

void PlayerWidget::setShowTranslation( bool showTranslation )
{
	if ( m_showTranslation != showTranslation )
	{
		m_showTranslation = showTranslation;

		invalidateOverlayLine();
		setPlayingLine( 0 );
	}
}

void PlayerWidget::increaseFontSize( int points )
{
	app()->playerConfig()->incFontPointSize( points );
}

void PlayerWidget::decreaseFontSize( int points )
{
	app()->playerConfig()->incFontPointSize( -points );
}

void PlayerWidget::updateOverlayLine( const Time& videoPosition )
{
	if ( ! m_subtitle )
		return;

	bool seekedBackwards = m_lastCheckedTime > videoPosition;
	m_lastCheckedTime = videoPosition;

	if ( m_overlayLine )
	{
		if ( ! seekedBackwards && videoPosition <= m_overlayLine->hideTime() )
		{
			// m_overlayLine is the line to show or the next line to show
			if ( videoPosition >= m_overlayLine->showTime() ) // m_overlayLine is the line to show
			{
				const SString& text = m_showTranslation ? m_overlayLine->secondaryText() : m_overlayLine->primaryText();
				m_textOverlay->setText( text.richString( SString::Verbose ) );
				setPlayingLine( m_overlayLine );
			}
			return;
		}
		else
		{
			// m_overlayLine is no longer the line to show nor the next line to show
			m_textOverlay->setText( QString() );

			setOverlayLine( 0 );
		}
	}

	if ( seekedBackwards || m_lastSearchedLineToShowTime > videoPosition )
	{
		// search the next line to show
		for ( SubtitleIterator it( *m_subtitle ); it.current(); ++it )
		{
			if ( videoPosition <= it.current()->hideTime() )
			{
				m_lastSearchedLineToShowTime = videoPosition;

				setOverlayLine( it.current() );

				if ( m_overlayLine->showTime() <= videoPosition && videoPosition <= m_overlayLine->hideTime() )
				{
					const SString& text = m_showTranslation ? m_overlayLine->secondaryText() : m_overlayLine->primaryText();
					m_textOverlay->setText( text.richString( SString::Verbose ) );
					setPlayingLine( m_overlayLine );
				}
				return;
			}
		}
		setPlayingLine( 0 );
	}
}

void PlayerWidget::invalidateOverlayLine()
{
	m_textOverlay->setText( QString() );

	setOverlayLine( 0 );

	if ( m_player->position() >= 0.0 )
		updateOverlayLine( (long)(m_player->position()*1000) );
}

void PlayerWidget::setOverlayLine( SubtitleLine* line )
{
	if ( m_overlayLine )
	{
		disconnect( m_overlayLine, SIGNAL(showTimeChanged(const Time&)), this, SLOT(invalidateOverlayLine()) );
		disconnect( m_overlayLine, SIGNAL(hideTimeChanged(const Time&)), this, SLOT(invalidateOverlayLine()) );
	}

	m_overlayLine = line;

	if ( m_overlayLine )
	{
		connect( m_overlayLine, SIGNAL(showTimeChanged(const Time&)), this, SLOT(invalidateOverlayLine()) );
		connect( m_overlayLine, SIGNAL(hideTimeChanged(const Time&)), this, SLOT(invalidateOverlayLine()) );
	}
	else
		m_lastSearchedLineToShowTime = Time::MaxMseconds;
}

void PlayerWidget::setPlayingLine( SubtitleLine* line )
{
	if ( ! line || m_playingLine != line )
	{
		m_playingLine = line;
		emit playingLineChanged( m_playingLine );
	}
}

void PlayerWidget::updatePositionEditVisibility()
{
	if ( m_showPositionTimeEdit && (m_player->state() == Player::Playing || m_player->state() == Player::Paused) )
		m_positionEdit->show();
	else
		m_positionEdit->hide();
}

void PlayerWidget::onVolumeSliderValueChanged( int value )
{
	if ( m_updatePlayerVolume )
	{
		m_updatePlayerVolume = false;
		m_updateVolumeControls = false;

		if ( sender() == m_fsVolumeSlider )
			m_volumeSlider->setValue( value );
		else
			m_fsVolumeSlider->setValue( value );

		m_player->setVolume( value );

		m_updateVolumeControls = true;
		m_updatePlayerVolume = true;
	}
}

void PlayerWidget::onSeekSliderPressed()
{
	m_updatePositionControls = 0;
}

void PlayerWidget::onSeekSliderReleased()
{
	m_updatePositionControls = MAGIC_NUMBER;
}

void PlayerWidget::onSeekSliderValueChanged( int value )
{
	if ( m_updateVideoPosition )
	{
		m_updatePositionControls = MAGIC_NUMBER;
		m_player->seek( m_player->length()*value/1000.0, true );
	}
}

void PlayerWidget::onSeekSliderMoved( int value )
{
	m_player->seek( m_player->length()*value/1000.0, false );

	Time time( (long)(m_player->length()*value) );

	m_positionLabel->setText( time.toString() );
	m_fsPositionLabel->setText( time.toString( false ) + m_lengthString );

	if ( m_showPositionTimeEdit )
		m_positionEdit->setValue( time.toMillis() );
}

void PlayerWidget::onPositionEditValueChanged( int position )
{
	if ( m_positionEdit->hasFocus() )
	{
		m_updatePositionControls = MAGIC_NUMBER;
		m_player->seek( position/1000.0, true );
	}
}

void PlayerWidget::onPlayerOptionChanged( const QString& option, const QString& value )
{
	if ( option == PlayerConfig::keyBackend() )
	{
		m_player->setActiveBackend( value );
	}
	else if ( option == PlayerConfig::keyShowPositionTimeEdit() )
	{
		m_showPositionTimeEdit = (value == "true");
		updatePositionEditVisibility();
	}
	else
	{
		m_textOverlay->setPrimaryColor( app()->playerConfig()->fontColor() );
		m_textOverlay->setFamily( app()->playerConfig()->fontFamily() );
		m_textOverlay->setPointSize( app()->playerConfig()->fontPointSize() );
		m_textOverlay->setOutlineColor( app()->playerConfig()->outlineColor() );
		m_textOverlay->setOutlineWidth( app()->playerConfig()->outlineWidth() );
	}
}

void PlayerWidget::onPlayerFileOpened( const QString& /*filePath*/ )
{
	m_infoControlsGroupBox->setEnabled( true );

	updatePositionEditVisibility();

	m_positionLabel->setText( i18n( "<i>Unknown</i>" ) );
	m_lengthLabel->setText( i18n( "<i>Unknown</i>" ) );
	m_fpsLabel->setText( i18n( "<i>Unknown</i>" ) );

	m_lengthString = UNKNOWN_LENGTH_STRING;

	m_fsPositionLabel->setText( Time().toString( false ) + m_lengthString );
}

void PlayerWidget::onPlayerFileOpenError( const QString& filePath )
{
	KMessageBox::sorry( this, i18n( "<qt>There was an error opening file<br/>%1</qt>", filePath ) );
}

void PlayerWidget::onPlayerFileClosed()
{
	m_lastCheckedTime = 0;

	m_infoControlsGroupBox->setEnabled( false );

	updatePositionEditVisibility();
	m_positionEdit->setValue( 0 );

 	m_positionLabel->setText( QString() );
	m_lengthLabel->setText( QString() );
	m_fpsLabel->setText( QString() );

	m_lengthString = UNKNOWN_LENGTH_STRING;

	m_fsPositionLabel->setText( Time().toString( false ) + m_lengthString );

	m_seekSlider->setEnabled( false );
	m_fsSeekSlider->setEnabled( false );
}

void PlayerWidget::onPlayerPlaybackError()
{
	KMessageBox::sorry( this, i18n( "Unexpected error while playing file." ) );
	//onPlayerFileClosed();
}

void PlayerWidget::onPlayerPlaying()
{
	m_seekSlider->setEnabled( true );
	m_fsSeekSlider->setEnabled( true );

	updatePositionEditVisibility();
}

void PlayerWidget::onPlayerPositionChanged( double seconds )
{
	if ( m_updatePositionControls > 0 )
	{
		if ( seconds >= 0 )
		{
			Time videoPosition( (long)(seconds*1000) );

			m_positionLabel->setText( videoPosition.toString() );
			m_fsPositionLabel->setText( videoPosition.toString( false ) + m_lengthString );

			if ( m_showPositionTimeEdit && ! m_positionEdit->hasFocus() )
				m_positionEdit->setValue( videoPosition.toMillis() );

			updateOverlayLine( videoPosition );

			int sliderValue = (int)((seconds/m_player->length())*1000);

			m_updateVideoPosition = false;
			m_seekSlider->setValue( sliderValue );
			m_fsSeekSlider->setValue( sliderValue );
			m_updateVideoPosition = true;
		}
		else
		{
			m_positionLabel->setText( i18n( "<i>Unknown</i>" ) );
			m_fsPositionLabel->setText( Time().toString( false ) + m_lengthString );
		}
	}
 	else if ( m_updatePositionControls < 0 )
		m_updatePositionControls += 2;
}

void PlayerWidget::onPlayerLengthChanged( double seconds )
{
	if ( seconds > 0 )
	{
		m_lengthLabel->setText( Time( (long)(seconds*1000) ).toString() );
		m_lengthString = " / " + m_lengthLabel->text().left( 8 ) + ' ';
	}
	else
	{
		m_lengthLabel->setText( i18n( "<i>Unknown</i>" ) );
		m_lengthString = UNKNOWN_LENGTH_STRING;
	}
}

void PlayerWidget::onPlayerFramesPerSecondChanged( double fps )
{
	m_fpsLabel->setText( fps > 0 ? QString::number( fps, 'f', 3 ) : i18n( "<i>Unknown</i>" ) );
}

void PlayerWidget::onPlayerStopped()
{
	onPlayerPositionChanged( 0 );

	m_seekSlider->setEnabled( false );
	m_fsSeekSlider->setEnabled( false );

	setPlayingLine( 0 );

	updatePositionEditVisibility();
}

void PlayerWidget::onPlayerVolumeChanged( double volume )
{
	if ( m_updateVolumeControls )
	{
		m_updatePlayerVolume = false;
		m_volumeSlider->setValue( (int)(volume + 0.5) );
		m_fsVolumeSlider->setValue( (int)(volume + 0.5) );
		m_updatePlayerVolume = true;
	}
}

void PlayerWidget::onPlayerLeftClicked( const QPoint& /*point*/ )
{
	m_player->togglePlayPaused();
}

void PlayerWidget::onPlayerRightClicked( const QPoint& point )
{
	static KMenu* menu = new KMenu( this );

	menu->clear();

	menu->addAction( app()->action( ACT_OPEN_VIDEO ) );
	menu->addAction( app()->action( ACT_CLOSE_VIDEO ) );

	menu->addSeparator();

	menu->addAction( app()->action( ACT_TOGGLE_FULL_SCREEN ) );

	menu->addSeparator();

	menu->addAction( app()->action( ACT_STOP ) );
	menu->addAction( app()->action( ACT_PLAY_PAUSE ) );
	menu->addAction( app()->action( ACT_SEEK_BACKWARDS ) );
	menu->addAction( app()->action( ACT_SEEK_FORWARDS ) );

	menu->addSeparator();

	menu->addAction( app()->action( ACT_SEEK_TO_PREVIOUS_LINE ) );
	menu->addAction( app()->action( ACT_SEEK_TO_NEXT_LINE ) );

	menu->addSeparator();

	menu->addAction( app()->action( ACT_SET_ACTIVE_AUDIO_STREAM ) );
	menu->addAction( app()->action( ACT_INCREASE_VOLUME ) );
	menu->addAction( app()->action( ACT_DECREASE_VOLUME ) );
	menu->addAction( app()->action( ACT_TOGGLE_MUTED ) );

	menu->addSeparator();

	if ( m_translationMode )
		menu->addAction( app()->action( ACT_SET_ACTIVE_SUBTITLE_STREAM ) );

	menu->addAction( app()->action( ACT_INCREASE_SUBTITLE_FONT ) );
	menu->addAction( app()->action( ACT_DECREASE_SUBTITLE_FONT ) );

	// NOTE do not use popup->exec() here!!! it freezes the application
	// when using the mplayer backend. i think it's related to the fact
	// that exec() creates a different event loop and the mplayer backend
	// depends on the main loop for catching synchronization signals
	menu->popup( point );
}

void PlayerWidget::onPlayerDoubleClicked( const QPoint& /*point*/ )
{
	app()->toggleFullScreenMode();
}

void PlayerWidget::onPlayerBackendInitialized()
{
	// NOTE when the player backend is initialized the video widget
	// is created in front of the text overlay, so we have to raise
	// it to make it visible again
	m_textOverlay->raise();
}

#include "playerwidget.moc"
