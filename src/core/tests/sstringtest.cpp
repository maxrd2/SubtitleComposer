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

#include "sstringtest.h"
#include "../sstring.h"

#include <QtTest>                               // krazy:exclude=c++/includes
#include <QtCore>                               // krazy:exclude=c++/includes

#include <KDebug>

using namespace SubtitleComposer;

void
SStringTest::testStyleFlags()
{
	SString sstring("0123456789");
	sstring.setStyleFlagsAt(0, SString::Bold);
	QVERIFY(sstring.styleFlagsAt(0) == SString::Bold);
	QVERIFY(sstring.cummulativeStyleFlags() == SString::Bold);
	QVERIFY(sstring.richString() == QString("<b>0</b>123456789"));
	sstring.setStyleFlagsAt(2, SString::Italic);
	QVERIFY(sstring.styleFlagsAt(2) == SString::Italic);
	QVERIFY(sstring.cummulativeStyleFlags() == (SString::Bold | SString::Italic));
	QVERIFY(sstring.richString() == QString("<b>0</b>1<i>2</i>3456789"));
	sstring.setStyleFlagsAt(1, SString::Italic);
	QVERIFY(sstring.styleFlagsAt(1) == SString::Italic);
	QVERIFY(sstring.cummulativeStyleFlags() == (SString::Bold | SString::Italic));
	QVERIFY(sstring.richString() == QString("<b>0</b><i>12</i>3456789"));
	sstring.setStyleFlagsAt(0, SString::Italic);
	QVERIFY(sstring.styleFlagsAt(0) == SString::Italic);
	QVERIFY(sstring.cummulativeStyleFlags() == SString::Italic);
	QVERIFY(sstring.richString() == QString("<i>012</i>3456789"));
	sstring.setStyleFlagsAt(9, SString::Underline);
	QVERIFY(sstring.styleFlagsAt(9) == SString::Underline);
	QVERIFY(sstring.cummulativeStyleFlags() == (SString::Italic | SString::Underline));
	QVERIFY(sstring.richString() == QString("<i>012</i>345678<u>9</u>"));

	sstring.setStyleFlags(0, 15, 0);
	QVERIFY(sstring.richString() == QString("0123456789"));
	sstring.setStyleFlags(0, 3, SString::Bold);
	QVERIFY(sstring.richString() == QString("<b>012</b>3456789"));
	sstring.setStyleFlags(3, 3, SString::Italic);
	QVERIFY(sstring.richString() == QString("<b>012</b><i>345</i>6789"));
	sstring.setStyleFlags(6, 3, SString::Underline);
	QVERIFY(sstring.richString() == QString("<b>012</b><i>345</i><u>678</u>9"));
	sstring.setStyleFlags(9, 3, SString::StrikeThrough);
	QVERIFY(sstring.richString() == QString("<b>012</b><i>345</i><u>678</u><s>9</s>"));

	sstring.setStyleFlags(0, 15, SString::Bold | SString::Underline, false);
	QVERIFY(sstring.richString() == QString("012<i>345</i>678<s>9</s>"));
	sstring.setStyleFlags(0, 15, SString::StrikeThrough, true);
	QVERIFY(sstring.richString() == QString("<s>012<i>345</i>6789</s>"));
	sstring.setStyleFlags(4, 1, SString::Italic, false);
	QVERIFY(sstring.richString() == QString("<s>012<i>3</i>4<i>5</i>6789</s>"));
}

void
SStringTest::testLeftMidRight()
{
	SString sstring;
	sstring.setRichString("<b>012</b><i>345</i><u>678</u><s>9</s>");

	QVERIFY(sstring.left(-1).richString() == QString("<b>012</b><i>345</i><u>678</u><s>9</s>"));
	QVERIFY(sstring.left(-5).richString() == QString("<b>012</b><i>345</i><u>678</u><s>9</s>"));
	QVERIFY(sstring.left(10).richString() == QString("<b>012</b><i>345</i><u>678</u><s>9</s>"));
	QVERIFY(sstring.left(11).richString() == QString("<b>012</b><i>345</i><u>678</u><s>9</s>"));
	QVERIFY(sstring.left(4).richString() == QString("<b>012</b><i>3</i>"));
	QVERIFY(sstring.left(0).richString() == QString(""));

	QVERIFY(sstring.mid(1, -1).richString() == QString("<b>12</b><i>345</i><u>678</u><s>9</s>"));
	QVERIFY(sstring.mid(1, -5).richString() == QString("<b>12</b><i>345</i><u>678</u><s>9</s>"));
	QVERIFY(sstring.mid(1, 10).richString() == QString("<b>12</b><i>345</i><u>678</u><s>9</s>"));
	QVERIFY(sstring.mid(1, 11).richString() == QString("<b>12</b><i>345</i><u>678</u><s>9</s>"));
	QVERIFY(sstring.mid(1, 4).richString() == QString("<b>12</b><i>34</i>"));
	QVERIFY(sstring.mid(1, 0).richString() == QString(""));
	QVERIFY(sstring.mid(10, 2).richString() == QString(""));
	QVERIFY(sstring.mid(-2, 4).richString() == QString("<b>01</b>"));
	QVERIFY(sstring.mid(-2, 11).richString() == QString("<b>012</b><i>345</i><u>678</u>"));
	QVERIFY(sstring.mid(-2, -1).richString() == QString("<b>012</b><i>345</i><u>678</u><s>9</s>"));

	QVERIFY(sstring.right(-1).richString() == QString("<b>012</b><i>345</i><u>678</u><s>9</s>"));
	QVERIFY(sstring.right(-5).richString() == QString("<b>012</b><i>345</i><u>678</u><s>9</s>"));
	QVERIFY(sstring.right(10).richString() == QString("<b>012</b><i>345</i><u>678</u><s>9</s>"));
	QVERIFY(sstring.right(11).richString() == QString("<b>012</b><i>345</i><u>678</u><s>9</s>"));
	QVERIFY(sstring.right(4).richString() == QString("<u>678</u><s>9</s>"));
	QVERIFY(sstring.right(0).richString() == QString(""));
}

void
SStringTest::testInsert()
{
	SString sstring;

	sstring.append("");
	QVERIFY(sstring == SString());

	sstring.append(SString("x", SString::Bold));
	QVERIFY(sstring.richString() == QString("<b>x</b>"));
	sstring.append(QString("y"));
	QVERIFY(sstring.richString() == QString("<b>xy</b>"));
	sstring.append('z');
	QVERIFY(sstring.richString() == QString("<b>xyz</b>"));

	sstring.prepend(SString("c", SString::Italic));
	QVERIFY(sstring.richString() == QString("<i>c</i><b>xyz</b>"));
	sstring.prepend(QString("b"));
	QVERIFY(sstring.richString() == QString("<i>bc</i><b>xyz</b>"));
	sstring.prepend('a');
	QVERIFY(sstring.richString() == QString("<i>abc</i><b>xyz</b>"));

	sstring.insert(3, SString("g", SString::Underline));
	QVERIFY(sstring.richString() == QString("<i>abc</i><u>g</u><b>xyz</b>"));
	sstring.insert(4, QString("h"));
	QVERIFY(sstring.richString() == QString("<i>abc</i><u>gh</u><b>xyz</b>"));
	sstring.insert(5, 'i');
	QVERIFY(sstring.richString() == QString("<i>abc</i><u>ghi</u><b>xyz</b>"));

	sstring.clear();
	QVERIFY(sstring == SString());
}

void
SStringTest::testReplace()
{
	SString sstring;

	// SString& replace( int index, int len, const QString& replacement )
	sstring = SString("0123456789");
	sstring.replace(-1, 4, QString("aaaa"));
	QVERIFY(sstring == SString("0123456789"));
	sstring.replace(-1, 0, QString("bbbb"));
	QVERIFY(sstring == SString("0123456789"));
	sstring.replace(15, 0, QString("cccc"));
	QVERIFY(sstring == SString("0123456789"));
	sstring.replace(15, 5, QString("dddd"));
	QVERIFY(sstring == SString("0123456789"));
	sstring.replace(0, 2, QString("eeee"));
	QVERIFY(sstring == SString("eeee23456789"));
	sstring.replace(5, -1, QString("dddd"));
	QVERIFY(sstring == SString("eeee2dddd"));

	// SString& replace( int index, int len, const SString& replacement )
	sstring = SString("0123456789");
	sstring.replace(-1, 4, SString("aaaa"));
	QVERIFY(sstring == SString("0123456789"));
	sstring.replace(-1, 0, SString("bbbb"));
	QVERIFY(sstring == SString("0123456789"));
	sstring.replace(15, 0, SString("cccc"));
	QVERIFY(sstring == SString("0123456789"));
	sstring.replace(15, 5, SString("dddd"));
	QVERIFY(sstring == SString("0123456789"));
	sstring.replace(0, 2, SString("eeee"));
	QVERIFY(sstring == SString("eeee23456789"));
	sstring.replace(5, -1, SString("dddd"));
	QVERIFY(sstring == SString("eeee2dddd"));

	// SString& replace( const QString& before, const QString& after, Qt::CaseSensitivity cs=Qt::CaseSensitive )
	sstring = SString("01234567890123456789");
	sstring.replace(QString("0"), QString("A"), Qt::CaseSensitive);
	QVERIFY(sstring == SString("A123456789A123456789"));
	sstring.replace(QString("23"), QString("B"), Qt::CaseSensitive);
	QVERIFY(sstring == SString("A1B456789A1B456789"));
	sstring.replace(QString("89"), QString("C"), Qt::CaseSensitive);
	QVERIFY(sstring == SString("A1B4567CA1B4567C"));
	sstring.replace(QString("67CA1B"), QString("D"), Qt::CaseSensitive);
	QVERIFY(sstring == SString("A1B45D4567C"));
	sstring.replace(QString(""), QString("E"), Qt::CaseSensitive);
	QVERIFY(sstring == SString("EAE1EBE4E5EDE4E5E6E7ECE"));
	sstring = SString("3");
	sstring.replace(QString(""), QString("AA"), Qt::CaseSensitive);
	QVERIFY(sstring == SString("AA3AA"));
	sstring = SString();
	sstring.replace(QString(""), QString("AA"), Qt::CaseSensitive);
	QVERIFY(sstring == SString("AA"));

	// SString& replace( const QString& before, const SString& after, Qt::CaseSensitivity cs=Qt::CaseSensitive )
	sstring = SString("01234567890123456789");
	sstring.replace(QString("0"), SString("A", SString::Bold), Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QString("<b>A</b>123456789<b>A</b>123456789"));
	sstring.replace(QString("23"), SString("B", SString::Italic), Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QString("<b>A</b>1<i>B</i>456789<b>A</b>1<i>B</i>456789"));
	sstring.replace(QString("89"), SString("C", SString::Underline), Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QString("<b>A</b>1<i>B</i>4567<u>C</u><b>A</b>1<i>B</i>4567<u>C</u>"));
	sstring.replace(QString("67CA1B"), SString("D", SString::StrikeThrough), Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QString("<b>A</b>1<i>B</i>45<s>D</s>4567<u>C</u>"));
	sstring.replace(QString(""), SString("E"), Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QString("E<b>A</b>E1E<i>B</i>E4E5E<s>D</s>E4E5E6E7E<u>C</u>E"));
	sstring = SString("3");
	sstring.replace(QString(""), SString("AA", SString::Bold), Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QString("<b>AA</b>3<b>AA</b>"));
	sstring = SString();
	sstring.replace(QString(""), SString("AA", SString::Italic), Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QString("<i>AA</i>"));

	// SString& replace( const QRegExp& rx, const QString& after )
	sstring = SString("this is a pretty test");
	sstring.replace(QRegExp("^this"), "\\2\\0");
	QVERIFY(sstring.richString() == QString("\\2this is a pretty test"));
	sstring.replace(QRegExp("[\\w]+"), "w");
	QVERIFY(sstring.richString() == QString("\\w w w w w"));
	sstring = SString("this is a pretty test");
	sstring.replace(QRegExp("([\\w]*)"), "[\\1]");
	QVERIFY(sstring.richString() == QString("[this][] [is][] [a][] [pretty][] [test][]"));

	// SString& SString::replace( QChar before, QChar after, Qt::CaseSensitivity cs=Qt::CaseSensitive )
	sstring.setRichString("<b>This </b>Is <i>a preTtI</i> testi");
	sstring.replace('I', 'X', Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QString("<b>This </b>Xs <i>a preTtX</i> testi"));
	sstring.replace('T', 'w', Qt::CaseInsensitive);
	QVERIFY(sstring.richString() == QString("<b>whis </b>Xs <i>a prewwX</i> weswi"));

	// SString& SString::replace( QChar ch, const QString& after, Qt::CaseSensitivity cs=Qt::CaseSensitive )
	sstring.setRichString("<b>This </b>Is <i>a preTtI</i> testi");
	sstring.replace('i', QString("AA"), Qt::CaseInsensitive);
	QVERIFY(sstring.richString() == QString("<b>ThAAs </b>AAs <i>a preTtAA</i> testAA"));
	sstring.replace('T', QString("XX"), Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QString("<b>XXhAAs </b>AAs <i>a preXXtAA</i> testAA"));

	// SString& SString::replace( QChar ch, const SString& after, Qt::CaseSensitivity cs=Qt::CaseSensitive )
	sstring.setRichString("<b>This </b>Is <i>a preTtI</i> testi");
	sstring.replace('i', SString("A", SString::Bold));
	QVERIFY(sstring.richString() == QString("<b>ThAs </b>Is <i>a preTtI</i> test<b>A</b>"));
	sstring.replace('T', SString("X", SString::Underline), Qt::CaseInsensitive);
	QVERIFY(sstring == SString().setRichString("<u>X</u><b>hAs </b>Is <i>a pre</i><u>XX</u><i>I</i> <u>X</u>es<u>X</u><b>A</b>"));

	// SString& remove( int index, int len );
	sstring.setRichString("<b>012</b><i>345</i><u>678</u><s>9</s>");
	sstring.remove(0, 0);
	QVERIFY(sstring.richString() == QString("<b>012</b><i>345</i><u>678</u><s>9</s>"));
	sstring.remove(0, 2);
	QVERIFY(sstring.richString() == QString("<b>2</b><i>345</i><u>678</u><s>9</s>"));
	sstring.remove(10, 2);
	QVERIFY(sstring.richString() == QString("<b>2</b><i>345</i><u>678</u><s>9</s>"));
	sstring.remove(8, 5);
	QVERIFY(sstring.richString() == QString("<b>2</b><i>345</i><u>678</u><s>9</s>"));
	sstring.remove(7, 3);
	QVERIFY(sstring.richString() == QString("<b>2</b><i>345</i><u>678</u>"));
	sstring.remove(0, 3);
	QVERIFY(sstring.richString() == QString("<i>5</i><u>678</u>"));
	sstring.remove(0, -1);
	QVERIFY(sstring.richString() == QString(""));

	// SString& remove( const QString& str, Qt::CaseSensitivity cs=Qt::CaseSensitive );
	sstring.setRichString("<b>012</b><i>012</i><u>345</u><s>345</s>");
	sstring.remove("013");
	QVERIFY(sstring.richString() == QString("<b>012</b><i>012</i><u>345</u><s>345</s>"));
	sstring.remove("01");
	QVERIFY(sstring.richString() == QString("<b>2</b><i>2</i><u>345</u><s>345</s>"));
	sstring.remove("2345");
	QVERIFY(sstring.richString() == QString("<b>2</b><s>345</s>"));

	// SString& remove( const QRegExp& rx );
	sstring.setRichString("<b>012</b><i>012</i><u>345</u><s>345</s>");
	sstring.remove(QRegExp("^013"));
	QVERIFY(sstring.richString() == QString("<b>012</b><i>012</i><u>345</u><s>345</s>"));
	sstring.remove(QRegExp("^012"));
	QVERIFY(sstring.richString() == QString("<i>012</i><u>345</u><s>345</s>"));
	sstring.remove(QRegExp("345$"));
	QVERIFY(sstring.richString() == QString("<i>012</i><u>345</u>"));
	sstring.remove(QRegExp("\\d+"));
	QVERIFY(sstring.richString() == QString(""));

	// SString& remove( QChar ch, Qt::CaseSensitivity cs=Qt::CaseSensitive );
	sstring.setRichString("<b>This </b>Is <i>a preTtI</i> testi");
	sstring.remove(QChar('T'), Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QString("<b>his </b>Is <i>a pretI</i> testi"));
	sstring.remove(QChar('i'), Qt::CaseInsensitive);
	QVERIFY(sstring.richString() == QString("<b>hs </b>s <i>a pret</i> test"));

	sstring.setRichString("<b>a</b>");
	sstring.replace(QRegExp("a$"), "b");
	QVERIFY(sstring.richString() == QString("<b>b</b>"));

	sstring.setRichString("c<b>a</b>c");
	sstring.replace("c", "b");      // krazy:exclude=c++/doublequote_chars
	QVERIFY(sstring.richString() == QString("b<b>a</b>b"));

	sstring.setRichString("c<b>a</b>c");
	sstring.replace('c', 'b');
	QVERIFY(sstring.richString() == QString("b<b>a</b>b"));
}

QTEST_MAIN(SStringTest);

#include "sstringtest.moc"
