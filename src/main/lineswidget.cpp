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

#include "lineswidget.h"
#include "application.h"
#include "configs/generalconfig.h"
#include "dialogs/actionwithtargetdialog.h"
#include "profiler.h"

#include <QtCore/QVariant>
#include <QtCore/QTimer>
#include <QtCore/QEvent>
#include <QtGui/QScrollBar>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QPainter>
#include <QtGui/QFontMetrics>
#include <QtGui/QTextDocument>
#include <QtGui/QAbstractTextDocumentLayout>
#include <QtGui/QHeaderView>

#include <KDebug>
#include <KLocale>
#include <KConfig>
#include <KConfigGroup>
#include <KMenu>
#include <KUrl>

#if QT_VERSION >= 0x40500
	#define TEXT_DOCUMENT_CORRECTION 4
#else
	#define TEXT_DOCUMENT_CORRECTION 2
#endif

using namespace SubtitleComposer;

/// LINES MODEL
/// ===========

LinesModel::LinesModel( QObject* parent ):
	QAbstractListModel( parent ),
	m_subtitle( 0 ),
	m_playingLine( 0 ),
	m_dataChangedTimer( new QTimer( this ) ),
	m_minChangedLineIndex( -1 ),
	m_maxChangedLineIndex( -1 )
{
	m_dataChangedTimer->setInterval( 0 );
	m_dataChangedTimer->setSingleShot( true );

	connect( m_dataChangedTimer, SIGNAL( timeout() ), this, SLOT( emitDataChanged() ) );
}

Subtitle* LinesModel::subtitle() const
{
	return m_subtitle;
}

void LinesModel::setSubtitle( Subtitle* subtitle )
{
	if ( m_subtitle != subtitle )
	{
		m_playingLine = 0;

		if ( m_subtitle )
		{
/*			disconnect( m_subtitle, SIGNAL( linesAboutToBeInserted( int, int ) ),
						this, SLOT( onLinesAboutToBeInserted( int, int ) ) );*/
			disconnect( m_subtitle, SIGNAL( linesInserted( int, int ) ),
						this, SLOT( onLinesInserted( int, int ) ) );
/*			disconnect( m_subtitle, SIGNAL( linesAboutToBeRemoved( int, int ) ),
						this, SLOT( onLinesAboutToBeRemoved( int, int ) ) );*/
			disconnect( m_subtitle, SIGNAL( linesRemoved( int, int ) ),
						this, SLOT( onLinesRemoved( int, int ) ) );

			disconnect( m_subtitle, SIGNAL( lineErrorFlagsChanged( SubtitleLine*, int ) ),
						this, SLOT( onLineChanged( SubtitleLine* ) ) );
			disconnect( m_subtitle, SIGNAL( linePrimaryTextChanged( SubtitleLine*, const SString& ) ),
						this, SLOT( onLineChanged( SubtitleLine* ) ) );
			disconnect( m_subtitle, SIGNAL( lineSecondaryTextChanged( SubtitleLine*, const SString& ) ),
						this, SLOT( onLineChanged( SubtitleLine* ) ) );
			disconnect( m_subtitle, SIGNAL( lineShowTimeChanged( SubtitleLine*, const Time& ) ),
						this, SLOT( onLineChanged( SubtitleLine* ) ) );
			disconnect( m_subtitle, SIGNAL( lineHideTimeChanged( SubtitleLine*, const Time& ) ),
						this, SLOT( onLineChanged( SubtitleLine* ) ) );

			if ( m_subtitle->linesCount() )
			{
// 				onLinesAboutToBeRemoved( 0, m_subtitle->linesCount() - 1 );
				onLinesRemoved( 0, m_subtitle->linesCount() - 1 );
			}
		}

		m_subtitle = subtitle;

		if ( m_subtitle )
		{
			if ( m_subtitle->linesCount() )
			{
// 				onLinesAboutToBeInserted( 0, m_subtitle->linesCount() - 1 );
				onLinesInserted( 0, m_subtitle->linesCount() - 1 );
			}

/*			connect( m_subtitle, SIGNAL( linesAboutToBeInserted( int, int ) ),
					 this, SLOT( onLinesAboutToBeInserted( int, int ) ) );*/
			connect( m_subtitle, SIGNAL( linesInserted( int, int ) ),
					 this, SLOT( onLinesInserted( int, int ) ) );
/*			connect( m_subtitle, SIGNAL( linesAboutToBeRemoved( int, int ) ),
					 this, SLOT( onLinesAboutToBeRemoved( int, int ) ) );*/
			connect( m_subtitle, SIGNAL( linesRemoved( int, int ) ),
					 this, SLOT( onLinesRemoved( int, int ) ) );

			connect( m_subtitle, SIGNAL( lineErrorFlagsChanged( SubtitleLine*, int ) ),
					 this, SLOT( onLineChanged( SubtitleLine* ) ) );
			connect( m_subtitle, SIGNAL( linePrimaryTextChanged( SubtitleLine*, const SString& ) ),
					 this, SLOT( onLineChanged( SubtitleLine* ) ) );
			connect( m_subtitle, SIGNAL( lineSecondaryTextChanged( SubtitleLine*, const SString& ) ),
					 this, SLOT( onLineChanged( SubtitleLine* ) ) );
			connect( m_subtitle, SIGNAL( lineShowTimeChanged( SubtitleLine*, const Time& ) ),
					 this, SLOT( onLineChanged( SubtitleLine* ) ) );
			connect( m_subtitle, SIGNAL( lineHideTimeChanged( SubtitleLine*, const Time& ) ),
					 this, SLOT( onLineChanged( SubtitleLine* ) ) );
		}
	}
}

SubtitleLine* LinesModel::playingLine() const
{
	return m_playingLine;
}

void LinesModel::setPlayingLine( SubtitleLine* line )
{
	if ( m_playingLine != line )
	{
		if ( m_playingLine )
		{
			int row = m_playingLine->index();
			m_playingLine = 0;
			emit dataChanged( index( row, 0 ), index( row, ColumnCount ) );
		}

		m_playingLine = line;

		if ( line )
		{
			int row = m_playingLine->index();
			emit dataChanged( index( row, 0 ), index( row, ColumnCount ) );
		}
	}
}


int LinesModel::rowCount( const QModelIndex& /*parent*/ ) const
{
	return m_subtitle ? m_subtitle->linesCount() : 0;
}

int LinesModel::columnCount( const QModelIndex& /*parent*/ ) const
{
	return ColumnCount;
}

Qt::ItemFlags LinesModel::flags( const QModelIndex& index ) const
{
	if ( ! index.isValid() || index.column() < Text )
		return QAbstractItemModel::flags( index );

	return QAbstractItemModel::flags( index ) | Qt::ItemIsEditable;
}

QString LinesModel::buildToolTip( SubtitleLine* line, bool primary )
{
	int errorFlags = line->errorFlags();
	if ( primary )
		errorFlags &= ~SubtitleLine::SecondaryOnlyErrors;
	else
		errorFlags &= ~SubtitleLine::PrimaryOnlyErrors;

	const SString& text = primary? line->primaryText() : line->secondaryText();

	if ( errorFlags )
	{
		QString toolTip = "<p style='white-space:pre;margin-bottom:6px;'>" + text.richString() + "</p><p style='white-space:pre;margin-top:0px;'>";

		if ( errorFlags )
		{
			toolTip += i18n( "<b>Observations:</b>" );

			for ( int id = 0; id < SubtitleLine::ErrorSIZE; ++id )
			{
				if ( ! ((0x1 << id) & errorFlags) )
					continue;

				QString errorText = line->fullErrorText( (SubtitleLine::ErrorID)id );
				if ( ! errorText.isEmpty() )
					toolTip += "\n  - " + errorText;
			}
		}

		toolTip += "</p>";

		return toolTip;
	}
	else
		return text.richString();
}

QVariant LinesModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
	if ( role != Qt::DisplayRole || orientation == Qt::Vertical )
		return QVariant();

	switch ( section )
	{
		case 0:		return i18nc( "@title:column Subtitle line number", "Line" );
		case 1:		return i18nc( "@title:column", "Show Time" );
		case 2:		return i18nc( "@title:column", "Hide Time" );
		case 3:		return i18nc( "@title:column Subtitle line (primary) text", "Text" );
		case 4:		return i18nc( "@title:column Subtitle line translation text", "Translation" );
		default:	return QVariant();
	}
}

QVariant LinesModel::data( const QModelIndex& index, int role ) const
{
	if ( ! m_subtitle /*|| ! index.isValid() || index.row() >= rowCount() || index.column() >= ColumnCount*/ )
		return QVariant();

	SubtitleLine* line = m_subtitle->line( index.row() );

	if ( role == PlayingLineRole )
		return line == m_playingLine;

	switch ( index.column() )
	{
		case Number:
			if ( role == Qt::DisplayRole )
				return index.row() + 1;
			break;
		case ShowTime:
			//if ( role == Qt::DisplayRole || role == Qt::ToolTipRole )
			if ( role == Qt::DisplayRole )
				return line->showTime().toString();
			else if ( role == Qt::TextAlignmentRole )
				return Qt::AlignCenter;
			break;
		case HideTime:
			//if ( role == Qt::DisplayRole || role == Qt::ToolTipRole )
			if ( role == Qt::DisplayRole )
				return line->hideTime().toString();
			else if ( role == Qt::TextAlignmentRole )
				return Qt::AlignCenter;
			break;
		case Text:
			if ( role == Qt::DisplayRole )
				return line->primaryText().richString();
			else if ( role == MarkedRole )
				return line->errorFlags() & SubtitleLine::UserMark;
			else if ( role == ErrorRole )
				return  line->errorFlags() &
						((SubtitleLine::SharedErrors|SubtitleLine::PrimaryOnlyErrors) & ~SubtitleLine::UserMark);
			else if ( role == Qt::ToolTipRole )
				return buildToolTip( m_subtitle->line( index.row() ), true );
			else if ( role == Qt::EditRole )
				return m_subtitle->line( index.row() )->primaryText().richString().replace( '\n', '|' );
			break;
		case Translation:
			if ( role == Qt::DisplayRole )
				return m_subtitle->line( index.row() )->secondaryText().richString();
			else if ( role == MarkedRole )
				return line->errorFlags() & SubtitleLine::UserMark;
			else if ( role == ErrorRole )
				return  line->errorFlags() &
						((SubtitleLine::SharedErrors|SubtitleLine::SecondaryOnlyErrors) & ~SubtitleLine::UserMark);
			else if ( role == Qt::ToolTipRole )
				return buildToolTip( m_subtitle->line( index.row() ), false );
			else if ( role == Qt::EditRole )
				return m_subtitle->line( index.row() )->secondaryText().richString().replace( '\n', '|' );
			break;
		default:
			break;
	}

	return QVariant();
}

bool LinesModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
	if ( ! m_subtitle || ! index.isValid() )
		return false;

	switch ( index.column() )
	{
		case Text:
			if ( role == Qt::EditRole )
			{
				SString sstring;
				sstring.setRichString( value.toString().replace( '|', '\n' ) );
				m_subtitle->line( index.row() )->setPrimaryText( sstring );
				emit dataChanged( index, index );
				return true;
			}
			break;
		case Translation:
			if ( role == Qt::EditRole )
			{
				SString sstring;
				sstring.setRichString( value.toString().replace( '|', '\n' ) );
				m_subtitle->line( index.row() )->setSecondaryText( sstring );
				emit dataChanged( index, index );
				return true;
			}
			break;
		default:
			break;
	}

	return false;
}

void LinesModel::onLinesInserted( int firstIndex, int lastIndex )
{
	static const QModelIndex rootIndex;

	beginInsertRows( rootIndex, firstIndex, lastIndex );
	endInsertRows(); // ridiculously costly operation
}

void LinesModel::onLinesRemoved( int firstIndex, int lastIndex )
{
	static const QModelIndex rootIndex;

	beginRemoveRows( rootIndex, firstIndex, lastIndex );
	endRemoveRows();
}

void LinesModel::onLineChanged( SubtitleLine* line )
{
	int lineIndex = line->index();

	if ( m_minChangedLineIndex < 0 )
	{
		m_minChangedLineIndex = lineIndex;
		m_maxChangedLineIndex = lineIndex;
		m_dataChangedTimer->start();
	}
	else if ( lineIndex < m_minChangedLineIndex )
		m_minChangedLineIndex = lineIndex;
	else if ( lineIndex > m_maxChangedLineIndex )
		m_maxChangedLineIndex = lineIndex;
}

void LinesModel::emitDataChanged()
{
	if ( m_minChangedLineIndex < 0 )
		m_minChangedLineIndex = m_maxChangedLineIndex;
	else if ( m_maxChangedLineIndex < 0 )
		m_maxChangedLineIndex = m_minChangedLineIndex;

	emit dataChanged( index( m_minChangedLineIndex, 0 ), index( m_maxChangedLineIndex, ColumnCount-1 ) );

	m_minChangedLineIndex = -1;
	m_maxChangedLineIndex = -1;
}




/// ITEM DELEGATE
/// =============

LinesItemDelegate::LinesItemDelegate( bool useStyle, bool singleLineMode, bool richTextMode, LinesWidget* parent ):
	QStyledItemDelegate( parent ),
	m_useStyle( useStyle ),
	m_singleLineMode( singleLineMode ),
	m_textDocument( 0 )
{
	setRichTextMode( richTextMode );
}

LinesItemDelegate::~LinesItemDelegate()
{
	delete m_textDocument;
}

bool LinesItemDelegate::useStyle() const
{
	return m_useStyle;
}

void LinesItemDelegate::setUseStyle( bool useStyle )
{
	m_useStyle = useStyle;
}

bool LinesItemDelegate::singleLineMode() const
{
	return m_singleLineMode;
}

void LinesItemDelegate::setSingleLineMode( bool singleLineMode )
{
	m_singleLineMode = singleLineMode;
}

bool LinesItemDelegate::richTextMode() const
{
	return m_textDocument != 0;
}

void LinesItemDelegate::setRichTextMode( bool richTextMode )
{
	if ( richTextMode != (m_textDocument != 0) )
	{
		if ( richTextMode )
		{
			QTextOption defaultTextOption;
			defaultTextOption.setWrapMode( QTextOption::NoWrap );

			m_textDocument = new QTextDocument();
			m_textDocument->setDefaultTextOption( defaultTextOption );
			m_textDocument->setUndoRedoEnabled( false );
			m_textDocument->setDefaultStyleSheet(
				"p { display: inline; white-space: pre; vertical-align: baseline; margin-bottom: 0px; margin-top: 0px }"
			);
		}
		else
		{
			delete m_textDocument;
			m_textDocument = 0;
		}
	}
}

bool LinesItemDelegate::eventFilter( QObject* object, QEvent* event )
{
	QWidget *editor = qobject_cast<QWidget*>(object);
	if ( ! editor )
		return false;

	if ( event->type() == QEvent::KeyPress )
	{
		switch ( static_cast<QKeyEvent *>(event)->key() )
		{
			case Qt::Key_Tab:
				emit commitData( editor );
				emit closeEditor( editor, QAbstractItemDelegate::EditNextItem );
				return true;

			case Qt::Key_Backtab:
				emit commitData( editor );
				emit closeEditor( editor, QAbstractItemDelegate::EditPreviousItem );
				return true;

			case Qt::Key_Up:
				emit commitData( editor );
				emit closeEditor( editor, (QAbstractItemDelegate::EndEditHint)EditUpperItem );
				return true;

			case Qt::Key_Down:
				emit commitData( editor );
				emit closeEditor( editor, (QAbstractItemDelegate::EndEditHint)EditLowerItem );
				return true;

			default:
				break;
		}
	}

	return QStyledItemDelegate::eventFilter( object, event );
}

void LinesItemDelegate::drawBackgroundPrimitive( QPainter* painter, const QStyle* style, const QStyleOptionViewItemV4& option ) const
{
	if ( m_useStyle )
	{
		style->drawPrimitive( QStyle::PE_PanelItemViewItem, &option, painter, option.widget );
	}
	else
	{
		QPalette::ColorGroup cg = option.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;

		if ( cg == QPalette::Normal && ! (option.state & QStyle::State_Active) )
			cg = QPalette::Inactive;

		if ( option.showDecorationSelected && (option.state & QStyle::State_Selected) )
		{
			painter->fillRect( option.rect, option.palette.brush( cg, QPalette::Highlight ) );
		}
		else
		{
			if ( option.backgroundBrush.style() != Qt::NoBrush )
			{
				QPointF oldBO = painter->brushOrigin();
				painter->setBrushOrigin( option.rect.topLeft() );
				painter->fillRect( option.rect, option.backgroundBrush );
				painter->setBrushOrigin( oldBO );
			}

			if ( option.state & QStyle::State_Selected )
			{
				QRect textRect = style->subElementRect( QStyle::SE_ItemViewItemText, &option, option.widget );
				painter->fillRect( textRect, option.palette.brush( cg, QPalette::Highlight ) );
			}
		}
	}
}

void LinesItemDelegate::drawTextPrimitive( QPainter* painter, const QStyle* style, const QStyleOptionViewItemV4& option,
									   const QRect& rect, QPalette::ColorGroup cg ) const
{
	const int textMargin = style->pixelMetric( QStyle::PM_FocusFrameHMargin, 0, option.widget ) + 1;
	const int alignment = QStyle::visualAlignment( option.direction, option.displayAlignment );

	QRect textRect = rect.adjusted( textMargin, 0, -textMargin, 0 ); // remove width padding

	QString text = QString( option.text );

	QFontMetrics fm = painter->fontMetrics();
	text = fm.elidedText( option.text, option.textElideMode, textRect.width() );

	if ( m_textDocument )
	{
		QTextOption textOption;
		textOption.setTextDirection( option.direction );
		textOption.setAlignment( (Qt::Alignment)alignment );

		m_textDocument->setDefaultTextOption( textOption );
		m_textDocument->setTextWidth( textRect.width() - textMargin );
		m_textDocument->setHtml( "<p>" + text + "</p>" );

		QPalette palette( option.palette );
		palette.setColor(
			QPalette::Text,
			palette.color(
				cg,
				option.state & QStyle::State_Selected ?
					QPalette::HighlightedText :
					QPalette::Text
			)
		);

		QAbstractTextDocumentLayout::PaintContext context;
		context.palette = palette;

		painter->translate( textRect.x() - TEXT_DOCUMENT_CORRECTION, textRect.y() - TEXT_DOCUMENT_CORRECTION );
		m_textDocument->documentLayout()->draw( painter, context );
		painter->translate( -textRect.x() + TEXT_DOCUMENT_CORRECTION, -textRect.y() + TEXT_DOCUMENT_CORRECTION );
	}
	else
	{
		if ( option.state & QStyle::State_Selected )
			painter->setPen( option.palette.color( cg, QPalette::HighlightedText ) );
		else
			painter->setPen( option.palette.color( cg, QPalette::Text ) );

		painter->drawText( textRect, alignment, text );
	}
}

void LinesItemDelegate::paint( QPainter* painter, const QStyleOptionViewItem& opt, const QModelIndex& index ) const
{
	QStyleOptionViewItemV4 option = opt;
	initStyleOption( &option, index );

	const QWidget* widget = option.widget;
	const QStyle* style = widget ? widget->style() : QApplication::style();

	painter->save();
	painter->setClipRect( option.rect );

	QRect textRect = style->subElementRect( QStyle::SE_ItemViewItemText, &option, widget );

	QPalette::ColorGroup cg = option.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
	if ( cg == QPalette::Normal && ! (option.state & QStyle::State_Active) )
		cg = QPalette::Inactive;


	// draw the background
	if ( index.data( LinesModel::PlayingLineRole ).toBool() )
	{
		if ( option.state & QStyle::State_Selected )
		{
			option.backgroundBrush = option.palette.highlight().color().lighter( 125 );
			option.palette.setBrush( cg, QPalette::Highlight, option.backgroundBrush );
		}
		else
			option.backgroundBrush = option.palette.highlight().color().lighter( 165 );
	}
	drawBackgroundPrimitive( painter, style, option );


	// draw the icon(s)
	bool showMarkedIcon = index.data( LinesModel::MarkedRole ).toBool();
	bool showErrorIcon = index.data( LinesModel::ErrorRole ).toBool();

	if ( showMarkedIcon || showErrorIcon )
	{
		int iconSize = qMin( textRect.width(), textRect.height() ) - 3;
		QRect iconRect = QRect( textRect.x() + 2, textRect.y() + 1, iconSize, iconSize );

		QIcon::Mode mode = QIcon::Normal;
		if ( ! ( option.state & QStyle::State_Enabled ) )
			mode = QIcon::Disabled;
		else if ( option.state & QStyle::State_Selected )
			mode = QIcon::Selected;
		QIcon::State state = option.state & QStyle::State_Open ? QIcon::On : QIcon::Off;

		if ( showMarkedIcon )
		{
			//painter->drawPixmap( iconRect, markPixmap() );
			markIcon().paint( painter, iconRect, option.decorationAlignment, mode, state );
			textRect.setX( textRect.x() + iconSize + 2 );
			if ( showErrorIcon )
				iconRect.translate( iconSize + 2, 0 );
		}

		if ( showErrorIcon )
		{
 			//painter->drawPixmap( iconRect, errorPixmap() );
			errorIcon().paint( painter, iconRect, option.decorationAlignment, mode, state );
			textRect.setX( textRect.x() + iconSize + 2 );
		}
	}

	// draw the text
	if ( ! option.text.isEmpty() )
	{
		if ( option.state & QStyle::State_Editing )
		{
			painter->setPen( option.palette.color( cg, QPalette::Text ) );
			painter->drawRect( textRect.adjusted( 0, 0, -1, -1 ) );
		}

		drawTextPrimitive( painter, style, option, textRect, cg );
	}

	// draw the focus rect
	if ( option.state & QStyle::State_HasFocus )
	{
		QStyleOptionFocusRect frOption;
		frOption.QStyleOption::operator=( option );
		frOption.rect = style->subElementRect( QStyle::SE_ItemViewItemFocusRect, &option, widget );
		frOption.state |= QStyle::State_KeyboardFocusChange;
		frOption.state |= QStyle::State_Item;
		frOption.backgroundColor = option.palette.color(
			(option.state & QStyle::State_Enabled) ? QPalette::Normal : QPalette::Disabled,
			(option.state & QStyle::State_Selected) ? QPalette::Highlight : QPalette::Window
		);

		style->drawPrimitive( QStyle::PE_FrameFocusRect, &frOption, painter, widget );
	}

	painter->restore();
}

QString LinesItemDelegate::displayText( const QVariant& value, const QLocale& locale ) const
{
	static const QChar pipeChar( '|' );
	static const QChar newLineChar( QChar::LineSeparator );

    if ( value.type() == QVariant::String )
		return value.toString().replace( '\n', m_singleLineMode ? pipeChar : newLineChar );
	else
		return QStyledItemDelegate::displayText( value, locale );
}

const QIcon& LinesItemDelegate::markIcon()
{
	static QIcon markIcon;
	if ( markIcon.isNull() )
		markIcon = KIcon( "dialog-warning" );
	return markIcon;
}

const QIcon& LinesItemDelegate::errorIcon()
{
	static QIcon errorIcon;
	if ( errorIcon.isNull() )
		errorIcon = KIcon( "dialog-error" );
	return errorIcon;
}


const QPixmap& LinesItemDelegate::markPixmap()
{
	static QPixmap markPixmap;
	if ( markPixmap.isNull() )
		markPixmap = markIcon().pixmap( 13, 13 );
	return markPixmap;
}

const QPixmap& LinesItemDelegate::errorPixmap()
{
	static QPixmap errorPixmap;
	if ( errorPixmap.isNull() )
		errorPixmap = errorIcon().pixmap( 13, 13 );
	return errorPixmap;
}




/// LINES WIDGET
/// ============

LinesWidget::LinesWidget( QWidget* parent ):
	TreeView( parent ),
	m_translationMode( false ),
	m_showingContextMenu( false )
{
	setModel( new LinesModel( this ) );

	LinesItemDelegate* plainTextDelegate = new LinesItemDelegate( true, true, false, this );
	LinesItemDelegate* richTextDelegate = new LinesItemDelegate( true, true, true, this );
	for ( int column = 0, columnCount = model()->columnCount(); column < columnCount; ++column )
		setItemDelegateForColumn( column, column < LinesModel::Text ? plainTextDelegate : richTextDelegate );

	QHeaderView* header = this->header();
	header->setClickable( false );
	header->setMovable( false );
	header->setResizeMode( LinesModel::Number, QHeaderView::ResizeToContents );
	header->setResizeMode( LinesModel::ShowTime, QHeaderView::ResizeToContents );
	header->setResizeMode( LinesModel::HideTime, QHeaderView::ResizeToContents );
	header->setResizeMode( LinesModel::Text, QHeaderView::Interactive );
	header->setResizeMode( LinesModel::Translation, QHeaderView::Interactive );
	header->setSectionHidden( LinesModel::Translation, true );

	setUniformRowHeights( true );
	setItemsExpandable( false );
	setAutoExpandDelay( -1 );
	setExpandsOnDoubleClick( false );
	setRootIsDecorated( false );
	setAllColumnsShowFocus( true );
	setSortingEnabled( false );

	//setIconSize( QSize( 12, 12 ) );

	setSelectionMode( QAbstractItemView::ExtendedSelection );
	setSelectionBehavior( QAbstractItemView::SelectRows );

	m_gridPen.setColor( palette().mid().color().light( 120 ) );

	setAcceptDrops( true );
	viewport()->installEventFilter( this );

	connect( selectionModel(), SIGNAL( currentRowChanged( const QModelIndex&, const QModelIndex& ) ),
			 this, SLOT( onCurrentRowChanged() ) );
}

LinesWidget::~LinesWidget()
{
}

void LinesWidget::closeEditor( QWidget* editor, QAbstractItemDelegate::EndEditHint hint )
{
	LinesItemDelegate::ExtendedEditHint ehint = (LinesItemDelegate::ExtendedEditHint)hint;
	QModelIndex editorIndex = currentIndex();

	if ( ehint == LinesItemDelegate::EditUpperItem )
		TreeView::closeEditor( editor, QAbstractItemDelegate::EditPreviousItem );
	else if ( ehint == LinesItemDelegate::EditLowerItem )
		TreeView::closeEditor( editor, QAbstractItemDelegate::EditNextItem );
	else if ( m_translationMode )
	{
		if ( ehint == LinesItemDelegate::EditNextItem )
			TreeView::closeEditor(
				editor,
				editorIndex.column() != LinesModel::Text ?
					QAbstractItemDelegate::EditNextItem :
					QAbstractItemDelegate::NoHint
			);
		else if ( ehint == LinesItemDelegate::EditPreviousItem )
			TreeView::closeEditor(
				editor,
				editorIndex.column() == LinesModel::Text ?
					QAbstractItemDelegate::EditPreviousItem :
					QAbstractItemDelegate::NoHint
			);
		else
			TreeView::closeEditor( editor, hint );
	}
	else
		TreeView::closeEditor( editor, hint );

	switch ( ehint )
	{
		case LinesItemDelegate::NoHint:
		case LinesItemDelegate::SubmitModelCache:
		case LinesItemDelegate::RevertModelCache:
			break;
		case LinesItemDelegate::EditUpperItem:
			if ( editorIndex.row() > 0 )
				editCurrentLineInPlace( editorIndex.column() == LinesModel::Text );
			break;
		case LinesItemDelegate::EditLowerItem:
			if ( editorIndex.row() < model()->rowCount() - 1 )
				editCurrentLineInPlace( editorIndex.column() == LinesModel::Text );
			break;

		case LinesItemDelegate::EditPreviousItem:
			if ( editorIndex.row() > 0 || (m_translationMode && editorIndex.column() == LinesModel::Translation) )
				editCurrentLineInPlace( editorIndex.column() != LinesModel::Text );
			break;

		case LinesItemDelegate::EditNextItem:
			if ( editorIndex.row() < model()->rowCount() - 1 || (m_translationMode && editorIndex.column() == LinesModel::Text) )
				editCurrentLineInPlace( editorIndex.column() != LinesModel::Text );
			break;
	}
}

void LinesWidget::rowsAboutToBeRemoved( const QModelIndex& parent, int start, int end )
{
	TreeView::rowsAboutToBeRemoved( parent, start, end );

	selectionModel()->select(
		model()->index( end + 1, 0, parent ),
		QItemSelectionModel::Rows|QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Current
	);

	scrollTo( model()->index( end + 1, 0, parent ), QAbstractItemView::EnsureVisible );
}

void LinesWidget::rowsInserted( const QModelIndex& parent, int start, int end )
{
	TreeView::rowsInserted( parent, start, end );

	if ( model()->rowCount() != (end - start + 1) ) // there were other rows previously
	{
		selectionModel()->select(
			QItemSelection( model()->index( start, 0, parent ), model()->index( end, 0, parent ) ),
			QItemSelectionModel::Rows|QItemSelectionModel::ClearAndSelect
		);

		scrollTo( model()->index( start, 0, parent ), QAbstractItemView::EnsureVisible );
	}

	selectionModel()->setCurrentIndex(
		model()->index( start, 0, parent ),
		QItemSelectionModel::Rows|QItemSelectionModel::SelectCurrent
	);
}

void LinesWidget::editCurrentLineInPlace( bool primaryText )
{
	QModelIndex currentIndex = this->currentIndex();
	if ( currentIndex.isValid() )
	{
		currentIndex = model()->index(
			currentIndex.row(),
			primaryText || ! m_translationMode ?
				LinesModel::Text :
				LinesModel::Translation
		);

		setCurrentIndex( currentIndex );

		edit( currentIndex );
	}
}

bool LinesWidget::showingContextMenu()
{
	return m_showingContextMenu;
}

SubtitleLine* LinesWidget::currentLine() const
{
	QModelIndex currentIndex = this->currentIndex();
	return currentIndex.isValid() ? model()->subtitle()->line( currentIndex.row() ) : 0;
}

int LinesWidget::currentLineIndex() const
{
	QModelIndex currentIndex = this->currentIndex();
	return currentIndex.isValid() ? currentIndex.row() : -1;
}

int LinesWidget::firstSelectedIndex() const
{
	QItemSelectionModel* selection = selectionModel();
	for ( int row = 0, rowCount = model()->rowCount(); row < rowCount; ++row )
		if ( selection->isSelected( model()->index( row, 0 ) ) )
			return row;
	return -1;
}

int LinesWidget::lastSelectedIndex() const
{
	QItemSelectionModel* selection = selectionModel();
	for ( int row = model()->rowCount() - 1; row >= 0; --row )
		if ( selection->isSelected( model()->index( row, 0 ) ) )
			return row;
	return -1;
}

bool LinesWidget::selectionHasMultipleRanges() const
{
	int selectionRanges = 0;

	QItemSelectionModel* selection = selectionModel();

	int rangeFirstRow = -1, row = 0;
	for ( int rowCount = model()->rowCount(); row < rowCount; ++row )
	{
		if ( selection->isSelected( model()->index( row, 0 ) ) )
		{
			if ( rangeFirstRow == -1 ) // mark start of selected range
			{
				rangeFirstRow = row;

				selectionRanges++;
				if ( selectionRanges > 1 )
					break;
			}
		}
		else
		{
			if ( rangeFirstRow != -1 )
				rangeFirstRow = -1;
		}
	}

	return selectionRanges > 1;
}

RangeList LinesWidget::selectionRanges() const
{
	RangeList ranges;

	QItemSelectionModel* selection = selectionModel();

	int rangeFirstRow = -1, row = 0;
	for ( const int rowCount = model()->rowCount(); row < rowCount; ++row )
	{
		if ( selection->isSelected( model()->index( row, 0 ) ) )
		{
			if ( rangeFirstRow == -1 ) // mark start of selected range
				rangeFirstRow = row;
		}
		else
		{
			if ( rangeFirstRow != -1 )
			{
				ranges << Range( rangeFirstRow, row - 1 );
				rangeFirstRow = -1;
			}
		}
	}

	if ( rangeFirstRow != -1 )
		ranges << Range( rangeFirstRow, row - 1 );

	return ranges;
}

RangeList LinesWidget::targetRanges( int target ) const
{
	switch ( target )
	{
		case ActionWithTargetDialog::AllLines:
			return Range::full();
		case ActionWithTargetDialog::Selection:
			return selectionRanges();
		case ActionWithTargetDialog::FromSelected:
		{
			int index = firstSelectedIndex();
			return index < 0 ? RangeList() : Range::upper( index );
		}
		case ActionWithTargetDialog::UpToSelected:
		{
			int index = lastSelectedIndex();
			return index < 0 ? RangeList() : Range::lower( index );
		}
		default:
			return RangeList();
	}
}

void LinesWidget::loadConfig()
{
	KConfigGroup group( KGlobal::config()->group( "LinesWidget Settings" ) );

	QByteArray state;
	QStringList strState = group.readXdgListEntry( "Columns State", QString( "" ).split( ' ' ) );
	for ( QStringList::ConstIterator it = strState.begin(), end = strState.end(); it != end; ++it )
		state.append( (char)(*it).toInt() );
	header()->restoreState( state );
}

void LinesWidget::saveConfig()
{
	KConfigGroup group( KGlobal::config()->group( "LinesWidget Settings" ) );

	QStringList strState;
	QByteArray state = header()->saveState();
	for ( int index = 0, size = state.size(); index < size; ++index )
		strState.append( QString::number( state[index] ) );
	group.writeXdgListEntry( "Columns State", strState );
}


void LinesWidget::setSubtitle( Subtitle* subtitle )
{
	model()->setSubtitle( subtitle );
}

void LinesWidget::setTranslationMode( bool enabled )
{
	if ( m_translationMode != enabled )
	{
		m_translationMode = enabled;

		QHeaderView* header = this->header();
		if ( m_translationMode )
		{
			int textColumnWidth = header->sectionSize( LinesModel::Text );
			header->resizeSection( LinesModel::Text, textColumnWidth / 2 );
			header->setSectionHidden( LinesModel::Translation, false );
			header->resizeSection( LinesModel::Translation, textColumnWidth / 2 );
		}
		else
		{
			int textColumnWidth = header->sectionSize( LinesModel::Text );
			int translationColumnSize = header->sectionSize( LinesModel::Translation );
			header->setSectionHidden( LinesModel::Translation, true );
			header->resizeSection( LinesModel::Text, textColumnWidth + translationColumnSize );
		}
	}
}

void LinesWidget::setCurrentLine( SubtitleLine* line, bool clearSelection )
{
	if ( line )
	{
		selectionModel()->setCurrentIndex(
			model()->index( line->index(), 0 ),
			clearSelection ?
				QItemSelectionModel::Select | QItemSelectionModel::Rows | QItemSelectionModel::Clear :
				QItemSelectionModel::Select | QItemSelectionModel::Rows
		);
	}
}

void LinesWidget::setPlayingLine( SubtitleLine* line )
{
	model()->setPlayingLine( line );
}

void LinesWidget::mouseDoubleClickEvent( QMouseEvent* e )
{
	QModelIndex index = indexAt( viewport()->mapFromGlobal( e->globalPos() ) );
	if ( index.isValid() )
		emit lineDoubleClicked( model()->subtitle()->line( index.row() ) );
}

bool LinesWidget::eventFilter( QObject* object, QEvent* event )
{
	if ( object == viewport() )
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

					app()->openSubtitle( *it );
					break;
				}
			}

			return true; // eat event
		}
		else
			return TreeView::eventFilter( object, event ); // standard event processing
	}

	return TreeView::eventFilter( object, event );
}

void LinesWidget::contextMenuEvent( QContextMenuEvent* e )
{
	SubtitleLine* referenceLine = 0;
	QItemSelectionModel* selection = selectionModel();
	for ( int row = 0, rowCount = model()->rowCount(); row < rowCount; ++row )
	{
		if ( selection->isSelected( model()->index( row, 0 ) ) )
		{
			referenceLine = model()->subtitle()->line( row );
			break;
		}
	}
	if ( ! referenceLine )
		return;


	Application* app = Application::instance();
	QAction* action;


	KMenu textsMenu( i18n( "Texts" ) );
	textsMenu.addAction( i18n( "Break Lines..." ), app, SLOT( breakLines() ) );
	textsMenu.addAction( 
		m_translationMode ?
			i18n( "Unbreak Lines..." ) :
			i18n( "Unbreak Lines" ),
		app, SLOT( unbreakTexts() )
	);
	textsMenu.addAction(
		m_translationMode ?
			i18n( "Simplify Spaces..." ) :
			i18n( "Simplify Spaces" ),
		app, SLOT( simplifySpaces() )
	);
	textsMenu.addAction( i18n( "Change Case..." ), app, SLOT( changeCase() ) );
	textsMenu.addAction( i18n( "Fix Punctuation..." ), app, SLOT( fixPunctuation() ) );
	textsMenu.addAction( i18n( "Translate..." ), app, SLOT( translate() ) );
	textsMenu.addSeparator();
	textsMenu.addAction( KIcon( "tools-check-spelling" ), i18n( "Spelling..." ), app, SLOT( spellCheck() ) );


	KMenu stylesMenu( i18n( "Styles" ) );
	int styleFlags = referenceLine->primaryText().cummulativeStyleFlags() | referenceLine->secondaryText().cummulativeStyleFlags();
	action = stylesMenu.addAction(
		KIcon( "format-text-bold" ),
		i18nc( "@action:inmenu Toggle bold style", "Bold" ),
		app, SLOT( toggleSelectedLinesBold() )
	);
	action->setCheckable( true );
	action->setChecked( styleFlags & SString::Bold );
	action = stylesMenu.addAction(
		KIcon( "format-text-italic" ),
		i18nc( "@action:inmenu Toggle italic style", "Italic" ),
		app, SLOT( toggleSelectedLinesItalic() )
	);
	action->setCheckable( true );
	action->setChecked( styleFlags & SString::Italic );
	action = stylesMenu.addAction(
		KIcon( "format-text-underline" ),
		i18nc( "@action:inmenu Toggle underline style", "Underline" ),
		app, SLOT( toggleSelectedLinesUnderline() )
	);
	action->setCheckable( true );
	action->setChecked( styleFlags & SString::Underline );
	action = stylesMenu.addAction(
		KIcon( "format-text-strikethrough" ),
		i18nc( "@action:inmenu Toggle strike through style", "Strike Through" ),
		app, SLOT( toggleSelectedLinesStrikeThrough() )
	);
	action->setCheckable( true );
	action->setChecked( styleFlags & SString::StrikeThrough );


	KMenu timesMenu( i18n( "Times" ) );
	QString shiftMillis( app->generalConfig()->linesQuickShiftAmount() );
	timesMenu.addAction( i18n( "Shift +%1 Milliseconds", shiftMillis ), app, SLOT( shiftSelectedLinesForwards() ) );
	timesMenu.addAction( i18n( "Shift -%1 Milliseconds", shiftMillis ), app, SLOT( shiftSelectedLinesBackwards() ) );
	timesMenu.addSeparator();
	timesMenu.addAction( i18n( "Shift..." ), app, SLOT( shiftLines() ) );
	timesMenu.addAction( i18n( "Sort" ), app, SLOT( sortLines() ) );
	timesMenu.addAction( i18n( "Enforce Duration Limits..." ), app, SLOT( enforceDurationLimits() ) );
	timesMenu.addAction( i18n( "Set Automatic Durations..." ), app, SLOT( setAutoDurations() ) );
	timesMenu.addAction( i18n( "Maximize Durations" ), app, SLOT( maximizeDurations() ) );
	timesMenu.addAction( i18n( "Fix Overlapping Times" ), app, SLOT( fixOverlappingLines() ) );


	KMenu errorsMenu( i18n( "Errors" ) );
	action = errorsMenu.addAction( i18n( "Mark" ), app, SLOT( toggleSelectedLinesMark() ) );
	action->setCheckable( true );
	action->setChecked( referenceLine->errorFlags() & SubtitleLine::UserMark );
	errorsMenu.addSeparator();
	errorsMenu.addAction( i18n( "Check Errors..." ), app, SLOT( checkErrors() ) );
	errorsMenu.addAction( i18n( "Clear Errors..." ), app, SLOT( clearErrors() ) );
	errorsMenu.addSeparator();
	errorsMenu.addAction( i18n( "Show Errors..." ), app, SLOT( showErrors() ) );


	KMenu menu;
	menu.addAction( i18n( "Select All" ), app, SLOT( selectAllLines() ) );
	menu.addSeparator();
	menu.addAction( i18n( "Remove" ), app, SLOT( removeSelectedLines() ) );
	menu.addAction( i18n( "Insert Before" ), app, SLOT( insertBeforeCurrentLine() ) );
	menu.addAction( i18n( "Insert After" ), app, SLOT( insertAfterCurrentLine() ) );
	menu.addSeparator();
	menu.addAction( i18n( "Join Lines" ), app, SLOT( joinSelectedLines() ) );
	menu.addAction( i18n( "Split Lines" ), app, SLOT( splitSelectedLines() ) );
	menu.addSeparator();
	menu.addMenu( &textsMenu );
	menu.addMenu( &stylesMenu );
	menu.addMenu( &timesMenu );
	menu.addMenu( &errorsMenu );


	m_showingContextMenu = true;
	menu.exec( e->globalPos() );
	m_showingContextMenu = false;

	e->ignore();

	TreeView::contextMenuEvent( e );
}


void LinesWidget::onCurrentRowChanged()
{
	QModelIndex current = this->currentIndex();
	emit currentLineChanged( current.isValid() ? model()->subtitle()->line( current.row() ) : 0 );
}

void LinesWidget::drawHorizontalDotLine( QPainter* painter, int x1, int x2, int y )
{
	static int aux;

	if ( x1 > x2 )
	{
		aux = x1;
		x1 = x2;
		x2 = aux;
	}

	for ( int x = x1 + 1; x <= x2; x += 2 )
		painter->drawPoint( x, y );
}

void LinesWidget::drawVerticalDotLine( QPainter* painter, int x, int y1, int y2 )
{
	static int aux;

	if ( y1 > y2 )
	{
		aux = y1;
		y1 = y2;
		y2 = aux;
	}

	for ( int y = y1 + 1; y <= y2; y += 2 )
		painter->drawPoint( x, y );
}

void LinesWidget::drawRow( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
	TreeView::drawRow( painter, option, index );

	const int visibleColumns = m_translationMode ? LinesModel::ColumnCount : LinesModel::ColumnCount - 1;
	const int row = index.row();
	const bool rowSelected = selectionModel()->isSelected( index );
	const QPalette palette = this->palette();
	const QRect rowRect = QRect(
		visualRect( model()->index( row, 0 ) ).topLeft(),
		visualRect( model()->index( row, visibleColumns - 1 ) ).bottomRight()
	);

	// draw row grid
	painter->setPen( m_gridPen );
	if ( ! rowSelected )
		drawHorizontalDotLine( painter, rowRect.left(), rowRect.right(), rowRect.bottom() );
	for ( int column = 0; column < visibleColumns - 1; ++column )
	{
		const QRect cellRect = visualRect( model()->index( row, column ) );
		drawVerticalDotLine( painter, cellRect.right(), rowRect.top(), rowRect.bottom() );
	}

	if ( index.row() == currentIndex().row() )
	{
		painter->setPen( palette.foreground().color() );

		drawHorizontalDotLine( painter, rowRect.left(), rowRect.right(), rowRect.top() );
		drawHorizontalDotLine( painter, rowRect.left(), rowRect.right(), rowRect.bottom() );

		drawVerticalDotLine( painter, rowRect.left(), rowRect.top(), rowRect.bottom() );
		drawVerticalDotLine( painter, rowRect.right(), rowRect.top(), rowRect.bottom() );
	}
}

#include "lineswidget.moc"
