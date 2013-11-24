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

void *
memset_n(void *ptr, int value, size_t length, size_t size)
{
	unsigned char *data = (unsigned char *)ptr;
	while(length > 0) {
		memcpy(data, &value, size);
		data += size;
		length--;
	}
	return ptr;
}

SString::SString(const QString &string, int styleFlags /* = 0*/, QRgb styleColor /* = 0*/) :
	m_string(string),
	m_styleFlags(NULL),
	m_styleColors(NULL),
	m_capacity(0)
{
	if(m_string.length()) {
		setMinFlagsCapacity(m_string.length());
		memset(m_styleFlags, styleFlags & AllStyles, m_string.length() * sizeof(*m_styleFlags));
		memset_n(m_styleColors, styleColor, m_string.length(), sizeof(*m_styleColors));
	}
}

SString::SString(const SString &sstring) :
	m_string(sstring.m_string),
	m_styleFlags(NULL),
	m_styleColors(NULL),
	m_capacity(0)
{
	if(m_string.length()) {
		setMinFlagsCapacity(m_string.length());
		memcpy(m_styleFlags, sstring.m_styleFlags, m_string.length() * sizeof(*m_styleFlags));
		memcpy(m_styleColors, sstring.m_styleColors, m_string.length() * sizeof(*m_styleColors));
	}
}

SString &
SString::operator=(const SString &sstring)
{
	if(this == &sstring)
		return *this;

	m_string = sstring.m_string;
	setMinFlagsCapacity(m_string.length());

	if(m_string.length()) {
		memcpy(m_styleFlags, sstring.m_styleFlags, m_string.length() * sizeof(*m_styleFlags));
		memcpy(m_styleColors, sstring.m_styleColors, m_string.length() * sizeof(*m_styleColors));
	}

	return *this;
}

SString::~SString()
{
	delete[] m_styleFlags;
	delete[] m_styleColors;
}

void
SString::setString(const QString &string, int styleFlags /* = 0*/, QRgb styleColor /* = 0*/)
{
	m_string = string;
	setMinFlagsCapacity(m_string.length());
	if(m_string.length()) {
		memset(m_styleFlags, styleFlags & AllStyles, m_string.length() * sizeof(*m_styleFlags));
		memset_n(m_styleColors, styleColor, m_string.length(), sizeof(*m_styleColors));
	}
}

QString
SString::richString(RichOutputMode mode) const
{
	if(m_string.isEmpty())
		return m_string;

	QString ret;

	if(mode == Compact) {
		char prevStyleFlags = m_styleFlags[0];
		QRgb prevStyleColor = m_styleColors[0];
		int prevIndex = 0;

		if(prevStyleFlags & Italic)
			ret += "<i>";
		if(prevStyleFlags & Bold)
			ret += "<b>";
		if(prevStyleFlags & Underline)
			ret += "<u>";
		if(prevStyleFlags & StrikeThrough)
			ret += "<s>";
		if(prevStyleFlags & Color)
			ret += "<font color=" + QColor(prevStyleColor).name() + ">";

		const int size = m_string.length();
		QChar ch;
		for(int index = 1; index < size; ++index) {
			if(m_styleFlags[index] != prevStyleFlags || ((prevStyleFlags & m_styleFlags[index] & Color) && m_styleColors[index] != prevStyleColor)) {
				QString token(m_string.mid(prevIndex, index - prevIndex));
				ret += token.replace('<', "&lt;").replace('>', "&gt;");

				if((prevStyleFlags & StrikeThrough) && !(m_styleFlags[index] & StrikeThrough))
					ret += "</s>";
				if((prevStyleFlags & Underline) && !(m_styleFlags[index] & Underline))
					ret += "</u>";
				if((prevStyleFlags & Bold) && !(m_styleFlags[index] & Bold))
					ret += "</b>";
				if((prevStyleFlags & Italic) && !(m_styleFlags[index] & Italic))
					ret += "</i>";
				if((prevStyleFlags & Color) && (!(m_styleFlags[index] & Color) || prevStyleColor != m_styleColors[index]))
					ret += "</font>";

				while(index < size) {
					// place opening html tags after spaces/newlines
					ch = m_string.at(index);
					if(ch != '\n' && ch != '\r' && ch != ' ' && ch != '\t')
						break;
					ret += ch;
					index++;
				}

				if(!(prevStyleFlags & Italic) && (m_styleFlags[index] & Italic))
					ret += "<i>";
				if(!(prevStyleFlags & Bold) && (m_styleFlags[index] & Bold))
					ret += "<b>";
				if(!(prevStyleFlags & Underline) && (m_styleFlags[index] & Underline))
					ret += "<u>";
				if(!(prevStyleFlags & StrikeThrough) && (m_styleFlags[index] & StrikeThrough))
					ret += "<s>";
				if((m_styleFlags[index] & Color) && (!(prevStyleFlags & Color) || prevStyleColor != m_styleColors[index]))
					ret += "<font color=" + QColor(m_styleColors[index]).name() + ">";

				prevIndex = index;
				prevStyleFlags = m_styleFlags[index];
				prevStyleColor = m_styleColors[index];
			}
		}
		QString token(m_string.mid(prevIndex, m_string.length() - prevIndex));
		if(token.length()) {
			ret += token.replace('<', "&lt;").replace('>', "&gt;");

			if(prevStyleFlags & StrikeThrough)
				ret += "</s>";
			if(prevStyleFlags & Underline)
				ret += "</u>";
			if(prevStyleFlags & Bold)
				ret += "</b>";
			if(prevStyleFlags & Italic)
				ret += "</i>";
			if(prevStyleFlags & Color)
				ret += "</font>";
		}
	} else { // outputMode == Verbose
		int currentStyleFlags = m_styleFlags[0];
		QRgb currentColor = m_styleColors[0];
		int prevIndex = 0;
		for(uint index = 1, size = m_string.length(); index < size; ++index) {
			if(currentStyleFlags != m_styleFlags[index] || ((currentStyleFlags & m_styleFlags[index] & Color) && currentColor != m_styleColors[index])) {
				if(currentStyleFlags & StrikeThrough)
					ret += "<s>";
				if(currentStyleFlags & Bold)
					ret += "<b>";
				if(currentStyleFlags & Italic)
					ret += "<i>";
				if(currentStyleFlags & Underline)
					ret += "<u>";
				if(currentStyleFlags & Color)
					ret += "<font color=" + QColor(currentColor).name() + ">";

				ret += m_string.mid(prevIndex, index - prevIndex);

				if(currentStyleFlags & Color)
					ret += "</font>";
				if(currentStyleFlags & Underline)
					ret += "</u>";
				if(currentStyleFlags & Italic)
					ret += "</i>";
				if(currentStyleFlags & Bold)
					ret += "</b>";
				if(currentStyleFlags & StrikeThrough)
					ret += "</s>";

				prevIndex = index;

				currentStyleFlags = m_styleFlags[index];
				currentColor = m_styleColors[index];
			}
		}

		if(prevIndex + 1 < m_string.length()) {
			if(currentStyleFlags & StrikeThrough)
				ret += "<s>";
			if(currentStyleFlags & Bold)
				ret += "<b>";
			if(currentStyleFlags & Italic)
				ret += "<i>";
			if(currentStyleFlags & Underline)
				ret += "<u>";
			if(currentStyleFlags & Color)
				ret += "<font color=" + QColor(currentColor).name() + ">";

			ret += m_string.mid(prevIndex);

			if(currentStyleFlags & Color)
				ret += "</font>";
			if(currentStyleFlags & Underline)
				ret += "</u>";
			if(currentStyleFlags & Italic)
				ret += "</i>";
			if(currentStyleFlags & Bold)
				ret += "</b>";
			if(currentStyleFlags & StrikeThrough)
				ret += "</s>";
		}
	}

	return ret;
}

SString &
SString::setRichString(const QString &string)
{
	QRegExp tagRegExp("<(/?([bBiIuUsS]|font))(\\s+color=\"?([\\w#]+)\"?)?>");

	m_string.clear();

	int currentStyle = 0;
	QColor currentColor;
	int offsetPos = 0, matchedPos;
	while((matchedPos = tagRegExp.indexIn(string, offsetPos)) != -1) {
		QString matched(tagRegExp.cap(1).toLower());

		int newStyle = currentStyle;
		QColor newColor(currentColor);

		if(matched == "b") {
			newStyle |= SString::Bold;
		} else if(matched == "i") {
			newStyle |= SString::Italic;
		} else if(matched == "u") {
			newStyle |= SString::Underline;
		} else if(matched == "s") {
			newStyle |= SString::StrikeThrough;
		} else if(matched == "font") {
			newStyle |= SString::Color;
			newColor.setNamedColor(tagRegExp.cap(4).toLower());
		} else if(matched == "/b") {
			newStyle &= ~SString::Bold;
		} else if(matched == "/i") {
			newStyle &= ~SString::Italic;
		} else if(matched == "/u") {
			newStyle &= ~SString::Underline;
		} else if(matched == "/s") {
			newStyle &= ~SString::StrikeThrough;
		} else if(matched == "/font") {
			newStyle &= ~SString::Color;
			newColor.setNamedColor("-invalid-");
		}

		if(newStyle != currentStyle || currentColor != newColor) {
			QString token(string.mid(offsetPos, matchedPos - offsetPos));
			append(SString(token /*.replace("&lt;", "<").replace("&gt;", ">")*/, currentStyle, currentColor.isValid() ? currentColor.rgb() : 0));
			currentStyle = newStyle;
			currentColor = newColor;
		}

		offsetPos = matchedPos + tagRegExp.cap(0).length();
	}

	QString token(string.mid(offsetPos, matchedPos - offsetPos));
	append(SString(token /*.replace("&lt;", "<").replace("&gt;", ">")*/, currentStyle, currentColor.isValid() ? currentColor.rgb() : 0));

	return *this;
}

int
SString::cummulativeStyleFlags() const
{
	int cummulativeStyleFlags = 0;
	for(int index = 0, size = m_string.length(); index < size; ++index) {
		cummulativeStyleFlags |= m_styleFlags[index];
		if(cummulativeStyleFlags == AllStyles)
			break;
	}
	return cummulativeStyleFlags;
}

bool
SString::hasStyleFlags(int styleFlags) const
{
	int cummulativeStyleFlags = 0;
	for(int index = 0, size = m_string.length(); index < size; ++index) {
		cummulativeStyleFlags |= m_styleFlags[index];
		if((cummulativeStyleFlags & styleFlags) == styleFlags)
			return true;
	}
	return false;
}

SString &
SString::setStyleFlags(int index, int len, int styleFlags)
{
	if(index < 0 || index >= (int)m_string.length())
		return *this;

	for(int index2 = index + length(index, len); index < index2; ++index)
		m_styleFlags[index] = styleFlags;

	return *this;
}

SString &
SString::setStyleFlags(int index, int len, int styleFlags, bool on)
{
	if(index < 0 || index >= (int)m_string.length())
		return *this;

	if(on) {
		for(int index2 = index + length(index, len); index < index2; ++index)
			m_styleFlags[index] = m_styleFlags[index] | styleFlags;
	} else {
		styleFlags = ~styleFlags;
		for(int index2 = index + length(index, len); index < index2; ++index)
			m_styleFlags[index] = m_styleFlags[index] & styleFlags;
	}

	return *this;
}

SString &
SString::setStyleColor(int index, int len, QRgb color)
{
	if(index < 0 || index >= (int)m_string.length())
		return *this;

	for(int sz = index + length(index, len); index < sz; index++) {
		m_styleColors[index] = color;
		if(color == 0)
			m_styleFlags[index] &= ~Color;
		else
			m_styleFlags[index] |= Color;
	}

	return *this;
}

void
SString::clear()
{
	m_string.clear();
	setMinFlagsCapacity(0);
}

void
SString::truncate(int size)
{
	// no need to change m_styleFlags
	m_string.truncate(size);
}

SString &
SString::insert(int index, QChar ch)
{
	int oldLength = m_string.length();

	if(index <= oldLength && index >= 0) {
		m_string.insert(index, ch);

		char *oldStyleFlags = detachFlags();
		QRgb *oldStyleColors = detachColors();
		setMinFlagsCapacity(m_string.length());

		char fillFlags = 0;
		int fillColor = 0;
		if(oldLength) {
			if(index == 0) {
				fillFlags = oldStyleFlags[0];
				fillColor = oldStyleColors[0];
			} else {
				fillFlags = oldStyleFlags[index - 1];
				fillColor = oldStyleColors[index - 1];
			}
		}

		memcpy(m_styleFlags, oldStyleFlags, index * sizeof(*m_styleFlags));
		m_styleFlags[index] = fillFlags;
		memcpy(m_styleFlags + index + 1, oldStyleFlags + index, (m_string.length() - index - 1) * sizeof(*m_styleFlags));

		memcpy(m_styleColors, oldStyleColors, index * sizeof(*m_styleColors));
		m_styleColors[index] = fillColor;
		memcpy(m_styleColors + index + 1, oldStyleColors + index, (m_string.length() - index - 1) * sizeof(*m_styleColors));

		delete[] oldStyleFlags;
		delete[] oldStyleColors;
	}

	return *this;
}

SString &
SString::insert(int index, const QString &str)
{
	int oldLength = m_string.length();

	if(str.length() && index <= oldLength && index >= 0) {
		m_string.insert(index, str);

		char *oldStyleFlags = detachFlags();
		QRgb *oldStyleColors = detachColors();
		setMinFlagsCapacity(m_string.length());

		char fillFlags = 0;
		int fillColor = 0;
		if(oldLength) {
			if(index == 0) {
				fillFlags = oldStyleFlags[0];
				fillColor = oldStyleColors[0];
			} else {
				fillFlags = oldStyleFlags[index - 1];
				fillColor = oldStyleColors[index - 1];
			}
		}

		int addedLength = str.length();

		memcpy(m_styleFlags, oldStyleFlags, index * sizeof(*m_styleFlags));
		memset(m_styleFlags + index, fillFlags, addedLength * sizeof(*m_styleFlags));
		memcpy(m_styleFlags + index + addedLength, oldStyleFlags + index, (m_string.length() - index - addedLength) * sizeof(*m_styleFlags));

		memcpy(m_styleColors, oldStyleColors, index * sizeof(*m_styleColors));
		memset_n(m_styleColors + index, fillColor, addedLength, sizeof(*m_styleColors));
		memcpy(m_styleColors + index + addedLength, oldStyleColors + index, (m_string.length() - index - addedLength) * sizeof(*m_styleColors));

		delete[] oldStyleFlags;
		delete[] oldStyleColors;
	}

	return *this;
}

SString &
SString::insert(int index, const SString &str)
{
	int oldLength = m_string.length();

	if(str.m_string.length() && index <= oldLength && index >= 0) {
		m_string.insert(index, str.m_string);

		char *oldStyleFlags = detachFlags();
		QRgb *oldStyleColors = detachColors();
		setMinFlagsCapacity(m_string.length());

		int addedLength = str.m_string.length();

		memcpy(m_styleFlags, oldStyleFlags, index * sizeof(*m_styleFlags));
		memcpy(m_styleFlags + index, str.m_styleFlags, addedLength * sizeof(*m_styleFlags));
		memcpy(m_styleFlags + index + addedLength, oldStyleFlags + index, (m_string.length() - index - addedLength) * sizeof(*m_styleFlags));

		memcpy(m_styleColors, oldStyleColors, index * sizeof(*m_styleColors));
		memcpy(m_styleColors + index, str.m_styleColors, addedLength * sizeof(*m_styleColors));
		memcpy(m_styleColors + index + addedLength, oldStyleColors + index, (m_string.length() - index - addedLength) * sizeof(*m_styleColors));

		delete[] oldStyleFlags;
		delete[] oldStyleColors;
	}

	return *this;
}

SString &
SString::replace(int index, int len, const QString &replacement)
{
	int oldLength = m_string.length();

	if(index < 0 || index >= oldLength)
		return *this;

	len = length(index, len);

	if(len == 0 && replacement.length() == 0) // nothing to do (replace nothing with nothing)
		return *this;

	m_string.replace(index, len, replacement);

	// simple path for when there's no need to change the styles (char substitution)
	if(len == 1 && replacement.length() == 1)
		return *this;

	if(len == replacement.length()) {
		// the length of the string wasn't changed
		if(index >= oldLength) {
			// index can't really be greater than oldLength
			memset(m_styleFlags + index, oldLength ? m_styleFlags[oldLength - 1] : 0, len * sizeof(*m_styleFlags));
			memset_n(m_styleColors + index, oldLength ? m_styleColors[oldLength - 1] : 0, len, sizeof(*m_styleColors));
		} else {
			// index < oldLength (NOTE: index is always >= 0)
			memset(m_styleFlags + index, m_styleFlags[index], len * sizeof(*m_styleFlags));
			memset_n(m_styleColors + index, m_styleColors[oldLength - 1], len, sizeof(*m_styleColors));
		}
	} else {
		// the length of the string was changed
		char *oldStyleFlags = detachFlags();
		QRgb *oldStyleColors = detachColors();
		setMinFlagsCapacity(m_string.length());

		memcpy(m_styleFlags, oldStyleFlags, index * sizeof(*m_styleFlags));
		memset(m_styleFlags + index, oldStyleFlags[index], replacement.length() * sizeof(*m_styleFlags));
		memcpy(m_styleFlags + index + replacement.length(), oldStyleFlags + index + len, (m_string.length() - index - replacement.length()) * sizeof(*m_styleFlags));

		memcpy(m_styleColors, oldStyleColors, index * sizeof(*m_styleColors));
		memset_n(m_styleColors + index, oldStyleColors[index], replacement.length(), sizeof(*m_styleColors));
		memcpy(m_styleColors + index + replacement.length(), oldStyleColors + index + len, (m_string.length() - index - replacement.length()) * sizeof(*m_styleColors));

		delete[] oldStyleFlags;
		delete[] oldStyleColors;
	}

	return *this;
}

SString &
SString::replace(int index, int len, const SString &replacement)
{
	int oldLength = m_string.length();

	if(index < 0 || index >= oldLength)
		return *this;

	len = length(index, len);

	if(len == 0 && replacement.m_string.length() == 0) // nothing to do (replace nothing with nothing)
		return *this;

	m_string.replace(index, len, replacement.m_string);

	// simple path for when there's no need to change the styles (char substitution)
	// if ( len == 1 && replacement.m_string.length() == 1 )
	//  return *this;

	char *oldStyleFlags = detachFlags();
	QRgb *oldStyleColors = detachColors();
	setMinFlagsCapacity(m_string.length());

	memcpy(m_styleFlags, oldStyleFlags, index * sizeof(*m_styleFlags));
	memcpy(m_styleFlags + index, replacement.m_styleFlags, replacement.m_string.length() * sizeof(*m_styleFlags));
	memcpy(m_styleFlags + index + replacement.m_string.length(), oldStyleFlags + index + len, (m_string.length() - index - replacement.m_string.length()) * sizeof(*m_styleFlags));

	memcpy(m_styleColors, oldStyleColors, index * sizeof(*m_styleColors));
	memcpy(m_styleColors + index, replacement.m_styleColors, replacement.m_string.length() * sizeof(*m_styleColors));
	memcpy(m_styleColors + index + replacement.m_string.length(), oldStyleColors + index + len, (m_string.length() - index - replacement.m_string.length()) * sizeof(*m_styleColors));

	delete[] oldStyleFlags;
	delete[] oldStyleColors;

	return *this;
}

SString &
SString::replace(const QString &before, const QString &after, Qt::CaseSensitivity cs)
{
	if(before.length() == 0 && after.length() == 0)
		return *this;

	if(before.length() == 1 && after.length() == 1) {
		// simple path for when there's no need to change the styles flags
		m_string.replace(before, after);
		return *this;
	}

	int oldLength = m_string.length();
	int beforeLength = before.length();
	int afterLength = after.length();

	QList<int> changedData;       // each entry contains the start index of a replaced substring

	for(int offsetIndex = 0, matchedIndex; (matchedIndex = m_string.indexOf(before, offsetIndex, cs)) != -1; offsetIndex = matchedIndex + afterLength) {
		m_string.replace(matchedIndex, beforeLength, after);

		changedData.append(matchedIndex);

		if(!beforeLength)
			matchedIndex++;
	}

	if(changedData.empty()) // nothing was replaced
		return *this;

	if(m_string.length()) {
		int newOffset = 0;
		int oldOffset = 0;
		int unchangedLength;

		char *oldStyleFlags = detachFlags();
		QRgb *oldStyleColors = detachColors();
		setMinFlagsCapacity(m_string.length());

		for(int index = 0; index < changedData.size(); ++index) {
			unchangedLength = changedData[index] - newOffset;

			memcpy(m_styleFlags + newOffset, oldStyleFlags + oldOffset, unchangedLength * sizeof(*m_styleFlags));
			memcpy(m_styleColors + newOffset, oldStyleColors + oldOffset, unchangedLength * sizeof(*m_styleColors));
			newOffset += unchangedLength;
			oldOffset += unchangedLength;

			memset(m_styleFlags + newOffset, oldOffset < oldLength ? oldStyleFlags[oldOffset] : 0, afterLength * sizeof(*m_styleFlags));
			memset_n(m_styleColors + newOffset, oldOffset < oldLength ? oldStyleColors[oldOffset] : 0, afterLength, sizeof(*m_styleColors));
			newOffset += afterLength;
			oldOffset += beforeLength;
		}

		memcpy(m_styleFlags + newOffset, oldStyleFlags + oldOffset, (oldLength - oldOffset) * sizeof(*m_styleFlags));
		memcpy(m_styleColors + newOffset, oldStyleColors + oldOffset, (oldLength - oldOffset) * sizeof(*m_styleColors));

		delete[] oldStyleFlags;
		delete[] oldStyleColors;
	} else {
		setMinFlagsCapacity(m_string.length());
	}

	return *this;
}

SString &
SString::replace(const QString &before, const SString &after, Qt::CaseSensitivity cs)
{
	if(before.length() == 0 && after.m_string.length() == 0)
		return *this;

	int oldLength = m_string.length();
	int beforeLength = before.length();
	int afterLength = after.length();

	QList<int> changedData;       // each entry contains the start index of a replaced substring

	for(int offsetIndex = 0, matchedIndex; (matchedIndex = m_string.indexOf(before, offsetIndex, cs)) != -1; offsetIndex = matchedIndex + afterLength) {
		m_string.replace(matchedIndex, beforeLength, after.m_string);

		changedData.append(matchedIndex);

		if(!beforeLength)
			matchedIndex++;
	}

	if(changedData.empty()) // nothing was replaced
		return *this;

	if(m_string.length()) {
		int newOffset = 0;
		int oldOffset = 0;
		int unchangedLength;

		char *oldStyleFlags = detachFlags();
		QRgb *oldStyleColors = detachColors();
		setMinFlagsCapacity(m_string.length());

		for(int index = 0; index < changedData.size(); ++index) {
			unchangedLength = changedData[index] - newOffset;

			memcpy(m_styleFlags + newOffset, oldStyleFlags + oldOffset, unchangedLength * sizeof(*m_styleFlags));
			memcpy(m_styleColors + newOffset, oldStyleColors + oldOffset, unchangedLength * sizeof(*m_styleColors));
			newOffset += unchangedLength;
			oldOffset += unchangedLength;

			memcpy(m_styleFlags + newOffset, after.m_styleFlags, afterLength * sizeof(*m_styleFlags));
			memcpy(m_styleColors + newOffset, after.m_styleColors, afterLength * sizeof(*m_styleColors));
			newOffset += afterLength;
			oldOffset += beforeLength;
		}

		memcpy(m_styleFlags + newOffset, oldStyleFlags + oldOffset, oldLength - oldOffset * sizeof(*m_styleFlags));
		memcpy(m_styleColors + newOffset, oldStyleColors + oldOffset, (oldLength - oldOffset) * sizeof(*m_styleColors));

		delete[] oldStyleFlags;
		delete[] oldStyleColors;
	} else {
		setMinFlagsCapacity(m_string.length());
	}

	return *this;
}

SString &
SString::replace(QChar before, QChar after, Qt::CaseSensitivity cs)
{
	m_string.replace(before, after, cs);
	return *this;
}

SString &
SString::replace(QChar ch, const QString &after, Qt::CaseSensitivity cs)
{
	if(after.length() == 1) {
		// simple path for when there's no need to change the styles flags
		m_string.replace(ch, after.at(0));
		return *this;
	}

	int oldLength = m_string.length();
	int afterLength = after.length();

	QList<int> changedData;       // each entry contains the start index of a replaced substring

	for(int offsetIndex = 0, matchedIndex; (matchedIndex = m_string.indexOf(ch, offsetIndex, cs)) != -1; offsetIndex = matchedIndex + afterLength) {
		m_string.replace(matchedIndex, 1, after);

		changedData.append(matchedIndex);
	}

	if(changedData.empty()) // nothing was replaced
		return *this;

	if(m_string.length()) {
		int newOffset = 0;
		int oldOffset = 0;
		int unchangedLength;

		char *oldStyleFlags = detachFlags();
		QRgb *oldStyleColors = detachColors();
		setMinFlagsCapacity(m_string.length());

		for(int index = 0; index < changedData.size(); ++index) {
			unchangedLength = changedData[index] - newOffset;

			memcpy(m_styleFlags + newOffset, oldStyleFlags + oldOffset, unchangedLength * sizeof(*m_styleFlags));
			memcpy(m_styleColors + newOffset, oldStyleColors + oldOffset, unchangedLength * sizeof(*m_styleColors));
			newOffset += unchangedLength;
			oldOffset += unchangedLength;

			memset(m_styleFlags + newOffset, oldOffset < oldLength ? oldStyleFlags[oldOffset] : 0, afterLength * sizeof(*m_styleFlags));
			memset_n(m_styleColors + newOffset, oldOffset < oldLength ? oldStyleColors[oldOffset] : 0, afterLength, sizeof(*m_styleColors));
			newOffset += afterLength;
			oldOffset += 1;
		}

		memcpy(m_styleFlags + newOffset, oldStyleFlags + oldOffset, oldLength - oldOffset * sizeof(*m_styleFlags));
		memcpy(m_styleColors + newOffset, oldStyleColors + oldOffset, (oldLength - oldOffset) * sizeof(*m_styleColors));

		delete[] oldStyleFlags;
		delete[] oldStyleColors;
	} else {
		setMinFlagsCapacity(m_string.length());
	}

	return *this;
}

SString &
SString::replace(QChar ch, const SString &after, Qt::CaseSensitivity cs)
{
	int oldLength = m_string.length();
	int afterLength = after.length();

	QList<int> changedData;       // each entry contains the start index of a replaced substring

	for(int offsetIndex = 0, matchedIndex; (matchedIndex = m_string.indexOf(ch, offsetIndex, cs)) != -1; offsetIndex = matchedIndex + afterLength) {
		m_string.replace(matchedIndex, 1, after.m_string);

		changedData.append(matchedIndex);
	}

	if(changedData.empty()) // nothing was replaced
		return *this;

	if(m_string.length()) {
		int newOffset = 0;
		int oldOffset = 0;
		int unchangedLength;

		char *oldStyleFlags = detachFlags();
		QRgb *oldStyleColors = detachColors();
		setMinFlagsCapacity(m_string.length());

		for(int index = 0; index < changedData.size(); ++index) {
			unchangedLength = changedData[index] - newOffset;

			memcpy(m_styleFlags + newOffset, oldStyleFlags + oldOffset, unchangedLength * sizeof(*m_styleFlags));
			memcpy(m_styleColors + newOffset, oldStyleColors + oldOffset, unchangedLength * sizeof(*m_styleColors));
			newOffset += unchangedLength;
			oldOffset += unchangedLength;

			memcpy(m_styleFlags + newOffset, after.m_styleFlags, afterLength * sizeof(*m_styleFlags));
			memcpy(m_styleColors + newOffset, after.m_styleColors, afterLength * sizeof(*m_styleColors));
			newOffset += afterLength;
			oldOffset += 1;
		}

		memcpy(m_styleFlags + newOffset, oldStyleFlags + oldOffset, oldLength - oldOffset * sizeof(*m_styleFlags));
		memcpy(m_styleColors + newOffset, oldStyleColors + oldOffset, (oldLength - oldOffset) * sizeof(*m_styleColors));

		delete[] oldStyleFlags;
		delete[] oldStyleColors;
	} else {
		setMinFlagsCapacity(m_string.length());
	}

	return *this;
}

SString &
SString::replace(const QRegExp &rx, const QString &a)
{
	QRegExp regExp(rx);

	int oldLength = m_string.length();

	QList<int> changedData; // each entry contains the start index of a replaced substring

	QRegExp::CaretMode caretMode = QRegExp::CaretAtZero;
	for(int offsetIndex = 0, matchedIndex; (matchedIndex = regExp.indexIn(m_string, offsetIndex, caretMode)) != -1;) {
		QString after(a);

		bool escaping = false;
		for(int afterIndex = 0, afterSize = after.length(); afterIndex < afterSize; ++afterIndex) {
			QChar chr = after.at(afterIndex);
			if(escaping) {          // perform replace
				escaping = false;
				if(chr.isNumber()) {
					int capNumber = chr.digitValue();
					if(capNumber <= regExp.numCaptures()) {
						QString cap(regExp.cap(capNumber));
						after.replace(afterIndex - 1, 2, cap);
						afterIndex = afterIndex - 1 + cap.length();
						afterSize = after.length();
					}
				}
			} else if(chr == '\\')
				escaping = !escaping;
		}

		if(regExp.matchedLength() == 0 && after.length() == 0)
			continue;

		m_string.replace(matchedIndex, regExp.matchedLength(), after);

		if(regExp.matchedLength() != 1 || after.length() != 1) {
			changedData.append(matchedIndex);
			changedData.append(regExp.matchedLength());     // before length
			changedData.append(after.length());     // after length

			if(!regExp.matchedLength())
				matchedIndex++;
		}

		offsetIndex = matchedIndex + after.length();
		caretMode = QRegExp::CaretWontMatch;    // caret should only be matched the first time
	}

	if(changedData.empty()) // nothing was replaced
		return *this;

	if(m_string.length()) {
		int newOffset = 0;
		int oldOffset = 0;
		int unchangedLength;
		int beforeLength;
		int afterLength;

		char *oldStyleFlags = detachFlags();
		QRgb *oldStyleColors = detachColors();
		setMinFlagsCapacity(m_string.length());

		for(int index = 0; index < changedData.size(); index += 3) {
			unchangedLength = changedData[index] - newOffset;
			beforeLength = changedData[index + 1];
			afterLength = changedData[index + 2];

			memcpy(m_styleFlags + newOffset, oldStyleFlags + oldOffset, unchangedLength * sizeof(*m_styleFlags));
			memcpy(m_styleColors + newOffset, oldStyleColors + oldOffset, unchangedLength * sizeof(*m_styleColors));
			newOffset += unchangedLength;
			oldOffset += unchangedLength;

			memset(m_styleFlags + newOffset, oldOffset < oldLength ? oldStyleFlags[oldOffset] : 0, afterLength * sizeof(*m_styleFlags));
			memset_n(m_styleColors + newOffset, oldOffset < oldLength ? oldStyleColors[oldOffset] : 0, afterLength, sizeof(*m_styleColors));
			newOffset += afterLength;
			oldOffset += beforeLength;
		}

		memcpy(m_styleFlags + newOffset, oldStyleFlags + oldOffset, oldLength - oldOffset * sizeof(*m_styleFlags));
		memcpy(m_styleColors + newOffset, oldStyleColors + oldOffset, (oldLength - oldOffset) * sizeof(*m_styleColors));

		delete[] oldStyleFlags;
		delete[] oldStyleColors;
	} else {
		setMinFlagsCapacity(m_string.length());
	}

	return *this;
}

SString &
SString::replace(const QRegExp &rx, const SString &a)
{
	QRegExp regExp(rx);

	QRegExp::CaretMode caretMode = QRegExp::CaretAtZero;
	for(int offsetIndex = 0, matchedIndex; (matchedIndex = regExp.indexIn(m_string, offsetIndex, caretMode)) != -1;) {
		SString after(a);

		bool escaping = false;
		for(int afterIndex = 0, afterSize = after.length(); afterIndex < afterSize; ++afterIndex) {
			QChar chr = after.at(afterIndex);
			if(escaping) {          // perform replace
				escaping = false;
				if(chr.isNumber()) {
					int capNumber = chr.digitValue();
					if(capNumber <= regExp.numCaptures()) {
						QString cap(regExp.cap(capNumber));
						after.replace(afterIndex - 1, 2, cap);
						afterIndex = afterIndex - 1 + cap.length();
						afterSize = after.length();
					}
				}
			} else if(chr == '\\')
				escaping = !escaping;
		}

		if(regExp.matchedLength() == 0 && after.length() == 0)
			continue;

		replace(matchedIndex, regExp.matchedLength(), after);

		if(!regExp.matchedLength())
			matchedIndex++;

		offsetIndex = matchedIndex + after.length();
		caretMode = QRegExp::CaretWontMatch;    // caret should only be matched the first time
	}

	return *this;
}

SStringList
SString::split(const QString &sep, QString::SplitBehavior behavior, Qt::CaseSensitivity cs) const
{
	SStringList ret;

	if(sep.length()) {
		int offsetIndex = 0;

		for(int matchedIndex; (matchedIndex = m_string.indexOf(sep, offsetIndex, cs)) != -1; offsetIndex = matchedIndex + sep.length()) {
			SString token(m_string.mid(offsetIndex, matchedIndex - offsetIndex));
			if(behavior == QString::KeepEmptyParts || token.length())
				ret << token;
		}
		SString token(m_string.mid(offsetIndex));
		if(behavior == QString::KeepEmptyParts || token.length())
			ret << token;
	} else if(behavior == QString::KeepEmptyParts || length()) {
		ret << *this;
	}

	return ret;
}

SStringList
SString::split(const QChar &sep, QString::SplitBehavior behavior, Qt::CaseSensitivity cs) const
{
	SStringList ret;

	int offsetIndex = 0;

	for(int matchedIndex; (matchedIndex = m_string.indexOf(sep, offsetIndex, cs)) != -1; offsetIndex = matchedIndex + 1) {
		SString token(m_string.mid(offsetIndex, matchedIndex - offsetIndex));
		if(behavior == QString::KeepEmptyParts || token.length())
			ret << token;
	}
	SString token(m_string.mid(offsetIndex));
	if(behavior == QString::KeepEmptyParts || token.length())
		ret << token;

	return ret;
}

SStringList
SString::split(const QRegExp &sep, QString::SplitBehavior behavior) const
{
	SStringList ret;

	QRegExp sepAux(sep);

	int offsetIndex = 0;

	for(int matchedIndex; (matchedIndex = sepAux.indexIn(m_string, offsetIndex)) != -1; offsetIndex = matchedIndex + sepAux.matchedLength()) {
		SString token(m_string.mid(offsetIndex, matchedIndex - offsetIndex));
		if(behavior == QString::KeepEmptyParts || token.length())
			ret << token;
	}
	SString token(m_string.mid(offsetIndex));
	if(behavior == QString::KeepEmptyParts || token.length())
		ret << token;

	return ret;
}

SString
SString::left(int len) const
{
	len = length(0, len);
	SString ret;
	ret.m_string = m_string.left(len);
	ret.setMinFlagsCapacity(len);
	memcpy(ret.m_styleFlags, m_styleFlags, len * sizeof(*m_styleFlags));
	memcpy(ret.m_styleColors, m_styleColors, len * sizeof(*m_styleColors));
	return ret;
}

SString
SString::right(int len) const
{
	len = length(0, len);
	SString ret;
	ret.m_string = m_string.right(len);
	ret.setMinFlagsCapacity(len);
	memcpy(ret.m_styleFlags, m_styleFlags + m_string.length() - len, len * sizeof(*m_styleFlags));
	memcpy(ret.m_styleColors, m_styleColors + m_string.length() - len, len * sizeof(*m_styleColors));
	return ret;
}

SString
SString::mid(int index, int len) const
{
	if(index < 0) {
		if(len >= 0)
			len += index;
		index = 0;
	}

	if(index >= (int)m_string.length())
		return SString();

	len = length(index, len);
	SString ret;
	ret.m_string = m_string.mid(index, len);
	ret.setMinFlagsCapacity(len);
	memcpy(ret.m_styleFlags, m_styleFlags + index, len * sizeof(*m_styleFlags));
	memcpy(ret.m_styleColors, m_styleColors + index, len * sizeof(*m_styleColors));
	return ret;
}

SString
SString::toLower() const
{
	SString ret(*this);
	ret.m_string = m_string.toLower();
	return ret;
}

SString
SString::toUpper() const
{
	SString ret(*this);
	ret.m_string = m_string.toUpper();
	return ret;
}

SString
SString::toTitleCase(bool lowerFirst) const
{
	const QString wordSeparators(" -_([:,;./\\\t\n\"");

	SString ret(*this);

	if(lowerFirst)
		ret.m_string = m_string.toLower();

	bool wordStart = true;
	for(uint idx = 0, size = m_string.length(); idx < size; ++idx) {
		QCharRef chr = ret.m_string[idx];
		if(wordStart) {
			if(!wordSeparators.contains(chr)) {
				wordStart = false;
				chr = chr.toUpper();
			}
		} else if(wordSeparators.contains(chr)) {
			wordStart = true;
		}
	}

	return ret;
}

SString
SString::toSentenceCase(bool lowerFirst, bool *cont) const
{
	const QString sentenceEndChars(".?!");

	SString ret(*this);

	if(lowerFirst)
		ret.m_string = m_string.toLower();

	if(m_string.isEmpty())
		return ret;

	uint prevDots = 0;
	bool startSentence = cont ? !*cont : true;

	for(uint index = 0, size = m_string.length(); index < size; ++index) {
		QCharRef chr = ret.m_string[index];

		if(sentenceEndChars.contains(chr)) {
			if(chr == '.') {
				prevDots++;
				startSentence = prevDots < 3;
			} else {
				prevDots = 0;
				startSentence = true;
			}
		} else {
			if(startSentence && chr.isLetterOrNumber()) {
				chr = chr.toUpper();
				startSentence = false;
			}

			if(!chr.isSpace())
				prevDots = 0;
		}
	}

	if(cont)
		*cont = prevDots != 1 && !startSentence;

	return ret;
}

SString
SString::simplified() const
{
	const QRegExp simplifySpaceRegExp("\\s{2,MAXINT}");

	return trimmed().replace(simplifySpaceRegExp, " ");
}

SString
SString::trimmed() const
{
	const QRegExp trimRegExp("(^\\s+|\\s+$)");

	SString ret(*this);
	return ret.remove(trimRegExp);
}

bool
SString::operator!=(const SString &sstring) const
{
	if(m_string != sstring.m_string)
		return true;

	for(int i = 0, sz = m_string.length(); i < sz; i++) {
		if(m_styleFlags[i] != sstring.m_styleFlags[i])
			return true;
		if((m_styleFlags[i] & Color) != 0 && m_styleColors[i] != sstring.m_styleColors[i])
			return true;
	}

	return false;
}

char *
SString::detachFlags()
{
	char *ret = m_styleFlags;
	m_styleFlags = NULL;
	m_capacity = 0;
	return ret;
}

QRgb *
SString::detachColors()
{
	QRgb *ret = m_styleColors;
	m_styleColors = NULL;
	m_capacity = 0;
	return ret;
}

void
SString::setMinFlagsCapacity(int capacity)
{
	if(capacity > m_capacity) {
		m_capacity = capacity * 2;
		delete[] m_styleFlags;
		m_styleFlags = new char[m_capacity];
		delete[] m_styleColors;
		m_styleColors = new QRgb[m_capacity];
	} else if(capacity == 0) {
		m_capacity = 0;
		delete[] m_styleFlags;
		m_styleFlags = NULL;
		delete[] m_styleColors;
		m_styleColors = NULL;
	} else if(m_capacity > 100 && capacity < m_capacity / 2) {
		m_capacity = m_capacity / 2;
		delete[] m_styleFlags;
		m_styleFlags = new char[m_capacity];
		delete[] m_styleColors;
		m_styleColors = new QRgb[m_capacity];
	}
}

SStringList::SStringList()
{}

SStringList::SStringList(const SString &str)
{
	append(str);
}

SStringList::SStringList(const SStringList &list) :
	QList<SString>(list)
{}

SStringList::SStringList(const QList<SString> &list) :
	QList<SString>(list)
{}

SStringList::SStringList(const QStringList &list)
{
	for(QStringList::ConstIterator it = list.begin(), end = list.end(); it != end; ++it)
		append(*it);
}

SStringList::SStringList(const QList<QString> &list)
{
	for(QList<QString>::ConstIterator it = list.begin(), end = list.end(); it != end; ++it)
		append(*it);
}

SString
SStringList::join(const SString &sep) const
{
	SString ret;

	bool skipSeparator = true;
	for(SStringList::ConstIterator it = begin(), end = this->end(); it != end; ++it) {
		if(skipSeparator) {
			ret += *it;
			skipSeparator = false;
			continue;
		}
		ret += sep;
		ret += *it;
	}

	return ret;
}
