#ifndef SSTRING_H
#define SSTRING_H

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

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include <string.h>

#include <QtCore/QString>
#include <QtCore/QRegExp>
#include <QtCore/QList>

#include <KDebug>

namespace SubtitleComposer
{
	class SString;

	class SStringList : public QList<SString>
	{
		public:

			SStringList();
			SStringList( const SString& str );
			SStringList( const SStringList& list );
			SStringList( const QList<SString>& list );
			SStringList( const QStringList& list );
			SStringList( const QList<QString>& list );

			SString join( const SString& sep ) const;
	};

	class SString
	{
		public:

			typedef enum {
				Bold =			0x1,
				Italic =		0x2,
				Underline = 	0x4,
				StrikeThrough = 0x8,
				AllStyles =		Bold|Italic|Underline|StrikeThrough
			} StyleFlag;

			typedef enum {
				Compact,
				Verbose
			} RichOutputMode;

			SString( const QString& string=QString(), int styleFlags=0 ); // krazy:exclude=c++/explicit
			SString( const SString& sstring );
			SString& operator=( const SString& sstring );

			~SString();

			bool isNull() const;
			bool isEmpty() const;

			int length() const;
			int size() const;

			QString string() const;
			void setString( const QString& string, int styleFlags=0 ); // always clears all the style flags

			QString richString( RichOutputMode mode=Compact ) const;
			SString& setRichString( const QString& string );

			const QChar at( int index );
			QCharRef operator[]( int index );
			const QChar operator[]( int index ) const;

			int styleFlagsAt( int index ) const;
			void setStyleFlagsAt( int index, int styleFlags ) const;

			int cummulativeStyleFlags() const;
			bool hasStyleFlags( int styleFlags ) const;
			SString& setStyleFlags( int index, int len, int styleFlags );
			SString& setStyleFlags( int index, int len, int styleFlags, bool on );

			void clear();
			void truncate( int size );

			SString& insert( int index, QChar ch );
			SString& insert( int index, const QString& str );
			SString& insert( int index, const SString& str );
			SString& append( QChar ch );
			SString& append( const QString& str );
			SString& append( const SString& str );
			SString& prepend( QChar ch );
			SString& prepend( const QString& str );
			SString& prepend( const SString& str );
			SString& operator+=( QChar ch );
			SString& operator+=( const QString& str );
			SString& operator+=( const SString& str );


			SString& remove( int index, int len );
			SString& remove( const QString& str, Qt::CaseSensitivity cs=Qt::CaseSensitive );
			SString& remove( QChar ch, Qt::CaseSensitivity cs=Qt::CaseSensitive );
			SString& remove( const QRegExp& rx );

			SString& replace( int index, int len, const QString& replacement );
			SString& replace( int index, int len, const SString& replacement );
			SString& replace( const QString& before, const QString& after, Qt::CaseSensitivity cs=Qt::CaseSensitive );
			SString& replace( const QString& before, const SString& after, Qt::CaseSensitivity cs=Qt::CaseSensitive );
			SString& replace( QChar before, QChar after, Qt::CaseSensitivity cs=Qt::CaseSensitive );
			SString& replace( QChar ch, const QString& after, Qt::CaseSensitivity cs=Qt::CaseSensitive );
			SString& replace( QChar ch, const SString& after, Qt::CaseSensitivity cs=Qt::CaseSensitive );
			SString& replace( const QRegExp& rx, const QString& after );
			SString& replace( const QRegExp& rx, const SString& after );

			int indexOf( QChar c, int index=0, Qt::CaseSensitivity cs=Qt::CaseSensitive ) const;
			int indexOf( char c, int index=0, Qt::CaseSensitivity cs=Qt::CaseSensitive ) const;
			int indexOf( const QString& str, int index=0, Qt::CaseSensitivity cs=Qt::CaseSensitive ) const;
			int indexOf( const QRegExp& rx, int index=0 ) const;
			int indexOf( const char* str, int index=0 ) const;

			int lastIndexOf( QChar c, int index=-1, Qt::CaseSensitivity cs=Qt::CaseSensitive ) const;
			int lastIndexOf( char c, int index=-1, Qt::CaseSensitivity cs=Qt::CaseSensitive ) const;
			int lastIndexOf( const QString& str, int index=-1, Qt::CaseSensitivity cs=Qt::CaseSensitive ) const;
			int lastIndexOf( const QRegExp& rx, int index=-1 ) const;
			int lastIndexOf( const char* str, int index=-1 ) const;

			bool contains( QChar c, Qt::CaseSensitivity cs=Qt::CaseSensitive ) const;
			bool contains( const QString& str, Qt::CaseSensitivity cs=Qt::CaseSensitive ) const;
			bool contains( const QRegExp& rx ) const;

			int count( const QString& str, Qt::CaseSensitivity cs=Qt::CaseSensitive ) const;
			int count( QChar ch, Qt::CaseSensitivity cs=Qt::CaseSensitive ) const;
			int count( const QRegExp& rx ) const;
			int count() const;

			SStringList split( const QString& sep, QString::SplitBehavior behavior=QString::KeepEmptyParts, Qt::CaseSensitivity cs=Qt::CaseSensitive ) const;
			SStringList split( const QChar& sep, QString::SplitBehavior behavior=QString::KeepEmptyParts, Qt::CaseSensitivity cs=Qt::CaseSensitive ) const;
			SStringList split( const QRegExp& sep, QString::SplitBehavior behavior=QString::KeepEmptyParts ) const;

			SString left( int len ) const;
			SString right( int len ) const;
			SString mid( int index, int len=-1 ) const;

			SString toLower() const;
			SString toUpper() const;
			SString toTitleCase( bool lowerFirst ) const;
			SString toSentenceCase( bool lowerFirst, bool* cont ) const;

			SString simplified() const;
			SString trimmed() const;

			bool operator==( const SString& sstring ) const;
			bool operator==( const QString& string ) const;
			bool operator!=( const SString& sstring ) const;
			bool operator!=( const QString& string ) const;

			bool operator<( const SString& sstring ) const;
			bool operator<( const QString& string ) const;
			bool operator<=( const SString& sstring ) const;
			bool operator<=( const QString& string ) const;
			bool operator>( const SString& sstring ) const;
			bool operator>( const QString& string ) const;
			bool operator>=( const SString& sstring ) const;
			bool operator>=( const QString& string ) const;

		private:

			char* detachFlags();
			void setMinFlagsCapacity( int capacity );

			int length( int index, int len ) const;

		private:

			QString m_string;
			char* m_styleFlags;
			int m_capacity;
	};

	inline bool SString::isNull() const
	{
		return m_string.isNull();
	}

	inline bool SString::isEmpty() const
	{
		return m_string.isEmpty();
	}

	inline int SString::length() const
	{
		return m_string.length();
	}

	inline int SString::size() const
	{
		return m_string.size();
	}

	inline QString SString::string() const
	{
		return m_string;
	}

	inline const QChar SString::at( int index )
	{
		return m_string.at( index );
	}

	inline QCharRef SString::operator[]( int index )
	{
		return m_string[index];
	}

	inline const QChar SString::operator[]( int index ) const
	{
		return m_string[index];
	}

	inline int SString::styleFlagsAt( int index ) const
	{
		return m_styleFlags[index];
	}

	inline void SString::setStyleFlagsAt( int index, int styleFlags ) const
	{
		m_styleFlags[index] = styleFlags;
	}

	inline SString& SString::append( QChar ch )
	{
		return insert( m_string.length(), ch );
	}

	inline SString& SString::append( const QString& str )
	{
		return insert( m_string.length(), str );
	}

	inline SString& SString::append( const SString& str )
	{
		return insert( m_string.length(), str );
	}

	inline SString& SString::prepend( QChar ch )
	{
		return insert( 0, ch );
	}

	inline SString& SString::prepend( const QString& str )
	{
		return insert( 0, str );
	}

	inline SString& SString::prepend( const SString& str )
	{
		return insert( 0, str );
	}

	inline SString& SString::operator+=( const QChar ch )
	{
		return append( ch );
	}

	inline SString& SString::operator+=( const QString& str )
	{
		return append( str );
	}

	inline SString& SString::operator+=( const SString& str )
	{
		return append( str );
	}

	inline SString& SString::remove( int index, int len )
	{
		return replace( index, len, "" );
	}

	inline SString& SString::remove( const QString& str, Qt::CaseSensitivity cs )
	{
		return replace( str, "", cs );
	}

	inline SString& SString::remove( QChar ch, Qt::CaseSensitivity cs )
	{
		return replace( ch, "", cs );
	}

	inline SString& SString::remove( const QRegExp& rx )
	{
		return replace( rx, "" );
	}

	inline int SString::indexOf( QChar c, int index, Qt::CaseSensitivity cs ) const
	{
		return m_string.indexOf( c, index, cs );
	}

	inline int SString::indexOf( char c, int index, Qt::CaseSensitivity cs ) const
	{
		return m_string.indexOf( c, index, cs );
	}

	inline int SString::indexOf( const QString& str, int index, Qt::CaseSensitivity cs ) const
	{
		return m_string.indexOf( str, index, cs );
	}

	inline int SString::indexOf( const QRegExp& rx, int index ) const
	{
		return m_string.indexOf( rx, index );
	}

	inline int SString::indexOf( const char* str, int index ) const
	{
		return m_string.indexOf( str, index );
	}

	inline int SString::lastIndexOf( QChar c, int index, Qt::CaseSensitivity cs ) const
	{
		return m_string.lastIndexOf( c, index, cs );
	}

	inline int SString::lastIndexOf( char c, int index, Qt::CaseSensitivity cs ) const
	{
		return m_string.lastIndexOf( c, index, cs );
	}

	inline int SString::lastIndexOf( const QString& str, int index, Qt::CaseSensitivity cs ) const
	{
		return m_string.lastIndexOf( str, index, cs );
	}

	inline int SString::lastIndexOf( const QRegExp& rx, int index ) const
	{
		return m_string.lastIndexOf( rx, index );
	}

	inline int SString::lastIndexOf( const char* str, int index ) const
	{
		return m_string.lastIndexOf( str, index );
	}

	inline bool SString::contains( QChar chr, Qt::CaseSensitivity cs ) const
	{
		return m_string.contains( chr, cs );
	}

	inline bool SString::contains( const QString& str, Qt::CaseSensitivity cs ) const
	{
		return m_string.contains( str, cs );
	}

	inline bool SString::contains( const QRegExp& rx ) const
	{
		return m_string.contains( rx );
	}

	inline int SString::count( const QString& str, Qt::CaseSensitivity cs ) const
	{
		return m_string.count( str, cs );
	}

	inline int SString::count( QChar chr, Qt::CaseSensitivity cs ) const
	{
		return m_string.count( chr, cs );
	}

	inline int SString::count( const QRegExp& rx ) const
	{
		return m_string.count( rx );
	}

	inline int SString::count() const
	{
		return m_string.count();
	}

	inline bool SString::operator==( const SString& sstring ) const
	{
		return ! operator!=( sstring );
	}

	inline bool SString::operator==( const QString& string ) const
	{
		return m_string == string;
	}

	inline bool SString::operator!=( const QString& string ) const
	{
		return m_string != string;
	}

	inline bool SString::operator<( const SString& sstring ) const
	{
		return m_string < sstring.m_string;
	}

	inline bool SString::operator<( const QString& string ) const
	{
		return m_string < string;
	}

	inline bool SString::operator<=( const SString& sstring ) const
	{
		return m_string <= sstring.m_string;
	}

	inline bool SString::operator<=( const QString& string ) const
	{
		return m_string <= string;
	}

	inline bool SString::operator>( const SString& sstring ) const
	{
		return m_string > sstring.m_string;
	}

	inline bool SString::operator>( const QString& string ) const
	{
		return m_string > string;
	}

	inline bool SString::operator>=( const SString& sstring ) const
	{
		return m_string >= sstring.m_string;
	}

	inline bool SString::operator>=( const QString& string ) const
	{
		return m_string >= string;
	}

	inline int SString::length( int index, int len ) const
	{
		return (len < 0 || (index + len) > (int)m_string.length()) ? m_string.length() - index : len;
	}
}

#endif
