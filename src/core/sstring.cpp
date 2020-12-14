/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "core/sstring.h"

#include <QList>
#include <QStringList>
#include <QRegularExpression>

#include <QDebug>

#include <QColor>

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
	QString(string),
	m_styleFlags(NULL),
	m_styleColors(NULL),
	m_capacity(0)
{
	if(QString::length()) {
		setMinFlagsCapacity(length());
		memset(m_styleFlags, styleFlags & AllStyles, length() * sizeof(*m_styleFlags));
		memset_n(m_styleColors, styleColor, length(), sizeof(*m_styleColors));
	}
}

SString::SString(const SString &sstring) :
	QString(sstring),
	m_styleFlags(NULL),
	m_styleColors(NULL),
	m_capacity(0)
{
	if(length()) {
		setMinFlagsCapacity(length());
		memcpy(m_styleFlags, sstring.m_styleFlags, length() * sizeof(*m_styleFlags));
		memcpy(m_styleColors, sstring.m_styleColors, length() * sizeof(*m_styleColors));
	}
}

SString &
SString::operator=(const SString &sstring)
{
	if(this == &sstring)
		return *this;

	QString::operator=(sstring);
	setMinFlagsCapacity(length());

	if(length()) {
		memcpy(m_styleFlags, sstring.m_styleFlags, length() * sizeof(*m_styleFlags));
		memcpy(m_styleColors, sstring.m_styleColors, length() * sizeof(*m_styleColors));
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
	QString::operator=(string);
	setMinFlagsCapacity(length());
	if(length()) {
		memset(m_styleFlags, styleFlags & AllStyles, length() * sizeof(*m_styleFlags));
		memset_n(m_styleColors, styleColor, length(), sizeof(*m_styleColors));
	}
}

QString
SString::richString(RichOutputMode mode) const
{
	if(isEmpty())
		return *this;

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

		const int size = length();
		QChar ch;
		for(int index = 1; index < size; ++index) {
			if(m_styleFlags[index] != prevStyleFlags || ((prevStyleFlags & m_styleFlags[index] & Color) && m_styleColors[index] != prevStyleColor)) {
				QString token(QString::mid(prevIndex, index - prevIndex));
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
					ch = at(index);
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
		QString token(QString::mid(prevIndex, length() - prevIndex));
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
		for(uint index = 1, size = length(); index < size; ++index) {
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

				ret += QString::mid(prevIndex, index - prevIndex);

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

		if(prevIndex + 1 < length()) {
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

			ret += QString::mid(prevIndex);

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
	QRegExp tagRegExp("<(/?([bBiIuUsS]|font))[^>]*(\\s+color=\"?([\\w#]+)\"?)?[^>]*>");

	clear();

	int currentStyle = 0;
	QColor currentColor;
	int offsetPos = 0, matchedPos;
	while((matchedPos = tagRegExp.indexIn(string, offsetPos)) != -1) {
		QString matched(tagRegExp.cap(1).toLower());

		int newStyle = currentStyle;
		QColor newColor(currentColor);

		if(matched == QLatin1String("b")) {
			newStyle |= SString::Bold;
		} else if(matched == QLatin1String("i")) {
			newStyle |= SString::Italic;
		} else if(matched == QLatin1String("u")) {
			newStyle |= SString::Underline;
		} else if(matched == QLatin1String("s")) {
			newStyle |= SString::StrikeThrough;
		} else if(matched == QLatin1String("font")) {
			const QString &color = tagRegExp.cap(4);
			if(!color.isEmpty()) {
				newStyle |= SString::Color;
				newColor.setNamedColor(color.toLower());
			}
		} else if(matched == QLatin1String("/b")) {
			newStyle &= ~SString::Bold;
		} else if(matched == QLatin1String("/i")) {
			newStyle &= ~SString::Italic;
		} else if(matched == QLatin1String("/u")) {
			newStyle &= ~SString::Underline;
		} else if(matched == QLatin1String("/s")) {
			newStyle &= ~SString::StrikeThrough;
		} else if(matched == QLatin1String("/font")) {
			newStyle &= ~SString::Color;
			newColor.setNamedColor("-invalid-");
		}

		QString token(string.mid(offsetPos, matchedPos - offsetPos));
		append(SString(token, currentStyle, currentColor.isValid() ? currentColor.rgb() : 0));
		currentStyle = newStyle;
		currentColor = newColor;

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
	for(int index = 0, size = length(); index < size; ++index) {
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
	for(int index = 0, size = length(); index < size; ++index) {
		cummulativeStyleFlags |= m_styleFlags[index];
		if((cummulativeStyleFlags & styleFlags) == styleFlags)
			return true;
	}
	return false;
}

SString &
SString::setStyleFlags(int index, int len, int styleFlags)
{
	if(index < 0 || index >= (int)length())
		return *this;

	for(int index2 = index + length(index, len); index < index2; ++index)
		m_styleFlags[index] = styleFlags;

	return *this;
}

SString &
SString::setStyleFlags(int index, int len, int styleFlags, bool on)
{
	if(index < 0 || index >= (int)length())
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
	if(index < 0 || index >= (int)length())
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
	QString::clear();
	setMinFlagsCapacity(0);
}

SString &
SString::insert(int index, QChar ch)
{
	int oldLength = length();

	if(index <= oldLength && index >= 0) {
		QString::insert(index, ch);

		char *oldStyleFlags = detachFlags();
		QRgb *oldStyleColors = detachColors();
		setMinFlagsCapacity(length());

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
		memcpy(m_styleFlags + index + 1, oldStyleFlags + index, (length() - index - 1) * sizeof(*m_styleFlags));

		memcpy(m_styleColors, oldStyleColors, index * sizeof(*m_styleColors));
		m_styleColors[index] = fillColor;
		memcpy(m_styleColors + index + 1, oldStyleColors + index, (length() - index - 1) * sizeof(*m_styleColors));

		delete[] oldStyleFlags;
		delete[] oldStyleColors;
	}

	return *this;
}

SString &
SString::insert(int index, const QString &str)
{
	int oldLength = length();

	if(str.length() && index <= oldLength && index >= 0) {
		QString::insert(index, str);

		char *oldStyleFlags = detachFlags();
		QRgb *oldStyleColors = detachColors();
		setMinFlagsCapacity(length());

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
		memcpy(m_styleFlags + index + addedLength, oldStyleFlags + index, (length() - index - addedLength) * sizeof(*m_styleFlags));

		memcpy(m_styleColors, oldStyleColors, index * sizeof(*m_styleColors));
		memset_n(m_styleColors + index, fillColor, addedLength, sizeof(*m_styleColors));
		memcpy(m_styleColors + index + addedLength, oldStyleColors + index, (length() - index - addedLength) * sizeof(*m_styleColors));

		delete[] oldStyleFlags;
		delete[] oldStyleColors;
	}

	return *this;
}

SString &
SString::insert(int index, const SString &str)
{
	int oldLength = length();

	if(str.length() && index <= oldLength && index >= 0) {
		QString::insert(index, str);

		char *oldStyleFlags = detachFlags();
		QRgb *oldStyleColors = detachColors();
		setMinFlagsCapacity(length());

		int addedLength = str.length();

		memcpy(m_styleFlags, oldStyleFlags, index * sizeof(*m_styleFlags));
		memcpy(m_styleFlags + index, str.m_styleFlags, addedLength * sizeof(*m_styleFlags));
		memcpy(m_styleFlags + index + addedLength, oldStyleFlags + index, (length() - index - addedLength) * sizeof(*m_styleFlags));

		memcpy(m_styleColors, oldStyleColors, index * sizeof(*m_styleColors));
		memcpy(m_styleColors + index, str.m_styleColors, addedLength * sizeof(*m_styleColors));
		memcpy(m_styleColors + index + addedLength, oldStyleColors + index, (length() - index - addedLength) * sizeof(*m_styleColors));

		delete[] oldStyleFlags;
		delete[] oldStyleColors;
	}

	return *this;
}

SString &
SString::replace(int index, int len, const QString &replacement)
{
	int oldLength = length();

	if(index < 0 || index >= oldLength)
		return *this;

	len = length(index, len);

	if(len == 0 && replacement.length() == 0) // nothing to do (replace nothing with nothing)
		return *this;

	QString::replace(index, len, replacement);

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
		setMinFlagsCapacity(length());

		memcpy(m_styleFlags, oldStyleFlags, index * sizeof(*m_styleFlags));
		memset(m_styleFlags + index, oldStyleFlags[index], replacement.length() * sizeof(*m_styleFlags));
		memcpy(m_styleFlags + index + replacement.length(), oldStyleFlags + index + len, (length() - index - replacement.length()) * sizeof(*m_styleFlags));

		memcpy(m_styleColors, oldStyleColors, index * sizeof(*m_styleColors));
		memset_n(m_styleColors + index, oldStyleColors[index], replacement.length(), sizeof(*m_styleColors));
		memcpy(m_styleColors + index + replacement.length(), oldStyleColors + index + len, (length() - index - replacement.length()) * sizeof(*m_styleColors));

		delete[] oldStyleFlags;
		delete[] oldStyleColors;
	}

	return *this;
}

SString &
SString::replace(int index, int len, const SString &replacement)
{
	int oldLength = length();

	if(index < 0 || index >= oldLength)
		return *this;

	len = length(index, len);

	if(len == 0 && replacement.length() == 0) // nothing to do (replace nothing with nothing)
		return *this;

	QString::replace(index, len, replacement);

	// simple path for when there's no need to change the styles (char substitution)
	// if ( len == 1 && replacement.length() == 1 )
	//  return *this;

	char *oldStyleFlags = detachFlags();
	QRgb *oldStyleColors = detachColors();
	setMinFlagsCapacity(length());

	memcpy(m_styleFlags, oldStyleFlags, index * sizeof(*m_styleFlags));
	memcpy(m_styleFlags + index, replacement.m_styleFlags, replacement.length() * sizeof(*m_styleFlags));
	memcpy(m_styleFlags + index + replacement.length(), oldStyleFlags + index + len, (length() - index - replacement.length()) * sizeof(*m_styleFlags));

	memcpy(m_styleColors, oldStyleColors, index * sizeof(*m_styleColors));
	memcpy(m_styleColors + index, replacement.m_styleColors, replacement.length() * sizeof(*m_styleColors));
	memcpy(m_styleColors + index + replacement.length(), oldStyleColors + index + len, (length() - index - replacement.length()) * sizeof(*m_styleColors));

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
		QString::replace(before, after);
		return *this;
	}

	int oldLength = length();
	int beforeLength = before.length();
	int afterLength = after.length();

	QList<int> changedData;       // each entry contains the start index of a replaced substring

	for(int offsetIndex = 0, matchedIndex; (matchedIndex = indexOf(before, offsetIndex, cs)) != -1; offsetIndex = matchedIndex + afterLength) {
		QString::replace(matchedIndex, beforeLength, after);

		changedData.append(matchedIndex);

		if(!beforeLength)
			matchedIndex++;
	}

	if(changedData.empty()) // nothing was replaced
		return *this;

	if(length()) {
		int newOffset = 0;
		int oldOffset = 0;
		int unchangedLength;

		char *oldStyleFlags = detachFlags();
		QRgb *oldStyleColors = detachColors();
		setMinFlagsCapacity(length());

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
		setMinFlagsCapacity(length());
	}

	return *this;
}

SString &
SString::replace(const QString &before, const SString &after, Qt::CaseSensitivity cs)
{
	if(before.length() == 0 && after.length() == 0)
		return *this;

	int oldLength = length();
	int beforeLength = before.length();
	int afterLength = after.length();

	QList<int> changedData;       // each entry contains the start index of a replaced substring

	for(int offsetIndex = 0, matchedIndex; (matchedIndex = indexOf(before, offsetIndex, cs)) != -1; offsetIndex = matchedIndex + afterLength) {
		QString::replace(matchedIndex, beforeLength, after);

		changedData.append(matchedIndex);

		if(!beforeLength)
			matchedIndex++;
	}

	if(changedData.empty()) // nothing was replaced
		return *this;

	if(length()) {
		int newOffset = 0;
		int oldOffset = 0;
		int unchangedLength;

		char *oldStyleFlags = detachFlags();
		QRgb *oldStyleColors = detachColors();
		setMinFlagsCapacity(length());

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
		setMinFlagsCapacity(length());
	}

	return *this;
}

SString &
SString::replace(QChar before, QChar after, Qt::CaseSensitivity cs)
{
	QString::replace(before, after, cs);
	return *this;
}

SString &
SString::replace(QChar ch, const QString &after, Qt::CaseSensitivity cs)
{
	if(after.length() == 1) {
		// simple path for when there's no need to change the styles flags
		QString::replace(ch, after.at(0));
		return *this;
	}

	int oldLength = length();
	int afterLength = after.length();

	QList<int> changedData;       // each entry contains the start index of a replaced substring

	for(int offsetIndex = 0, matchedIndex; (matchedIndex = indexOf(ch, offsetIndex, cs)) != -1; offsetIndex = matchedIndex + afterLength) {
		QString::replace(matchedIndex, 1, after);

		changedData.append(matchedIndex);
	}

	if(changedData.empty()) // nothing was replaced
		return *this;

	if(length()) {
		int newOffset = 0;
		int oldOffset = 0;
		int unchangedLength;

		char *oldStyleFlags = detachFlags();
		QRgb *oldStyleColors = detachColors();
		setMinFlagsCapacity(length());

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
		setMinFlagsCapacity(length());
	}

	return *this;
}

SString &
SString::replace(QChar ch, const SString &after, Qt::CaseSensitivity cs)
{
	int oldLength = length();
	int afterLength = after.length();

	QList<int> changedData;       // each entry contains the start index of a replaced substring

	for(int offsetIndex = 0, matchedIndex; (matchedIndex = indexOf(ch, offsetIndex, cs)) != -1; offsetIndex = matchedIndex + afterLength) {
		QString::replace(matchedIndex, 1, after);

		changedData.append(matchedIndex);
	}

	if(changedData.empty()) // nothing was replaced
		return *this;

	if(length()) {
		int newOffset = 0;
		int oldOffset = 0;
		int unchangedLength;

		char *oldStyleFlags = detachFlags();
		QRgb *oldStyleColors = detachColors();
		setMinFlagsCapacity(length());

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
		setMinFlagsCapacity(length());
	}

	return *this;
}

SString &
SString::replace(const QRegExp &rx, const QString &a)
{
	QRegExp regExp(rx);

	int oldLength = length();

	QList<int> changedData; // each entry contains the start index of a replaced substring

	QRegExp::CaretMode caretMode = QRegExp::CaretAtZero;
	for(int offsetIndex = 0, matchedIndex; (matchedIndex = regExp.indexIn(*this, offsetIndex, caretMode)) != -1;) {
		QString after(a);

		bool escaping = false;
		for(int afterIndex = 0, afterSize = after.length(); afterIndex < afterSize; ++afterIndex) {
			QChar chr = after.at(afterIndex);
			if(escaping) {          // perform replace
				escaping = false;
				if(chr.isNumber()) {
					int capNumber = chr.digitValue();
					if(capNumber <= regExp.captureCount()) {
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

		QString::replace(matchedIndex, regExp.matchedLength(), after);

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

	if(length()) {
		int newOffset = 0;
		int oldOffset = 0;
		int unchangedLength;
		int beforeLength;
		int afterLength;

		char *oldStyleFlags = detachFlags();
		QRgb *oldStyleColors = detachColors();
		setMinFlagsCapacity(length());

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
		setMinFlagsCapacity(length());
	}

	return *this;
}

SString &
SString::replace(const QRegExp &rx, const SString &a)
{
	QRegExp regExp(rx);

	QRegExp::CaretMode caretMode = QRegExp::CaretAtZero;
	for(int offsetIndex = 0, matchedIndex; (matchedIndex = regExp.indexIn(*this, offsetIndex, caretMode)) != -1;) {
		SString after(a);

		bool escaping = false;
		for(int afterIndex = 0, afterSize = after.length(); afterIndex < afterSize; ++afterIndex) {
			QChar chr = after.at(afterIndex);
			if(escaping) {          // perform replace
				escaping = false;
				if(chr.isNumber()) {
					int capNumber = chr.digitValue();
					if(capNumber <= regExp.captureCount()) {
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

struct SStringCapture {
	int pos;
	int len;
	int no;
};

SString &
SString::replace(const QRegularExpression &regExp, const QString &replacement)
{
	return replace(regExp, SString(replacement), false);
}

SString &
SString::replace(const QRegularExpression &regExp, const SString &replacement, bool styleFromReplacement/*=true*/)
{
	if(!regExp.isValid()) {
		qWarning("SString::replace(): invalid regular expression at character %d:\n\t%s\n\t%s",
				 regExp.patternErrorOffset(),
				 regExp.pattern().toLatin1().constData(),
				 regExp.errorString().toLatin1().constData());
		return *this;
	}

	const QString copy(*this);
	QRegularExpressionMatchIterator iterator = regExp.globalMatch(copy);
	if(!iterator.hasNext())
		return *this;

	int numCaptures = regExp.captureCount();

	// store backreference offsets
	QVector<SStringCapture> backReferences;
	const int repLen = replacement.length();
	const QChar *repChar = replacement.unicode();
	for(int i = 0; i < repLen - 1; i++) {
		if(repChar[i] == QLatin1Char('\\')) {
			int no = repChar[i + 1].digitValue();
			if(no > 0 && no <= numCaptures) {
				SStringCapture backRef;
				backRef.pos = i;
				backRef.len = 2;

				if(i < repLen - 2) {
					int secondDigit = repChar[i + 2].digitValue();
					if(secondDigit != -1 && ((no * 10) + secondDigit) <= numCaptures) {
						no = (no * 10) + secondDigit;
						++backRef.len;
					}
				}

				backRef.no = no;
				backReferences.append(backRef);
			}
		}
	}

	// store data offsets
	int newLength = 0;
	int lastEnd = 0;
	int len;
	QVector<int> chunks; // set of (offset, length) values, even values reference 'copy' string, odd values reference 'replacement' string
	while(iterator.hasNext()) {
		QRegularExpressionMatch match = iterator.next();

		// add the part from 'copy' string before the match
		len = match.capturedStart() - lastEnd;
		Q_ASSERT(len >= 0);
		chunks << lastEnd << len;
		newLength += len;

		lastEnd = 0;
		for(const SStringCapture &backRef: qAsConst(backReferences)) {
			// part of 'replacement' before the backreference
			len = backRef.pos - lastEnd;
			Q_ASSERT(len >= 0);
			chunks << lastEnd << len;
			newLength += len;

			// add the 'copy' string that backreference points to
			len = match.capturedLength(backRef.no);
			Q_ASSERT(len >= 0);
			chunks << match.capturedStart(backRef.no) << len;
			newLength += len;

			lastEnd = backRef.pos + backRef.len;
		}

		// add the last part of the 'replacement' string
		len = replacement.length() - lastEnd;
		Q_ASSERT(len >= 0);
		chunks << lastEnd << len;
		newLength += len;

		lastEnd = match.capturedEnd();
	}

	// add trailing part from 'copy' string after the last match
	len = copy.length() - lastEnd;
	Q_ASSERT(len >= 0);
	chunks << lastEnd << len;
	newLength += len;

	// finally copy the data
	resize(newLength);
	char *oldStyleFlags = detachFlags();
	QRgb *oldStyleColors = detachColors();
	setMinFlagsCapacity(newLength);

	int newOff = 0;
	QChar *newData = data();
	for(int i = 0, n = chunks.size(); i < n; i += 2) {
		// copy data from 'copy' string
		int off = chunks[i];
		int len = chunks[i + 1];
		if(len > 0) {
			memcpy(newData + newOff, copy.midRef(off, len).unicode(), len * sizeof(QChar));
			memcpy(m_styleFlags + newOff, oldStyleFlags + off, len * sizeof(*m_styleFlags));
			memcpy(m_styleColors + newOff, oldStyleColors + off, len * sizeof(*m_styleColors));
			newOff += len;
		}

		i += 2;
		if(i < n) {
			// copy data from 'replacement' string
			int repOff = chunks[i];
			int repLen = chunks[i + 1];
			if(repLen > 0) {
				memcpy(newData + newOff, replacement.midRef(repOff, repLen).unicode(), repLen * sizeof(QChar));
				if(styleFromReplacement) {
					memcpy(m_styleFlags + newOff, replacement.m_styleFlags + repOff, repLen * sizeof(*m_styleFlags));
					memcpy(m_styleColors + newOff, replacement.m_styleColors + repOff, repLen * sizeof(*m_styleColors));
				} else {
					memset(m_styleFlags + newOff, oldStyleFlags[off + len], repLen * sizeof(*m_styleFlags));
					memset_n(m_styleColors + newOff, oldStyleColors[off + len], repLen, sizeof(*m_styleColors));
				}
				newOff += repLen;
			}
		}
	}

	delete[] oldStyleFlags;
	delete[] oldStyleColors;

	return *this;
}

SStringList
SString::split(const QString &sep, QString::SplitBehavior behavior, Qt::CaseSensitivity cs) const
{
	SStringList ret;

	if(sep.length()) {
		int offsetIndex = 0;

		for(int matchedIndex; (matchedIndex = indexOf(sep, offsetIndex, cs)) != -1; offsetIndex = matchedIndex + sep.length()) {
			SString token(QString::mid(offsetIndex, matchedIndex - offsetIndex));
			if(behavior == QString::KeepEmptyParts || token.length())
				ret << token;
		}
		SString token(QString::mid(offsetIndex));
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

	for(int matchedIndex; (matchedIndex = indexOf(sep, offsetIndex, cs)) != -1; offsetIndex = matchedIndex + 1) {
		SString token(QString::mid(offsetIndex, matchedIndex - offsetIndex));
		if(behavior == QString::KeepEmptyParts || token.length())
			ret << token;
	}
	SString token(QString::mid(offsetIndex));
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

	for(int matchedIndex; (matchedIndex = sepAux.indexIn(*this, offsetIndex)) != -1; offsetIndex = matchedIndex + sepAux.matchedLength()) {
		SString token(QString::mid(offsetIndex, matchedIndex - offsetIndex));
		if(behavior == QString::KeepEmptyParts || token.length())
			ret << token;
	}
	SString token(QString::mid(offsetIndex));
	if(behavior == QString::KeepEmptyParts || token.length())
		ret << token;

	return ret;
}

SString
SString::left(int len) const
{
	len = length(0, len);
	SString ret;
	ret.operator=(QString::left(len));
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
	ret.operator=(QString::right(len));
	ret.setMinFlagsCapacity(len);
	memcpy(ret.m_styleFlags, m_styleFlags + length() - len, len * sizeof(*m_styleFlags));
	memcpy(ret.m_styleColors, m_styleColors + length() - len, len * sizeof(*m_styleColors));
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

	if(index >= (int)length())
		return SString();

	len = length(index, len);
	SString ret;
	ret.operator=(QString::mid(index, len));
	ret.setMinFlagsCapacity(len);
	memcpy(ret.m_styleFlags, m_styleFlags + index, len * sizeof(*m_styleFlags));
	memcpy(ret.m_styleColors, m_styleColors + index, len * sizeof(*m_styleColors));
	return ret;
}

SString
SString::toLower() const
{
	SString ret(*this);
	ret.operator=(QString::toLower());
	return ret;
}

SString
SString::toUpper() const
{
	SString ret(*this);
	ret.operator=(QString::toUpper());
	return ret;
}

SString
SString::toTitleCase(bool lowerFirst) const
{
	const QString wordSeparators(QStringLiteral(" -_([:,;./\\\t\n\""));

	SString ret(*this);

	if(lowerFirst)
		ret.operator=(QString::toLower());

	bool wordStart = true;
	for(uint idx = 0, size = length(); idx < size; ++idx) {
		QCharRef chr = ret[idx];
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
		ret.operator=(QString::toLower());

	if(isEmpty())
		return ret;

	uint prevDots = 0;
	bool startSentence = cont ? !*cont : true;

	for(uint index = 0, size = length(); index < size; ++index) {
		QCharRef chr = ret[index];

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

void
SString::simplifyWhiteSpace(QString &text)
{
	int di = 0;
	bool lastWasSpace = true;
	bool lastWasLineFeed = true;
	for(int i = 0, l = text.size(); i < l; i++) {
		const QChar ch = text.at(i);
		if(lastWasSpace && (ch == QChar::Space || ch == QChar::Tabulation)) // skip consecutive spaces
			continue;
		if(lastWasLineFeed && (ch == QChar::LineFeed || ch == QChar::CarriageReturn)) // skip consecutive newlines
			continue;
		if(lastWasSpace && (ch == QChar::LineFeed || ch == QChar::CarriageReturn)) // skip space before newline
			di--;

		if(ch == QChar::Tabulation) // convert tab to space
			text[di] = QChar::Space;
		else if(ch == QChar::CarriageReturn) // convert cr to lf
			text[di] = QChar::LineFeed;
		else if(di != i) // copy other chars
			text[di] = ch;

		lastWasLineFeed = text[di] == QChar::LineFeed;
		lastWasSpace = lastWasLineFeed || text[di] == QChar::Space;

		di++;
	}
	if(lastWasLineFeed)
		di--;
	text.truncate(di);
}

void
SString::simplifyWhiteSpace()
{
	int di = 0;
	bool lastWasSpace = true;
	bool lastWasLineFeed = true;
	for(int i = 0, l = size(); i < l; i++) {
		const QChar ch = at(i);
		if(lastWasSpace && (ch == QChar::Space || ch == QChar::Tabulation)) // skip consecutive spaces
			continue;
		if(lastWasLineFeed && (ch == QChar::LineFeed || ch == QChar::CarriageReturn)) // skip consecutive newlines
			continue;
		if(lastWasSpace && (ch == QChar::LineFeed || ch == QChar::CarriageReturn)) // skip space before newline
			di--;

		if(ch == QChar::Tabulation) // convert tab to space
			operator[](di) = QChar::Space;
		else if(ch == QChar::CarriageReturn) // convert cr to lf
			operator[](di) = QChar::LineFeed;
		else if(di != i) // copy other chars
			operator[](di) = ch;

		if(di != i) {
			m_styleFlags[di] = m_styleFlags[i];
			m_styleColors[di] = m_styleColors[i];
		}

		lastWasLineFeed = at(di) == QChar::LineFeed;
		lastWasSpace = lastWasLineFeed || at(di) == QChar::Space;

		di++;
	}
	if(lastWasLineFeed)
		di--;
	truncate(di);
}

bool
SString::operator!=(const SString &sstring) const
{
	if(!(static_cast<const QString &>(*this) == static_cast<const QString &>(sstring)))
		return true;

	for(int i = 0, sz = length(); i < sz; i++) {
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
	m_styleFlags = nullptr;
	m_capacity = 0;
	return ret;
}

QRgb *
SString::detachColors()
{
	QRgb *ret = m_styleColors;
	m_styleColors = nullptr;
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
		m_styleFlags = nullptr;
		delete[] m_styleColors;
		m_styleColors = nullptr;
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
