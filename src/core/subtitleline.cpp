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

#include "subtitleline.h"
#include "subtitlelineactions.h"
#include "subtitle.h"

#include <QtCore/QRegExp>

#include <KLocale>

#include <math.h>

using namespace SubtitleComposer;

/// "magic" code taken from http://tekpool.wordpress.com/category/bit-count/
int SubtitleLine::bitsCount( unsigned int u )
{
	unsigned int uCount;
	uCount = u - ((u >> 1) & 033333333333) - ((u >> 2) & 011111111111);
	return ((uCount + (uCount >> 3)) & 030707070707) % 63;
}

SubtitleLine::ErrorFlag SubtitleLine::errorFlag( SubtitleLine::ErrorID id )
{
	if ( id < 0 || id >= ErrorSIZE )
		return (ErrorFlag)0;

	return (ErrorFlag)(0x1 << id);
}

SubtitleLine::ErrorID SubtitleLine::errorID( SubtitleLine::ErrorFlag flag )
{
	if ( flag < 1 )
		return ErrorUNKNOWN;

	int id = (int)log2( flag );
	return id < ErrorSIZE ? (ErrorID)id : ErrorUNKNOWN;
}

/// ERRORS DESCRIPTIONS
/// ===================

const QString& SubtitleLine::simpleErrorText( SubtitleLine::ErrorFlag errorFlag )
{
	return simpleErrorText( errorID( errorFlag ) );
}

const QString& SubtitleLine::simpleErrorText( SubtitleLine::ErrorID errorID )
{
	static const QString empty;
	static QString texts[ErrorSIZE];

	if ( texts[EmptyPrimaryTextID].isEmpty() )
	{
		texts[EmptyPrimaryTextID]				= i18n( "Empty text" );
		texts[EmptySecondaryTextID]				= texts[EmptySecondaryTextID - 1];
		texts[MaxPrimaryCharsID]				= i18n( "Too many characters" );
		texts[MaxSecondaryCharsID]				= texts[MaxSecondaryCharsID - 1];
		texts[MaxPrimaryLinesID]				= i18n( "Too many lines" );
		texts[MaxSecondaryLinesID]				= texts[MaxSecondaryLinesID - 1];
		texts[PrimaryUnneededSpacesID]			= i18n( "Unnecessary white space" );
		texts[SecondaryUnneededSpacesID]		= texts[SecondaryUnneededSpacesID - 1];
		texts[PrimaryUnneededDashID]			= i18n( "Unnecessary dash" );
		texts[SecondaryUnneededDashID]			= texts[SecondaryUnneededDashID - 1];
		texts[PrimaryCapitalAfterEllipsisID]	= i18n( "Capital letter after continuation ellipsis" );
		texts[SecondaryCapitalAfterEllipsisID]	= texts[SecondaryCapitalAfterEllipsisID - 1];
		texts[MaxDurationPerPrimaryCharID]		= i18n( "Too long duration per character" );
		texts[MaxDurationPerSecondaryCharID]	= texts[MaxDurationPerSecondaryCharID - 1];
		texts[MinDurationPerPrimaryCharID]		= i18n( "Too short duration per character" );
		texts[MinDurationPerSecondaryCharID]	= texts[MinDurationPerSecondaryCharID - 1];
		texts[MaxDurationID]					= i18n( "Too long duration" );
		texts[MinDurationID]					= i18n( "Too short duration" );
		texts[OverlapsWithNextID]				= i18n( "Overlapping times" );
		texts[UntranslatedTextID]				= i18n( "Untranslated text" );
		texts[UserMarkID]						= i18n( "User mark" );
	}

	return ( errorID < 0 || errorID >= ErrorSIZE ) ? empty : texts[errorID];
}

QString SubtitleLine::fullErrorText( SubtitleLine::ErrorFlag errorFlag ) const
{
	return fullErrorText( errorID( errorFlag ) );
}

QString SubtitleLine::fullErrorText( SubtitleLine::ErrorID errorID ) const
{
	if ( ! (m_errorFlags & (0x1 << errorID)) )
		return QString();

	switch ( errorID )
	{
		case EmptyPrimaryTextID:
			return i18n(
				"Has no primary text."
			);
		case EmptySecondaryTextID:
			return i18n(
				"Has no translation text."
			);
		case MaxPrimaryCharsID:
			return i18np(
				"Has too many characters in primary text (1 char).",
				"Has too many characters in primary text (%1 chars).",
				primaryCharacters()
			);
		case MaxSecondaryCharsID:
			return i18np(
				"Has too many characters in translation text (1 char).",
				"Has too many characters in translation text (%1 chars).",
				secondaryCharacters()
			);
		case MaxPrimaryLinesID:
			return i18np(
				"Has too many lines in primary text (1 line).",
				"Has too many lines in primary text (%1 lines).",
				primaryLines()
			);
		case MaxSecondaryLinesID:
			return i18np(
				"Has too many lines in translation text (1 line).",
				"Has too many lines in translation text (%1 lines).",
				secondaryLines()
			);
		case PrimaryUnneededSpacesID:
			return i18n(
				"Has unnecessary spaces in primary text."
			);
		case SecondaryUnneededSpacesID:
			return i18n(
				"Has unnecessary spaces in translation text."
			);
		case PrimaryUnneededDashID:
			return i18n(
				"Has unnecessary dash in primary text."
			);
		case SecondaryUnneededDashID:
			return i18n(
				"Has unnecessary dash in translation text."
			);
		case PrimaryCapitalAfterEllipsisID:
			return i18n(
				"Has capital letter after line continuation ellipsis in primary text."
			);
		case SecondaryCapitalAfterEllipsisID:
			return i18n(
				"Has capital letter after line continuation ellipsis in translation text."
			);
		case MaxDurationPerPrimaryCharID:
			return i18np(
				"Has too long duration per character in primary text (1 msec/char).",
				"Has too long duration per character in primary text (%1 msecs/char).",
				durationTime().toMillis()/primaryCharacters()
			);
		case MaxDurationPerSecondaryCharID:
			return i18np(
				"Has too long duration per character in translation text (1 msec/char).",
				"Has too long duration per character in translation text (%1 msecs/char).",
				durationTime().toMillis()/secondaryCharacters()
			);
		case MinDurationPerPrimaryCharID:
			return i18np(
				"Has too short duration per character in primary text (1 msec/char).",
				"Has too short duration per character in primary text (%1 msecs/char).",
				durationTime().toMillis()/primaryCharacters()
			);
		case MinDurationPerSecondaryCharID:
			return i18np(
				"Has too short duration per character in translation text (1 msec/char).",
				"Has too short duration per character in translation text (%1 msecs/char).",
				durationTime().toMillis()/secondaryCharacters()
			);
		case MaxDurationID:
			return i18np(
				"Has too long duration (1 msec).",
				"Has too long duration (%1 msecs).",
				durationTime().toMillis()
			);
		case MinDurationID:
			return i18np(
				"Has too short duration (1 msec).",
				"Has too short duration (%1 msecs).",
				durationTime().toMillis()
			);
		case OverlapsWithNextID:
			return i18n(
				"Overlaps with next line."
			);
		case UntranslatedTextID:
			return i18n(
				"Has untranslated text."
			);
		case UserMarkID:
			return i18n(
				"Has user mark."
			);
		default:
			return QString();
	}
}


SubtitleLine::SubtitleLine( const SString& pText, const SString& sText ):
	QObject(),
	m_subtitle( 0 ),
	m_primaryText( pText ),
	m_secondaryText( sText ),
	m_showTime(),
	m_hideTime(),
	m_errorFlags( 0 ),
	m_cachedIndex( -1 ),
	m_formatData( 0 )
{
}

SubtitleLine::SubtitleLine( const SString& pText, const Time& showTime, const Time& hideTime ):
	QObject(),
	m_subtitle( 0 ),
	m_primaryText( pText ),
	m_secondaryText( QString() ),
	m_showTime( showTime ),
	m_hideTime( hideTime ),
	m_errorFlags( 0 ),
	m_cachedIndex( -1 ),
	m_formatData( 0 )
{
}

SubtitleLine::SubtitleLine( const SString& pText, const SString& sText, const Time& showTime, const Time& hideTime ):
	QObject(),
	m_subtitle( 0 ),
	m_primaryText( pText ),
	m_secondaryText( sText ),
	m_showTime( showTime ),
	m_hideTime( hideTime ),
	m_errorFlags( 0 ),
	m_cachedIndex( -1 ),
	m_formatData( 0 )
{
}

SubtitleLine::SubtitleLine( const SubtitleLine& line ):
	QObject(),
	m_subtitle( 0 ),
	m_primaryText( line.m_primaryText ),
	m_secondaryText( line.m_secondaryText ),
	m_showTime( line.m_showTime ),
	m_hideTime( line.m_hideTime ),
	m_errorFlags( line.m_errorFlags ),
	m_cachedIndex( -1 ),
	m_formatData( 0 )
{
}

SubtitleLine& SubtitleLine::operator=( const SubtitleLine& line )
{
	if ( this == &line )
		return *this;

	m_primaryText = line.m_primaryText;
	m_secondaryText = line.m_secondaryText;
	m_showTime = line.m_showTime;
	m_hideTime = line.m_hideTime;
	m_errorFlags = line.m_errorFlags;

	return *this;
}

SubtitleLine::~SubtitleLine()
{
	delete m_formatData;
}


FormatData* const SubtitleLine::formatData() const
{
	return m_formatData;
}

void SubtitleLine::setFormatData( const FormatData* formatData )
{
	delete m_formatData;

	m_formatData = formatData ? new FormatData( *formatData ) : 0;
}

int SubtitleLine::number() const
{
	return index() + 1;
}

int SubtitleLine::index() const
{
	if ( ! m_subtitle )
		return -1;

	if ( m_cachedIndex < 0 || (m_cachedIndex > m_subtitle->m_lastValidCachedIndex) )
	{
		SubtitleLine* line;
		for ( int index = m_subtitle->m_lastValidCachedIndex + 1, size = m_subtitle->m_lines.count(); index < size; ++index )
		{
			line = m_subtitle->m_lines.at( index );
			line->m_cachedIndex = index;
			if ( line == this )
			{
				m_subtitle->setLastValidCachedIndex( index );
				break;
			}
		}

		//kDebug() << "searched index";
	}

	//kDebug() << "index" << m_index << "| text" << m_primaryText.string();

	return m_cachedIndex;
}

Subtitle* SubtitleLine::subtitle()
{
	return m_subtitle;
}

const Subtitle* SubtitleLine::subtitle() const
{
	return m_subtitle;
}

SubtitleLine* SubtitleLine::prevLine()
{
	return m_subtitle ? m_subtitle->line( index() - 1 ) : 0;
}

SubtitleLine* SubtitleLine::nextLine()
{
	return m_subtitle ? m_subtitle->line( index() + 1 ) : 0;
}

const SString& SubtitleLine::primaryText() const
{
	return m_primaryText;
}

void SubtitleLine::setPrimaryText( const SString& pText )
{
	if ( m_primaryText != pText )
		processAction( new SetLinePrimaryTextAction( *this, pText ) );
}

const SString& SubtitleLine::secondaryText() const
{
	return m_secondaryText;
}

void SubtitleLine::setSecondaryText( const SString& sText )
{
	if ( m_secondaryText != sText )
		processAction( new SetLineSecondaryTextAction( *this, sText ) );
}

void SubtitleLine::setTexts( const SString& pText, const SString& sText )
{
	if ( m_primaryText != pText || m_secondaryText != sText )
		processAction( new SetLineTextsAction( *this, pText, sText ) );
}

SString SubtitleLine::fixPunctuation( const SString& t, bool spaces, bool quotes, bool englishI, bool ellipsis, bool* cont )
{
	if ( ! t.length() )
		return t;

	SString text( t );

	if ( spaces )
	{
		text = simplifyTextWhiteSpace( text );

		// remove spaces after " or ' at the beginning of line
		text.replace( QRegExp( "^([\"'])\\s" ), "\\1" );

		// remove space before " or ' at the end of line
		text.replace( QRegExp( "\\s([\"'])$" ), "\\1" );

		// if not present, add space after '?', '!', ',', ';', ':', ')' and ']'
		text.replace( QRegExp( "([\\?!,;:\\)\\]])([^\\s\"'])" ), "\\1 \\2" );

		// if not present, add space after '.'
		text.replace( QRegExp( "(\\.)([^\\s\\.\"'])" ), "\\1 \\2" );

		// remove space after '¿', '¡', '(' and '['
		text.replace( QRegExp( "([¿¡\\(\\[])\\s" ), "\\1" );

		// remove space before '?', '!', ',', ';', ':', '.', ')' and ']'
		text.replace( QRegExp( "\\s([\\?!,;:\\.\\)\\]])" ), "\\1" );

		// remove space after ... at the beginning of sentence
		text.replace( QRegExp( "^\\.\\.\\.?\\s" ), "..." );
	}

	if ( quotes ) // quotes and double quotes
	{
		text.replace( QRegExp( "`|´|" ), "'" );
		text.replace( QRegExp( "''|«|»" ), "\"" );
	}

	if ( englishI ) // fix english I pronoun capitalization
		text.replace( QRegExp( "([\\s\"'\\(\\[])i([\\s'\",;:\\.\\?!\\]\\)]|$)" ), "\\1I\\2" );

	if ( ellipsis ) // fix ellipsis
	{
		text.replace( QRegExp( "[,;]?\\.{2,}" ), "..." );
		text.replace( QRegExp( "[,;]\\s*$" ), "..." );

		if ( text.indexOf( QRegExp( "[\\.:?!\\)\\]'\\\"]$" ) ) == -1 )
			text.append( "..." );

		if ( cont )
		{
			if ( *cont && text.indexOf( QRegExp( "^\\s*\\.{3}[^\\.]?" ) ) == -1 )
				text.replace( QRegExp( "^\\s*\\.*\\s*" ), "..." );

			*cont = text.indexOf( QRegExp( "\\.{3,3}\\s*$" ) ) != -1;
		}
	}
	else
	{
		if ( cont )
		{
			*cont = text.indexOf( QRegExp( "[?!\\)\\]'\\\"]\\s*$" ) ) == -1;

			if ( ! *cont )
				*cont = text.indexOf( QRegExp( "[^\\.]?\\.\\s*$" ) ) == -1;
		}
	}

	return text;
}

SString SubtitleLine::breakText( const SString& t, int minLengthForBreak )
{
	Q_ASSERT( minLengthForBreak >= 0 );

	SString text( t );
	text.replace( '\n', ' ' );

	if ( text.length() <= minLengthForBreak )
		return text;

	static const QRegExp spaceRegExp( "[^ \t\n][ \t\n]" );

	// this is a fricking test case
	// this is a fri|cking test case	IDEAL: 13
	// this is a| fricking test case	LOWER: 10 <--
	// this is a fricking| test case	UPPER: 18

	int position = text.length()/2;
	int lowerPosition = text.lastIndexOf( spaceRegExp, position );
	int upperPosition = text.indexOf( spaceRegExp, position );

	if ( (lowerPosition == -1 || lowerPosition == 0) && (upperPosition == -1 || upperPosition == (text.length()-1)) )
		return text;
	else if ( position - lowerPosition <= upperPosition - position )
		position = lowerPosition;
	else
		position = upperPosition;

	position++;
	text.insert( position, "\n" );
	position++;
	while ( text.at( position ).isSpace() && position < text.length() )
		text.remove( position, 1 );

	return text;
}

void SubtitleLine::breakText( int minLengthForBreak, TextTarget target )
{
	switch ( target )
	{
		case Primary:
			setPrimaryText( breakText( m_primaryText, minLengthForBreak ) );
			break;
		case Secondary:
			setSecondaryText( breakText( m_secondaryText, minLengthForBreak ) );
			break;
		case Both:
			setTexts( breakText( m_primaryText, minLengthForBreak ), breakText( m_secondaryText, minLengthForBreak ) );
			break;
		default:
			break;
	}
}

void SubtitleLine::unbreakText( TextTarget target )
{
	switch ( target )
	{
		case Primary:
			setPrimaryText( SString( primaryText() ).replace( '\n', ' ' ) );
			break;
		case Secondary:
			setSecondaryText( SString( secondaryText() ).replace( '\n', ' ' ) );
			break;
		case Both:
			setTexts( SString( primaryText() ).replace( '\n', ' ' ), SString( secondaryText() ).replace( '\n', ' ' ) );
			break;
		default:
			break;
	}
}

QString SubtitleLine::simplifyTextWhiteSpace( QString text )
{
	static const QRegExp regExp1( " *\n *" );
	static const QRegExp regExp2( " +" );
	static const QRegExp regExp3( "\n+" );

	text.replace( '\t', ' ' );
	text.replace( regExp1, "\n" );
	text.replace( regExp2, " " );
	text.replace( regExp3, "\n" );

	return text.trimmed();
}

SString SubtitleLine::simplifyTextWhiteSpace( SString text )
{
	static const QRegExp regExp1( " *\n *" );
	static const QRegExp regExp2( " +" );
	static const QRegExp regExp3( "\n+" );

	text.replace( '\t', ' ' );
	text.replace( regExp1, "\n" );
	text.replace( regExp2, " " );
	text.replace( regExp3, "\n" );

	return text.trimmed();
}

void SubtitleLine::simplifyTextWhiteSpace( TextTarget target )
{
	switch ( target )
	{
		case Primary:
			setPrimaryText( simplifyTextWhiteSpace( m_primaryText ) );
			break;
		case Secondary:
			setSecondaryText( simplifyTextWhiteSpace( m_secondaryText ) );
			break;
		case Both:
			setTexts( simplifyTextWhiteSpace( m_primaryText ), simplifyTextWhiteSpace( m_secondaryText ) );
			break;
		default:
			break;
	}
}

Time SubtitleLine::showTime() const
{
	return m_showTime;
}

void SubtitleLine::setShowTime( const Time& showTime )
{
	if ( m_showTime != showTime )
		processAction( new SetLineShowTimeAction( *this, showTime ) );
}


Time SubtitleLine::hideTime() const
{
	return m_hideTime;
}

void SubtitleLine::setHideTime( const Time& hideTime )
{
	if ( m_hideTime != hideTime )
		processAction( new SetLineHideTimeAction( *this, hideTime ) );
}

Time SubtitleLine::durationTime() const
{
	return Time( m_hideTime.toMillis() - m_showTime.toMillis() );
}

void SubtitleLine::setDurationTime( const Time& durationTime )
{
	setHideTime( m_showTime + durationTime );
}

int SubtitleLine::primaryCharacters() const
{
	return m_primaryText.string().simplified().length();
}

int SubtitleLine::primaryWords() const
{
	QString text( m_primaryText.string().simplified() );
	return text.length() ? text.count( ' ' ) + 1 : 0;
}

int SubtitleLine::primaryLines() const
{
	QString text( simplifyTextWhiteSpace( m_primaryText.string() ) );
	return text.length() ? text.count( '\n' ) + 1 : 0;
}

int SubtitleLine::secondaryCharacters() const
{
	return m_secondaryText.string().simplified().length();
}

int SubtitleLine::secondaryWords() const
{
	QString text( m_secondaryText.string().simplified() );
	return text.length() ? text.count( ' ' ) + 1 : 0;
}

int SubtitleLine::secondaryLines() const
{
	QString text( simplifyTextWhiteSpace( m_secondaryText.string() ) );
	return text.length() ? text.count( '\n' ) + 1 : 0;
}

Time SubtitleLine::autoDuration( const QString& t, int msecsPerChar, int msecsPerWord, int msecsPerLine )
{
	Q_ASSERT( msecsPerChar >= 0 );
	Q_ASSERT( msecsPerWord >= 0 );
	Q_ASSERT( msecsPerLine >= 0 );

	QString text( simplifyTextWhiteSpace( t ) );
	if ( ! text.length() )
		return 0;

	int chars = text.length();
	int lines = text.count( '\n' ) + 1;
	int words = text.count( ' ' ) + lines;

	return chars*msecsPerChar + words*msecsPerWord + lines*msecsPerLine;
}

Time SubtitleLine::autoDuration( int msecsPerChar, int msecsPerWord, int msecsPerLine, TextTarget calculationTarget )
{
	switch ( calculationTarget )
	{
		case Secondary:
			return autoDuration( m_secondaryText.string(), msecsPerChar, msecsPerWord, msecsPerLine );
		case Both:
		{
			Time primary = autoDuration( m_primaryText.string(), msecsPerChar, msecsPerWord, msecsPerLine );
			Time secondary = autoDuration( m_secondaryText.string(), msecsPerChar, msecsPerWord, msecsPerLine );
			return primary > secondary ? primary : secondary;
		}
		case Primary:
		default:
			return autoDuration( m_primaryText.string(), msecsPerChar, msecsPerWord, msecsPerLine );
	}
}

void SubtitleLine::setTimes( const Time& showTime, const Time& hideTime )
{
	if ( m_showTime != showTime || m_hideTime != hideTime )
		processAction( new SetLineTimesAction( *this, showTime, hideTime, i18n( "Set Line Times" ) ) );
}

void SubtitleLine::shiftTimes( long mseconds )
{
	if ( mseconds )
		processAction(
			new SetLineTimesAction(
				*this,
				Time( m_showTime.toMillis() + mseconds ),
				Time( m_hideTime.toMillis() + mseconds ),
				i18n( "Shift Line Times" )
			)
		);
}

void SubtitleLine::adjustTimes( long shiftMseconds, double scaleFactor )
{
	if ( shiftMseconds || scaleFactor != 1.0 )
	{
		processAction(
			new SetLineTimesAction(
				*this,
				m_showTime.adjusted( shiftMseconds, scaleFactor ),
				m_hideTime.adjusted( shiftMseconds, scaleFactor ),
				i18n( "Adjust Line Times" )
			)
		);
	}
}



/// ERRORS

int SubtitleLine::errorFlags() const
{
	return m_errorFlags;
}

int SubtitleLine::errorCount() const
{
	return bitsCount( m_errorFlags );
}

void SubtitleLine::setErrorFlags( int errorFlags )
{
	if ( m_errorFlags != errorFlags )
		processAction( new SetLineErrorsAction( *this, errorFlags ) );
}

void SubtitleLine::setErrorFlags( int errorFlags, bool value )
{
	setErrorFlags( value ? (m_errorFlags | errorFlags) : (m_errorFlags & ~errorFlags) );
}

bool SubtitleLine::checkEmptyPrimaryText( bool update )
{
	static const QRegExp emptyTextRegExp( "^\\s*$" );

	bool error = m_primaryText.isEmpty() || m_primaryText.indexOf( emptyTextRegExp ) != -1;

	if ( update )
		setErrorFlags( EmptyPrimaryText, error );

	return error;
}

bool SubtitleLine::checkEmptySecondaryText( bool update )
{
	static const QRegExp emptyTextRegExp( "^\\s*$" );

	bool error = m_secondaryText.isEmpty() || m_secondaryText.indexOf( emptyTextRegExp ) != -1;

	if ( update )
		setErrorFlags( EmptySecondaryText, error );

	return error;
}

bool SubtitleLine::checkUntranslatedText( bool update )
{
	bool error = m_primaryText.string() == m_secondaryText.string();

	if ( update )
		setErrorFlags( UntranslatedText, error );

	return error;
}

bool SubtitleLine::checkOverlapsWithNext( bool update )
{
	SubtitleLine* nextLine = this->nextLine();
	bool error = nextLine && nextLine->m_showTime <= m_hideTime;

	if ( update )
		setErrorFlags( OverlapsWithNext, error );

	return error;
}

bool SubtitleLine::checkMinDuration( int minMsecs, bool update )
{
	Q_ASSERT( minMsecs >= 0 );

	bool error = durationTime().toMillis() < minMsecs;

	if ( update )
		setErrorFlags( MinDuration, error );

	return error;
}

bool SubtitleLine::checkMaxDuration( int maxMsecs, bool update )
{
	Q_ASSERT( maxMsecs >= 0 );

	bool error = durationTime().toMillis() > maxMsecs;

	if ( update )
		setErrorFlags( MaxDuration, error );

	return error;
}

bool SubtitleLine::checkMinDurationPerPrimaryChar( int minMsecsPerChar, bool update )
{
	Q_ASSERT( minMsecsPerChar >= 0 );

	int characters = primaryCharacters();
	bool error = characters ? (durationTime().toMillis()/characters < minMsecsPerChar) : false;

	if ( update )
		setErrorFlags( MinDurationPerPrimaryChar, error );

	return error;
}

bool SubtitleLine::checkMinDurationPerSecondaryChar( int minMsecsPerChar, bool update )
{
	Q_ASSERT( minMsecsPerChar >= 0 );

	int characters = secondaryCharacters();
	bool error = characters ? (durationTime().toMillis()/characters < minMsecsPerChar) : false;

	if ( update )
		setErrorFlags( MinDurationPerSecondaryChar, error );

	return error;
}

bool SubtitleLine::checkMaxDurationPerPrimaryChar( int maxMsecsPerChar, bool update )
{
	Q_ASSERT( maxMsecsPerChar >= 0 );

	int characters = primaryCharacters();
	bool error = characters ? (durationTime().toMillis()/characters > maxMsecsPerChar) : false;

	if ( update )
		setErrorFlags( MaxDurationPerPrimaryChar, error );

	return error;
}

bool SubtitleLine::checkMaxDurationPerSecondaryChar( int maxMsecsPerChar, bool update )
{
	Q_ASSERT( maxMsecsPerChar >= 0 );

	int characters = secondaryCharacters();
	bool error = characters ? (durationTime().toMillis()/characters > maxMsecsPerChar) : false;

	if ( update )
		setErrorFlags( MaxDurationPerSecondaryChar, error );

	return error;
}

bool SubtitleLine::checkMaxPrimaryChars( int maxCharacters, bool update )
{
	Q_ASSERT( maxCharacters >= 0 );

	bool error = primaryCharacters() > maxCharacters;

	if ( update )
		setErrorFlags( MaxPrimaryChars, error );

	return error;
}

bool SubtitleLine::checkMaxSecondaryChars( int maxCharacters, bool update )
{
	Q_ASSERT( maxCharacters >= 0 );

	bool error = secondaryCharacters() > maxCharacters;

	if ( update )
		setErrorFlags( MaxSecondaryChars, error );

	return error;
}

bool SubtitleLine::checkMaxPrimaryLines( int maxLines, bool update )
{
	Q_ASSERT( maxLines >= 0 );

	bool error = primaryLines() > maxLines;

	if ( update )
		setErrorFlags( MaxPrimaryLines, error );

	return error;
}

bool SubtitleLine::checkMaxSecondaryLines( int maxLines, bool update )
{
	Q_ASSERT( maxLines >= 0 );

	bool error = secondaryLines() > maxLines;

	if ( update )
		setErrorFlags( MaxSecondaryLines, error );

	return error;
}

bool SubtitleLine::checkPrimaryUnneededSpaces( bool update )
{
	static const QRegExp unneededSpaceRegExp( "(^\\s|\\s$|¿\\s|¡\\s|\\s\\s|\\s!|\\s\\?|\\s:|\\s;|\\s,|\\s\\.)" );

	bool error = m_primaryText.indexOf( unneededSpaceRegExp ) != -1;

	if ( update )
		setErrorFlags( PrimaryUnneededSpaces, error );

	return error;
}

bool SubtitleLine::checkSecondaryUnneededSpaces( bool update )
{
	static const QRegExp unneededSpaceRegExp( "(^\\s|\\s$|¿\\s|¡\\s|\\s\\s|\\s!|\\s\\?|\\s:|\\s;|\\s,|\\s\\.)" );

	bool error = m_secondaryText.indexOf( unneededSpaceRegExp ) != -1;

	if ( update )
		setErrorFlags( SecondaryUnneededSpaces, error );

	return error;
}

bool SubtitleLine::checkPrimaryCapitalAfterEllipsis( bool update )
{
	static const QRegExp capitalAfterEllipsisRegExp( "^\\s*\\.\\.\\.[¡¿\\.,;\\(\\[\\{\"'\\s]*" );

	bool error = m_primaryText.indexOf( capitalAfterEllipsisRegExp ) != -1;
	if ( error )
	{
		QChar chr = m_primaryText.at( capitalAfterEllipsisRegExp.matchedLength() );
		error = chr.isLetter() && chr == chr.toUpper();
	}

	if ( update )
		setErrorFlags( PrimaryCapitalAfterEllipsis, error );

	return error;
}

bool SubtitleLine::checkSecondaryCapitalAfterEllipsis( bool update )
{
	static const QRegExp capitalAfterEllipsisRegExp( "^\\s*\\.\\.\\.[¡¿\\.,;\\(\\[\\{\"'\\s]*" );

	bool error = m_secondaryText.indexOf( capitalAfterEllipsisRegExp ) != -1;
	if ( error )
	{
		QChar chr = m_secondaryText.at( capitalAfterEllipsisRegExp.matchedLength() );
		error = chr.isLetter() && chr == chr.toUpper();
	}

	if ( update )
		setErrorFlags( SecondaryCapitalAfterEllipsis, error );

	return error;
}

bool SubtitleLine::checkPrimaryUnneededDash( bool update )
{
	static const QRegExp unneededDashRegExp( "(^|\n)\\s*-[^-]" );

	bool error = m_primaryText.count( unneededDashRegExp ) == 1;

	if ( update )
		setErrorFlags( PrimaryUnneededDash, error );

	return error;
}

bool SubtitleLine::checkSecondaryUnneededDash( bool update )
{
	static const QRegExp unneededDashRegExp( "(^|\n)\\s*-[^-]" );

	bool error = m_secondaryText.count( unneededDashRegExp ) == 1;

	if ( update )
		setErrorFlags( SecondaryUnneededDash, error );

	return error;
}

int SubtitleLine::check( int errorFlagsToCheck, int minDurationMsecs, int maxDurationMsecs, int minMsecsPerChar, int maxMsecsPerChar, int maxChars, int maxLines, bool update )
{
	int lineErrorFlags = m_errorFlags & ~errorFlagsToCheck; // clear the flags we're going to (re)check

	if ( errorFlagsToCheck & EmptyPrimaryText )
		if ( checkEmptyPrimaryText( false ) )
			lineErrorFlags |= EmptyPrimaryText;

	if ( errorFlagsToCheck & EmptySecondaryText )
		if ( checkEmptySecondaryText( false ) )
			lineErrorFlags |= EmptySecondaryText;

	if ( errorFlagsToCheck & OverlapsWithNext )
		if ( checkOverlapsWithNext( false ) )
			lineErrorFlags |= OverlapsWithNext;

	if ( errorFlagsToCheck & UntranslatedText )
		if ( checkUntranslatedText( false ) )
			lineErrorFlags |= UntranslatedText;

	if ( errorFlagsToCheck & MaxDuration )
		if ( checkMaxDuration( maxDurationMsecs, false ) )
			lineErrorFlags |= MaxDuration;

	if ( errorFlagsToCheck & MinDuration )
		if ( checkMinDuration( minDurationMsecs, false ) )
			lineErrorFlags |= MinDuration;

	if ( errorFlagsToCheck & MaxDurationPerPrimaryChar )
		if ( checkMaxDurationPerPrimaryChar( maxMsecsPerChar, false ) )
			lineErrorFlags |= MaxDurationPerPrimaryChar;

	if ( errorFlagsToCheck & MaxDurationPerSecondaryChar )
		if ( checkMaxDurationPerSecondaryChar( maxMsecsPerChar, false ) )
			lineErrorFlags |= MaxDurationPerSecondaryChar;

	if ( errorFlagsToCheck & MinDurationPerPrimaryChar )
		if ( checkMinDurationPerPrimaryChar( minMsecsPerChar, false ) )
			lineErrorFlags |= MinDurationPerPrimaryChar;

	if ( errorFlagsToCheck & MinDurationPerSecondaryChar )
		if ( checkMinDurationPerSecondaryChar( minMsecsPerChar, false ) )
			lineErrorFlags |= MinDurationPerSecondaryChar;

	if ( errorFlagsToCheck & MaxPrimaryChars )
		if ( checkMaxPrimaryChars( maxChars, false ) )
			lineErrorFlags |= MaxPrimaryChars;

	if ( errorFlagsToCheck & MaxSecondaryChars )
		if ( checkMaxSecondaryChars( maxChars, false ) )
			lineErrorFlags |= MaxSecondaryChars;

	if ( errorFlagsToCheck & MaxPrimaryLines )
		if ( checkMaxPrimaryLines( maxLines, false ) )
			lineErrorFlags |= MaxPrimaryLines;

	if ( errorFlagsToCheck & MaxSecondaryLines )
		if ( checkMaxSecondaryLines( maxLines, false ) )
			lineErrorFlags |= MaxSecondaryLines;

	if ( errorFlagsToCheck & MaxSecondaryLines )
		if ( checkMaxSecondaryLines( maxLines, false ) )
			lineErrorFlags |= MaxSecondaryLines;

	if ( errorFlagsToCheck & PrimaryUnneededSpaces )
		if ( checkPrimaryUnneededSpaces( false ) )
			lineErrorFlags |= PrimaryUnneededSpaces;

	if ( errorFlagsToCheck & SecondaryUnneededSpaces )
		if ( checkSecondaryUnneededSpaces( false ) )
			lineErrorFlags |= SecondaryUnneededSpaces;

	if ( errorFlagsToCheck & PrimaryCapitalAfterEllipsis )
		if ( checkPrimaryCapitalAfterEllipsis( false ) )
			lineErrorFlags |= PrimaryCapitalAfterEllipsis;

	if ( errorFlagsToCheck & SecondaryCapitalAfterEllipsis )
		if ( checkSecondaryCapitalAfterEllipsis( false ) )
			lineErrorFlags |= SecondaryCapitalAfterEllipsis;

	if ( errorFlagsToCheck & PrimaryUnneededDash )
		if ( checkPrimaryUnneededDash( false ) )
			lineErrorFlags |= PrimaryUnneededDash;

	if ( errorFlagsToCheck & SecondaryUnneededDash )
		if ( checkSecondaryUnneededDash( false ) )
			lineErrorFlags |= SecondaryUnneededDash;

	if ( update )
		setErrorFlags( lineErrorFlags );

	return lineErrorFlags;
}

void SubtitleLine::processAction( Action* action )
{
	if ( m_subtitle )
		m_subtitle->processAction( action );
	else
	{
		action->redo();
		delete action;
	}
}

#include "subtitleline.moc"
