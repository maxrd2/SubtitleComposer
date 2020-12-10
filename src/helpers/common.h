#ifndef HELPERS_COMMON_H
#define HELPERS_COMMON_H
/*
 * Copyright (C) 2020 Mladen Milinkovic <max@smoothware.net>
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

#define $(str) QStringLiteral(str)

#define RE$1(regExp) QRegularExpression(QStringLiteral(regExp))
#define RE$2(regExp, opts) QRegularExpression(QStringLiteral(regExp), opts)
#define RE$_VA(_1, _2, NAME, ...) NAME
#define RE$(...) RE$_VA(__VA_ARGS__, RE$2, RE$1)(__VA_ARGS__)

#define staticRE$2(sVar, regExp) const static QRegularExpression sVar(QStringLiteral(regExp))
#define staticRE$3(sVar, regExp, opts) const static QRegularExpression sVar(QStringLiteral(regExp), opts)
#define staticRE$_VA(_1, _2, _3, NAME, ...) NAME
#define staticRE$(...) staticRE$_VA(__VA_ARGS__, staticRE$3, staticRE$2)(__VA_ARGS__)

#define REu QRegularExpression::UseUnicodePropertiesOption
#define REm QRegularExpression::MultilineOption
#define REs QRegularExpression::DotMatchesEverythingOption
#define REi QRegularExpression::CaseInsensitiveOption

#endif // HELPERS_COMMON_H
