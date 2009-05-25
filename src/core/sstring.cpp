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

#include "sstring.h"

#include <QtCore/QList>
#include <QtCore/QStringList>

#include <KDebug>

using namespace SubtitleComposer;

SString::SString( const QString& string, int styleFlags ):
	m_string( string ),
	m_styleFlags( 0 ),
	m_capacity( 0 )
{
	if ( m_string.length() )
	{
		setMinFlagsCapacity( m_string.length() );
		memset( m_styleFlags, styleFlags & AllStyles, m_string.length() );
	}
}

SString::SString( const SString& sstring ):
	m_string( sstring.m_string ),
	m_styleFlags( 0 ),
	m_capacity( 0 )
{
	if ( m_string.length() )
	{
		setMinFlagsCapacity( m_string.length() );
		memcpy( m_styleFlags, sstring.m_styleFlags, m_string.length() );
	}
}

SString& SString::operator=( const SString& sstring )
{
	if ( this == &sstring )
		return *this;

	m_string = sstring.m_string;
	setMinFlagsCapacity( m_string.length() );

	if ( m_string.length() )
		memcpy( m_styleFlags, sstring.m_styleFlags, m_string.length() );

	return *this;
}

SString::~SString()
{
	delete [] m_styleFlags;
}

void SString::setString( const QString& string, int styleFlags )
{
	m_string = string;
	setMinFlagsCapacity( m_string.length() );
	if ( m_string.length() )
		memset( m_styleFlags, styleFlags & AllStyles, m_string.length() );
}

QString SString::richString( RichOutputMode mode ) const
{
	if ( m_string.isEmpty() )
		return m_string;

	QString ret;

	if ( mode == Compact )
	{
		char prevStyleFlags = m_styleFlags[0];
		int prevIndex = 0;

		if ( prevStyleFlags & Italic )
			ret += "<i>";
		if ( prevStyleFlags & Bold )
			ret += "<b>";
		if ( prevStyleFlags & Underline )
			ret += "<u>";
		if ( prevStyleFlags & StrikeThrough )
			ret += "<s>";

		const int size = m_string.length();
		for ( int index = 1; index < size; ++index )
		{
			if ( m_styleFlags[index] != prevStyleFlags )
			{
				QString token( m_string.mid( prevIndex, index - prevIndex ) );
				ret += token.replace( '<', "&lt;" ).replace( '>', "&gt;" );

				if ( (prevStyleFlags & StrikeThrough) && ! (m_styleFlags[index] & StrikeThrough) )
					ret += "</s>";
				if ( (prevStyleFlags & Underline) && ! (m_styleFlags[index] & Underline) )
					ret += "</u>";
				if ( (prevStyleFlags & Bold) && ! (m_styleFlags[index] & Bold) )
					ret += "</b>";
				if ( (prevStyleFlags & Italic) && ! (m_styleFlags[index] & Italic) )
					ret += "</i>";

				if ( ! (prevStyleFlags & Italic) && (m_styleFlags[index] & Italic) )
					ret += "<i>";
				if ( ! (prevStyleFlags & Bold) && (m_styleFlags[index] & Bold) )
					ret += "<b>";
				if ( ! (prevStyleFlags & Underline) && (m_styleFlags[index] & Underline) )
					ret += "<u>";
				if ( ! (prevStyleFlags & StrikeThrough) && (m_styleFlags[index] & StrikeThrough) )
					ret += "<s>";

				prevIndex = index;
				prevStyleFlags = m_styleFlags[index];
			}
		}

		QString token( m_string.mid( prevIndex, m_string.length() - prevIndex ) );
		if ( token.length() )
		{
			ret += token.replace( '<', "&lt;" ).replace( '>', "&gt;" );

			if ( prevStyleFlags & StrikeThrough )
				ret += "</s>";
			if ( prevStyleFlags & Underline )
				ret += "</u>";
			if ( prevStyleFlags & Bold )
				ret += "</b>";
			if ( prevStyleFlags & Italic )
				ret += "</i>";
		}
	}
	else // outputMode == Verbose
	{
		int currentStyleFlags = m_styleFlags[0];
		int prevIndex = 0;
		for ( uint index = 1, size = m_string.length(); index < size; ++index )
		{
			if ( currentStyleFlags != m_styleFlags[index] )
			{
				if ( currentStyleFlags & StrikeThrough )	ret += "<s>";
				if ( currentStyleFlags & Bold )				ret += "<b>";
				if ( currentStyleFlags & Italic )			ret += "<i>";
				if ( currentStyleFlags & Underline )		ret += "<u>";

				ret += m_string.mid( prevIndex, index - prevIndex );

				if ( currentStyleFlags & Underline )		ret += "</u>";
				if ( currentStyleFlags & Italic )			ret += "</i>";
				if ( currentStyleFlags & Bold )				ret += "</b>";
				if ( currentStyleFlags & StrikeThrough )	ret += "</s>";

				prevIndex = index;

				currentStyleFlags = m_styleFlags[index];
			}
		}

		if ( prevIndex + 1 < m_string.length() )
		{
			if ( currentStyleFlags & StrikeThrough )	ret += "<s>";
			if ( currentStyleFlags & Bold )				ret += "<b>";
			if ( currentStyleFlags & Italic )			ret += "<i>";
			if ( currentStyleFlags & Underline )		ret += "<u>";

			ret += m_string.mid( prevIndex );

			if ( currentStyleFlags & Underline )		ret += "</u>";
			if ( currentStyleFlags & Italic )			ret += "</i>";
			if ( currentStyleFlags & Bold )				ret += "</b>";
			if ( currentStyleFlags & StrikeThrough )	ret += "</s>";
		}
	}

	return ret;
}

SString& SString::setRichString( const QString& string )
{
	QRegExp tagRegExp( "</?[bBiIuUsS]>" );

	m_string.clear();

	int currentStyle = 0;
	int offsetPos = 0, matchedPos;
	while ( (matchedPos = tagRegExp.indexIn( string, offsetPos ) ) != -1 )
	{
		QString matched( tagRegExp.cap( 0 ).toLower() );

		int newStyle = currentStyle;

		if ( matched == "<b>" )
			newStyle |= SString::Bold;
		else if ( matched == "<i>" )
			newStyle |= SString::Italic;
		else if ( matched == "<u>" )
			newStyle |= SString::Underline;
		else if ( matched == "<s>" )
			newStyle |= SString::StrikeThrough;
		else if ( matched == "</b>" )
			newStyle &= ~SString::Bold;
		else if ( matched == "</i>" )
			newStyle &= ~SString::Italic;
		else if ( matched == "</u>" )
			newStyle &= ~SString::Underline;
		else if ( matched == "</s>" )
			newStyle &= ~SString::StrikeThrough;

		if ( newStyle != currentStyle )
		{
			QString token( string.mid( offsetPos, matchedPos - offsetPos ) );
			append( SString( token.replace( "&lt;", "<" ).replace( "&gt;", ">" ), currentStyle ) );
			currentStyle = newStyle;
		}

		offsetPos = matchedPos + matched.length();
	}

	QString token( string.mid( offsetPos, matchedPos - offsetPos ) );
	append( SString( token.replace( "&lt;", "<" ).replace( "&gt;", ">" ), currentStyle ) );

	return *this;
}

int SString::cummulativeStyleFlags() const
{
	int cummulativeStyleFlags = 0;
	for ( int index = 0, size = m_string.length(); index < size; ++index )
	{
		cummulativeStyleFlags |= m_styleFlags[index];
		if ( cummulativeStyleFlags == AllStyles )
			break;
	}
	return cummulativeStyleFlags;
}

bool SString::hasStyleFlags( int styleFlags ) const
{
	int cummulativeStyleFlags = 0;
	for ( int index = 0, size = m_string.length(); index < size; ++index )
	{
		cummulativeStyleFlags |= m_styleFlags[index];
		if ( (cummulativeStyleFlags & styleFlags) == styleFlags )
			return true;
	}
	return false;
}

SString& SString::setStyleFlags( int index, int len, int styleFlags )
{
	if ( index < 0 || index >= (int)m_string.length() )
		return *this;

	for ( int index2 = index + length( index, len ); index < index2; ++index )
		m_styleFlags[index] = styleFlags;

	return *this;
}

SString& SString::setStyleFlags( int index, int len, int styleFlags, bool on )
{
	if ( index < 0 || index >= (int)m_string.length() )
		return *this;

	if ( on )
	{
		for ( int index2 = index + length( index, len ); index < index2; ++index )
			m_styleFlags[index] = m_styleFlags[index] | styleFlags;
	}
	else
	{
		styleFlags = ~styleFlags;
		for ( int index2 = index + length( index, len ); index < index2; ++index )
			m_styleFlags[index] = m_styleFlags[index] & styleFlags;
	}

	return *this;
}

void SString::clear()
{
	m_string.clear();
	setMinFlagsCapacity( 0 );
}

void SString::truncate( int size )
{
	// no need to change m_styleFlags
	m_string.truncate( size );
}

SString& SString::insert( int index, QChar ch )
{
	int oldLength = m_string.length();

	if ( index <= oldLength && index >= 0 )
	{
		m_string.insert( index, ch );

		char* oldStyleFlags = detachFlags();
		setMinFlagsCapacity( m_string.length() );

		char fillFlags = 0;
		if ( oldLength )
		{
			if ( index == 0 )
				fillFlags = oldStyleFlags[0];
			else
				fillFlags = oldStyleFlags[index-1];
		}

		memcpy( m_styleFlags, oldStyleFlags, index );
		m_styleFlags[index] = fillFlags;
		memcpy( m_styleFlags + index + 1, oldStyleFlags + index, m_string.length() - index - 1 );

		delete [] oldStyleFlags;
	}

	return *this;
}

SString& SString::insert( int index, const QString& str )
{
	int oldLength = m_string.length();

	if ( str.length() && index <= oldLength && index >= 0 )
	{
		m_string.insert( index, str );

		char* oldStyleFlags = detachFlags();
		setMinFlagsCapacity( m_string.length() );

		char fillFlags = 0;
		if ( oldLength )
		{
			if ( index == 0 )
				fillFlags = oldStyleFlags[0];
			else
				fillFlags = oldStyleFlags[index-1];
		}

		int addedLength = str.length();
		memcpy( m_styleFlags, oldStyleFlags, index );
		memset( m_styleFlags + index, fillFlags, addedLength );
		memcpy( m_styleFlags + index + addedLength, oldStyleFlags + index, m_string.length() - index - addedLength );

		delete [] oldStyleFlags;
	}

	return *this;
}

SString& SString::insert( int index, const SString& str )
{
	int oldLength = m_string.length();

	if ( str.m_string.length() && index <= oldLength && index >= 0 )
	{
		m_string.insert( index, str.m_string );

		char* oldStyleFlags = detachFlags();
		setMinFlagsCapacity( m_string.length() );

		int addedLength = str.m_string.length();
		memcpy( m_styleFlags, oldStyleFlags, index );
		memcpy( m_styleFlags + index, str.m_styleFlags, addedLength );
		memcpy( m_styleFlags + index + addedLength, oldStyleFlags + index, m_string.length() - index - addedLength );

		delete [] oldStyleFlags;
	}

	return *this;
}

SString& SString::replace( int index, int len, const QString& replacement )
{
	int oldLength = m_string.length();

	if ( index < 0 || index >= oldLength )
		return *this;

	len = length( index, len );

	if ( len == 0 && replacement.length() == 0 ) // nothing to do (replace nothing with nothing)
		return *this;

	m_string.replace( index, len, replacement );

	// simple path for when there's no need to change the styles (char substitution)
	if ( len == 1 && replacement.length() == 1 )
		return *this;

	if ( len == replacement.length() ) // the length of the string wasn't changed
	{
		if ( index >= oldLength ) // index can't really be greater than oldLength
			memset( m_styleFlags + index, oldLength ? m_styleFlags[oldLength-1] : 0, len );
		else // index < oldLength (NOTE: index is always >= 0)
			memset( m_styleFlags + index, m_styleFlags[index], len );
	}
	else // the length of the string was changed
	{
		char* oldStyleFlags = detachFlags();
		setMinFlagsCapacity( m_string.length() );

		memcpy( m_styleFlags, oldStyleFlags, index );
		memset( m_styleFlags + index, oldStyleFlags[index], replacement.length() );
		memcpy(
			m_styleFlags + index + replacement.length(),
			oldStyleFlags + index + len,
			m_string.length() - index - replacement.length()
		);

		delete [] oldStyleFlags;
	}

	return *this;
}

SString& SString::replace( int index, int len, const SString& replacement )
{
	int oldLength = m_string.length();

	if ( index < 0 || index >= oldLength )
		return *this;

	len = length( index, len );

	if ( len == 0 && replacement.m_string.length() == 0 ) // nothing to do (replace nothing with nothing)
		return *this;

	m_string.replace( index, len, replacement.m_string );

	// simple path for when there's no need to change the styles (char substitution)
	if ( len == 1 && replacement.m_string.length() == 1 )
		return *this;

	char* oldStyleFlags = detachFlags();
	setMinFlagsCapacity( m_string.length() );

	memcpy( m_styleFlags, oldStyleFlags, index );
	memcpy( m_styleFlags + index, replacement.m_styleFlags, replacement.m_string.length() );
	memcpy(
		m_styleFlags + index + replacement.m_string.length(),
		oldStyleFlags + index + len,
		m_string.length() - index - replacement.m_string.length()
	);

	delete [] oldStyleFlags;

	return *this;
}

SString& SString::replace( const QString& before, const QString& after, Qt::CaseSensitivity cs )
{
	if ( before.length() == 0 && after.length() == 0 )
		return *this;

	if ( before.length() == 1 && after.length() == 1 )
	{
		// simple path for when there's no need to change the styles flags
		m_string.replace( before, after );
		return *this;
	}

	int oldLength = m_string.length();
	int beforeLength = before.length();
	int afterLength = after.length();

	QList<int> changedData; // each entry contains the start index of a replaced substring

	for ( int offsetIndex = 0, matchedIndex; (matchedIndex = m_string.indexOf( before, offsetIndex, cs )) != -1; offsetIndex = matchedIndex + afterLength )
	{
		m_string.replace( matchedIndex, beforeLength, after );

		changedData.append( matchedIndex );

		if ( ! beforeLength )
			matchedIndex++;
	}

	if ( changedData.empty() ) // nothing was replaced
		return *this;

	if ( m_string.length() )
	{
		int newOffset = 0;
		int oldOffset = 0;
		int unchangedLength;

		char* oldStyleFlags = detachFlags();
		setMinFlagsCapacity( m_string.length() );

		for ( int index = 0; index < changedData.size(); ++index )
		{
			unchangedLength = changedData[index] - newOffset;

			memcpy( m_styleFlags + newOffset, oldStyleFlags + oldOffset, unchangedLength );
			newOffset += unchangedLength;
			oldOffset += unchangedLength;

			memset( m_styleFlags + newOffset, oldOffset<oldLength ? oldStyleFlags[oldOffset] : 0, afterLength );
			newOffset += afterLength;
			oldOffset += beforeLength;
		}

		memcpy( m_styleFlags + newOffset, oldStyleFlags + oldOffset, oldLength - oldOffset );

		delete [] oldStyleFlags;
	}
	else
		setMinFlagsCapacity( m_string.length() );

	return *this;
}

SString& SString::replace( const QString& before, const SString& after, Qt::CaseSensitivity cs )
{
	if ( before.length() == 0 && after.m_string.length() == 0 )
		return *this;

	int oldLength = m_string.length();
	int beforeLength = before.length();
	int afterLength = after.length();

	QList<int> changedData; // each entry contains the start index of a replaced substring

	for ( int offsetIndex = 0, matchedIndex; (matchedIndex = m_string.indexOf( before, offsetIndex, cs )) != -1; offsetIndex = matchedIndex + afterLength )
	{
		m_string.replace( matchedIndex, beforeLength, after.m_string );

		changedData.append( matchedIndex );

		if ( ! beforeLength )
			matchedIndex++;
	}

	if ( changedData.empty() ) // nothing was replaced
		return *this;

	if ( m_string.length() )
	{
		int newOffset = 0;
		int oldOffset = 0;
		int unchangedLength;

		char* oldStyleFlags = detachFlags();
		setMinFlagsCapacity( m_string.length() );

		for ( int index = 0; index < changedData.size(); ++index )
		{
			unchangedLength = changedData[index] - newOffset;

			memcpy( m_styleFlags + newOffset, oldStyleFlags + oldOffset, unchangedLength );
			newOffset += unchangedLength;
			oldOffset += unchangedLength;

			memcpy( m_styleFlags + newOffset, after.m_styleFlags, afterLength );
			newOffset += afterLength;
			oldOffset += beforeLength;
		}

		memcpy( m_styleFlags + newOffset, oldStyleFlags + oldOffset, oldLength - oldOffset );

		delete [] oldStyleFlags;
	}
	else
		setMinFlagsCapacity( m_string.length() );

	return *this;
}

SString& SString::replace( QChar before, QChar after, Qt::CaseSensitivity cs )
{
	m_string.replace( before, after, cs );
	return *this;
}

SString& SString::replace( QChar ch, const QString& after, Qt::CaseSensitivity cs )
{
	if ( after.length() == 1 )
	{
		// simple path for when there's no need to change the styles flags
		m_string.replace( ch, after.at( 0 ) );
		return *this;
	}

	int oldLength = m_string.length();
	int afterLength = after.length();

	QList<int> changedData; // each entry contains the start index of a replaced substring

	for ( int offsetIndex = 0, matchedIndex; (matchedIndex = m_string.indexOf( ch, offsetIndex, cs )) != -1; offsetIndex = matchedIndex + afterLength )
	{
		m_string.replace( matchedIndex, 1, after );

		changedData.append( matchedIndex );
	}

	if ( changedData.empty() ) // nothing was replaced
		return *this;

	if ( m_string.length() )
	{
		int newOffset = 0;
		int oldOffset = 0;
		int unchangedLength;

		char* oldStyleFlags = detachFlags();
		setMinFlagsCapacity( m_string.length() );

		for ( int index = 0; index < changedData.size(); ++index )
		{
			unchangedLength = changedData[index] - newOffset;

			memcpy( m_styleFlags + newOffset, oldStyleFlags + oldOffset, unchangedLength );
			newOffset += unchangedLength;
			oldOffset += unchangedLength;

			memset( m_styleFlags + newOffset, oldOffset<oldLength ? oldStyleFlags[oldOffset] : 0, afterLength );
			newOffset += afterLength;
			oldOffset += 1;
		}

		memcpy( m_styleFlags + newOffset, oldStyleFlags + oldOffset, oldLength - oldOffset );

		delete [] oldStyleFlags;
	}
	else
		setMinFlagsCapacity( m_string.length() );

	return *this;
}

SString& SString::replace( QChar ch, const SString& after, Qt::CaseSensitivity cs )
{
	int oldLength = m_string.length();
	int afterLength = after.length();

	QList<int> changedData; // each entry contains the start index of a replaced substring

	for ( int offsetIndex = 0, matchedIndex; (matchedIndex = m_string.indexOf( ch, offsetIndex, cs )) != -1; offsetIndex = matchedIndex + afterLength )
	{
		m_string.replace( matchedIndex, 1, after.m_string );

		changedData.append( matchedIndex );
	}

	if ( changedData.empty() ) // nothing was replaced
		return *this;

	if ( m_string.length() )
	{
		int newOffset = 0;
		int oldOffset = 0;
		int unchangedLength;

		char* oldStyleFlags = detachFlags();
		setMinFlagsCapacity( m_string.length() );

		for ( int index = 0; index < changedData.size(); ++index )
		{
			unchangedLength = changedData[index] - newOffset;

			memcpy( m_styleFlags + newOffset, oldStyleFlags + oldOffset, unchangedLength );
			newOffset += unchangedLength;
			oldOffset += unchangedLength;

			memcpy( m_styleFlags + newOffset, after.m_styleFlags, afterLength );
			newOffset += afterLength;
			oldOffset += 1;
		}

		memcpy( m_styleFlags + newOffset, oldStyleFlags + oldOffset, oldLength - oldOffset );

		delete [] oldStyleFlags;
	}
	else
		setMinFlagsCapacity( m_string.length() );

	return *this;

}


SString& SString::replace( const QRegExp& rx, const QString& a )
{
	QRegExp regExp( rx );

	int oldLength = m_string.length();

	QList<int> changedData; // each entry contains the start index of a replaced substring

	QRegExp::CaretMode caretMode = QRegExp::CaretAtZero;
	for ( int offsetIndex = 0, matchedIndex; (matchedIndex = regExp.indexIn( m_string, offsetIndex, caretMode )) != -1; )
	{
		QString after( a );

		bool escaping = false;
		for ( int afterIndex = 0, afterSize = after.length(); afterIndex < afterSize; ++afterIndex )
		{
			QChar chr = after.at( afterIndex );
			if ( escaping ) // perform replace
			{
				escaping = false;
				if ( chr.isNumber() )
				{
					int capNumber = chr.digitValue();
					if ( capNumber <= regExp.numCaptures() )
					{
						QString cap( regExp.cap( capNumber ) );
						after.replace( afterIndex - 1, 2, cap );
						afterIndex = afterIndex - 1 + cap.length();
						afterSize = after.length();
					}
				}
			}
			else if ( chr == '\\' )
				escaping = ! escaping;
		}

		if ( regExp.matchedLength() == 0 && after.length() == 0 )
			continue;

		m_string.replace( matchedIndex, regExp.matchedLength(), after );

		if ( regExp.matchedLength() != 1 || after.length() != 1 )
		{
			changedData.append( matchedIndex );
			changedData.append( regExp.matchedLength() ); // before length
			changedData.append( after.length() ); // after length

			if ( ! regExp.matchedLength() )
				matchedIndex++;
		}

		offsetIndex = matchedIndex + after.length();
		caretMode = QRegExp::CaretWontMatch; // caret should only be matched the first time
	}

	if ( changedData.empty() ) // nothing was replaced
		return *this;

	if ( m_string.length() )
	{
		int newOffset = 0;
		int oldOffset = 0;
		int unchangedLength;
		int beforeLength;
		int afterLength;

		char* oldStyleFlags = detachFlags();
		setMinFlagsCapacity( m_string.length() );

		for ( int index = 0; index < changedData.size(); index += 3 )
		{
			unchangedLength = changedData[index] - newOffset;
			beforeLength = changedData[index+1];
			afterLength = changedData[index+2];

			memcpy( m_styleFlags + newOffset, oldStyleFlags + oldOffset, unchangedLength );
			newOffset += unchangedLength;
			oldOffset += unchangedLength;

			memset( m_styleFlags + newOffset, oldOffset<oldLength ? oldStyleFlags[oldOffset] : 0, afterLength );
			newOffset += afterLength;
			oldOffset += beforeLength;
		}

		memcpy( m_styleFlags + newOffset, oldStyleFlags + oldOffset, oldLength - oldOffset );

		delete [] oldStyleFlags;
	}
	else
		setMinFlagsCapacity( m_string.length() );

	return *this;
}

SString& SString::replace( const QRegExp& rx, const SString& a )
{
	QRegExp regExp( rx );

	QRegExp::CaretMode caretMode = QRegExp::CaretAtZero;
	for ( int offsetIndex = 0, matchedIndex; (matchedIndex = regExp.indexIn( m_string, offsetIndex, caretMode )) != -1; )
	{
		SString after( a );

		bool escaping = false;
		for ( int afterIndex = 0, afterSize = after.length(); afterIndex < afterSize; ++afterIndex )
		{
			QChar chr = after.at( afterIndex );
			if ( escaping ) // perform replace
			{
				escaping = false;
				if ( chr.isNumber() )
				{
					int capNumber = chr.digitValue();
					if ( capNumber <= regExp.numCaptures() )
					{
						QString cap( regExp.cap( capNumber ) );
						after.replace( afterIndex - 1, 2, cap );
						afterIndex = afterIndex - 1 + cap.length();
						afterSize = after.length();
					}
				}
			}
			else if ( chr == '\\' )
				escaping = ! escaping;
		}

		if ( regExp.matchedLength() == 0 && after.length() == 0 )
			continue;

		replace( matchedIndex, regExp.matchedLength(), after );

		if ( ! regExp.matchedLength() )
			matchedIndex++;

		offsetIndex = matchedIndex + after.length();
		caretMode = QRegExp::CaretWontMatch; // caret should only be matched the first time
	}

	return *this;
}

SStringList SString::split( const QString& sep, QString::SplitBehavior behavior, Qt::CaseSensitivity cs ) const
{
	SStringList ret;

	if ( sep.length() )
	{
		int offsetIndex = 0;

		for ( int matchedIndex;
			(matchedIndex = m_string.indexOf( sep, offsetIndex, cs )) != -1;
			offsetIndex = matchedIndex + sep.length() )
		{
			SString token( m_string.mid( offsetIndex, matchedIndex - offsetIndex ) );
			if ( behavior == QString::KeepEmptyParts || token.length() )
				ret << token;
		}

		SString token( m_string.mid( offsetIndex ) );
		if ( behavior == QString::KeepEmptyParts || token.length() )
			ret << token;
	}
	else if ( behavior == QString::KeepEmptyParts || length() )
		ret << *this;

	return ret;
}

SStringList SString::split( const QChar& sep, QString::SplitBehavior behavior, Qt::CaseSensitivity cs ) const
{
	SStringList ret;

	int offsetIndex = 0;

	for ( int matchedIndex;
		(matchedIndex = m_string.indexOf( sep, offsetIndex, cs )) != -1;
		offsetIndex = matchedIndex + 1 )
	{
		SString token( m_string.mid( offsetIndex, matchedIndex - offsetIndex ) );
		if ( behavior == QString::KeepEmptyParts || token.length() )
			ret << token;
	}

	SString token( m_string.mid( offsetIndex ) );
	if ( behavior == QString::KeepEmptyParts || token.length() )
		ret << token;

	return ret;
}

SStringList SString::split( const QRegExp& sep, QString::SplitBehavior behavior ) const
{
	SStringList ret;

	QRegExp sepAux( sep );

	int offsetIndex = 0;

	for ( int matchedIndex;
		(matchedIndex = sepAux.indexIn( m_string, offsetIndex )) != -1;
		offsetIndex = matchedIndex + sepAux.matchedLength() )
	{
		SString token( m_string.mid( offsetIndex, matchedIndex - offsetIndex ) );
		if ( behavior == QString::KeepEmptyParts || token.length() )
			ret << token;
	}

	SString token( m_string.mid( offsetIndex ) );
	if ( behavior == QString::KeepEmptyParts || token.length() )
		ret << token;

	return ret;
}

SString SString::left( int len ) const
{
	len = length( 0, len );
	SString ret;
	ret.m_string = m_string.left( len );
	ret.setMinFlagsCapacity( len );
	memcpy( ret.m_styleFlags, m_styleFlags, len );
	return ret;
}

SString SString::right( int len ) const
{
	len = length( 0, len );
	SString ret;
	ret.m_string = m_string.right( len );
	ret.setMinFlagsCapacity( len );
	memcpy( ret.m_styleFlags, m_styleFlags + m_string.length() - len, len );
	return ret;
}

SString SString::mid( int index, int len ) const
{
	if ( index < 0 )
	{
		if ( len >= 0 )
			len += index;
		index = 0;
	}

	if ( index >= (int)m_string.length() )
		return SString();

	len = length( index, len );
	SString ret;
	ret.m_string = m_string.mid( index, len );
	ret.setMinFlagsCapacity( len );
	memcpy( ret.m_styleFlags, m_styleFlags + index, len );
	return ret;
}

SString SString::toLower() const
{
	SString ret( *this );
	ret.m_string = m_string.toLower();
	return ret;
}

SString SString::toUpper() const
{
	SString ret( *this );
	ret.m_string = m_string.toUpper();
	return ret;
}

SString SString::toTitleCase( bool lowerFirst ) const
{
	const QString wordSeparators( " -_([:,;./\\\t\n\"" );

	SString ret( *this );

	if ( lowerFirst )
		ret.m_string = m_string.toLower();

	bool wordStart = true;
	for ( uint idx = 0, size = m_string.length(); idx < size; ++idx )
	{
		QCharRef chr = ret.m_string[idx];
		if ( wordStart )
		{
			if ( ! wordSeparators.contains( chr ) )
			{
				wordStart = false;
				chr = chr.toUpper();
			}
		}
		else if ( wordSeparators.contains( chr ) )
			wordStart = true;
	}

	return ret;
}

SString SString::toSentenceCase( bool lowerFirst, bool* cont ) const
{
	const QString sentenceEndChars( ".?!" );

	SString ret( *this );

	if ( lowerFirst )
		ret.m_string = m_string.toLower();

	if ( m_string.isEmpty() )
		return ret;

	uint prevDots = 0;
	bool startSentence = cont ? ! *cont : true;

	for ( uint index = 0, size = m_string.length(); index < size; ++index )
	{
		QCharRef chr = ret.m_string[index];

		if ( sentenceEndChars.contains( chr ) )
		{
			if ( chr == '.' )
			{
				prevDots++;
				startSentence = prevDots < 3;
			}
			else
			{
				prevDots = 0;
				startSentence = true;
			}
		}
		else
		{
			if ( startSentence && chr.isLetterOrNumber() )
			{
				chr = chr.toUpper();
				startSentence = false;
			}

			if ( ! chr.isSpace() )
				prevDots = 0;
		}
	}

	if ( cont )
		*cont = prevDots != 1 && ! startSentence;

	return ret;
}

SString SString::simplified() const
{
	const QRegExp simplifySpaceRegExp( "\\s{2,MAXINT}" );

	return trimmed().replace( simplifySpaceRegExp, " " );
}


SString SString::trimmed() const
{
	const QRegExp trimRegExp( "(^\\s+|\\s+$)" );

	SString ret( *this );
	return ret.remove( trimRegExp );
}

bool SString::operator!=( const SString& sstring ) const
{
	if ( m_string != sstring.m_string )
		return true;

	for ( int index = 0, size = m_string.length(); index < size; ++index )
		if ( m_styleFlags[index] != sstring.m_styleFlags[index] )
			return true;

	return false;
}

char* SString::detachFlags()
{
	char* ret = m_styleFlags;
	m_styleFlags = 0;
	m_capacity = 0;
	return ret;
}

void SString::setMinFlagsCapacity( int capacity )
{
	if ( capacity > m_capacity )
	{
		m_capacity = capacity * 2;
		delete [] m_styleFlags;
		m_styleFlags = new char[m_capacity];
	}
	else if ( capacity == 0 )
	{
		m_capacity = 0;
		delete [] m_styleFlags;
		m_styleFlags = 0;
	}
	else if ( m_capacity > 100 && capacity < m_capacity / 2 )
	{
		m_capacity = m_capacity / 2;
		delete [] m_styleFlags;
		m_styleFlags = new char[m_capacity];
	}
}



SStringList::SStringList()
{
}

SStringList::SStringList( const SString& str )
{
	append( str );
}

SStringList::SStringList( const SStringList::SStringList& list ):
	QList<SString>( list )
{
}

SStringList::SStringList( const QList<SString>& list ):
	QList<SString>( list )
{
}

SStringList::SStringList( const QStringList& list )
{
	for ( QStringList::ConstIterator it = list.begin(), end = list.end(); it != end; ++it )
		append( *it );
}

SStringList::SStringList( const QList<QString>& list )
{
	for ( QList<QString>::ConstIterator it = list.begin(), end = list.end(); it != end; ++it )
		append( *it );
}

SString SStringList::join( const SString& sep ) const
{
	SString ret;

	bool skipSeparator = true;
	for ( SStringList::ConstIterator it = begin(), end = this->end(); it != end; ++it )
	{
		if ( skipSeparator )
		{
			ret += *it;
			skipSeparator = false;
			continue;
		}
		ret += sep;
		ret += *it;
	}

	return ret;
}
