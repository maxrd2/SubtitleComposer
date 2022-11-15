/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "richstringtest.h"
#include "core/richstring.h"
#include "helpers/common.h"

#include <QDebug>
#include <QTest>
#include <QRegularExpression>

using namespace SubtitleComposer;

void
RichStringTest::testStyleFlags()
{
	RichString sstring("0123456789");
	sstring.setStyleFlagsAt(0, RichString::Bold);
	QVERIFY(sstring.styleFlagsAt(0) == RichString::Bold);
	QVERIFY(sstring.cummulativeStyleFlags() == RichString::Bold);
	QVERIFY(sstring.richString() == QLatin1String("<b>0</b>123456789"));
	sstring.setStyleFlagsAt(2, RichString::Italic);
	QVERIFY(sstring.styleFlagsAt(2) == RichString::Italic);
	QVERIFY(sstring.cummulativeStyleFlags() == (RichString::Bold | RichString::Italic));
	QVERIFY(sstring.richString() == QLatin1String("<b>0</b>1<i>2</i>3456789"));
	sstring.setStyleFlagsAt(1, RichString::Italic);
	QVERIFY(sstring.styleFlagsAt(1) == RichString::Italic);
	QVERIFY(sstring.cummulativeStyleFlags() == (RichString::Bold | RichString::Italic));
	QVERIFY(sstring.richString() == QLatin1String("<b>0</b><i>12</i>3456789"));
	sstring.setStyleFlagsAt(0, RichString::Italic);
	QVERIFY(sstring.styleFlagsAt(0) == RichString::Italic);
	QVERIFY(sstring.cummulativeStyleFlags() == RichString::Italic);
	QVERIFY(sstring.richString() == QLatin1String("<i>012</i>3456789"));
	sstring.setStyleFlagsAt(9, RichString::Underline);
	QVERIFY(sstring.styleFlagsAt(9) == RichString::Underline);
	QVERIFY(sstring.cummulativeStyleFlags() == (RichString::Italic | RichString::Underline));
	QVERIFY(sstring.richString() == QLatin1String("<i>012</i>345678<u>9</u>"));

	sstring.setStyleFlags(0, 15, 0);
	QVERIFY(sstring.richString() == QLatin1String("0123456789"));
	sstring.setStyleFlags(0, 3, RichString::Bold);
	QVERIFY(sstring.richString() == QLatin1String("<b>012</b>3456789"));
	sstring.setStyleFlags(3, 3, RichString::Italic);
	QVERIFY(sstring.richString() == QLatin1String("<b>012</b><i>345</i>6789"));
	sstring.setStyleFlags(6, 3, RichString::Underline);
	QVERIFY(sstring.richString() == QLatin1String("<b>012</b><i>345</i><u>678</u>9"));
	sstring.setStyleFlags(9, 3, RichString::StrikeThrough);
	QVERIFY(sstring.richString() == QLatin1String("<b>012</b><i>345</i><u>678</u><s>9</s>"));

	sstring.setStyleFlags(0, 15, RichString::Bold | RichString::Underline, false);
	QVERIFY(sstring.richString() == QLatin1String("012<i>345</i>678<s>9</s>"));
	sstring.setStyleFlags(0, 15, RichString::StrikeThrough, true);
	QVERIFY(sstring.richString() == QLatin1String("<s>012<i>345</i>6789</s>"));
	sstring.setStyleFlags(4, 1, RichString::Italic, false);
	QVERIFY(sstring.richString() == QLatin1String("<s>012<i>3</i>4<i>5</i>6789</s>"));
}

void
RichStringTest::testLeftMidRight()
{
	RichString sstring;
	sstring.setRichString("<b>012</b><i>345</i><u>678</u><s>9</s>");

	QVERIFY(sstring.left(-1).richString() == QLatin1String("<b>012</b><i>345</i><u>678</u><s>9</s>"));
	QVERIFY(sstring.left(-5).richString() == QLatin1String("<b>012</b><i>345</i><u>678</u><s>9</s>"));
	QVERIFY(sstring.left(10).richString() == QLatin1String("<b>012</b><i>345</i><u>678</u><s>9</s>"));
	QVERIFY(sstring.left(11).richString() == QLatin1String("<b>012</b><i>345</i><u>678</u><s>9</s>"));
	QVERIFY(sstring.left(4).richString() == QLatin1String("<b>012</b><i>3</i>"));
	QVERIFY(sstring.left(0).richString().isEmpty());

	QVERIFY(sstring.mid(1, -1).richString() == QLatin1String("<b>12</b><i>345</i><u>678</u><s>9</s>"));
	QVERIFY(sstring.mid(1, -5).richString() == QLatin1String("<b>12</b><i>345</i><u>678</u><s>9</s>"));
	QVERIFY(sstring.mid(1, 10).richString() == QLatin1String("<b>12</b><i>345</i><u>678</u><s>9</s>"));
	QVERIFY(sstring.mid(1, 11).richString() == QLatin1String("<b>12</b><i>345</i><u>678</u><s>9</s>"));
	QVERIFY(sstring.mid(1, 4).richString() == QLatin1String("<b>12</b><i>34</i>"));
	QVERIFY(sstring.mid(1, 0).richString().isEmpty());
	QVERIFY(sstring.mid(10, 2).richString().isEmpty());
	QVERIFY(sstring.mid(-2, 4).richString() == QLatin1String("<b>01</b>"));
	QVERIFY(sstring.mid(-2, 11).richString() == QLatin1String("<b>012</b><i>345</i><u>678</u>"));
	QVERIFY(sstring.mid(-2, -1).richString() == QLatin1String("<b>012</b><i>345</i><u>678</u><s>9</s>"));

	QVERIFY(sstring.right(-1).richString() == QLatin1String("<b>012</b><i>345</i><u>678</u><s>9</s>"));
	QVERIFY(sstring.right(-5).richString() == QLatin1String("<b>012</b><i>345</i><u>678</u><s>9</s>"));
	QVERIFY(sstring.right(10).richString() == QLatin1String("<b>012</b><i>345</i><u>678</u><s>9</s>"));
	QVERIFY(sstring.right(11).richString() == QLatin1String("<b>012</b><i>345</i><u>678</u><s>9</s>"));
	QVERIFY(sstring.right(4).richString() == QLatin1String("<u>678</u><s>9</s>"));
	QVERIFY(sstring.right(0).richString().isEmpty());
}

void
RichStringTest::testInsert()
{
	RichString sstring;

	sstring.append(QString());
	QVERIFY(sstring == RichString());

	sstring.append(RichString("x", RichString::Bold));
	QVERIFY(sstring.richString() == QLatin1String("<b>x</b>"));
	sstring.append($("y"));
	QVERIFY(sstring.richString() == QLatin1String("<b>xy</b>"));
	sstring.append('z');
	QVERIFY(sstring.richString() == QLatin1String("<b>xyz</b>"));

	sstring.prepend(RichString("c", RichString::Italic));
	QVERIFY(sstring.richString() == QLatin1String("<i>c</i><b>xyz</b>"));
	sstring.prepend($("b"));
	QVERIFY(sstring.richString() == QLatin1String("<i>bc</i><b>xyz</b>"));
	sstring.prepend('a');
	QVERIFY(sstring.richString() == QLatin1String("<i>abc</i><b>xyz</b>"));

	sstring.insert(3, RichString("g", RichString::Underline));
	QVERIFY(sstring.richString() == QLatin1String("<i>abc</i><u>g</u><b>xyz</b>"));
	sstring.insert(4, $("h"));
	QVERIFY(sstring.richString() == QLatin1String("<i>abc</i><u>gh</u><b>xyz</b>"));
	sstring.insert(5, 'i');
	QVERIFY(sstring.richString() == QLatin1String("<i>abc</i><u>ghi</u><b>xyz</b>"));

	sstring.clear();
	QVERIFY(sstring == RichString());
}

void
RichStringTest::testReplace()
{
	RichString sstring;

	// RichString & RichString::replace(int index, int len, const QString &replacement)
	sstring = RichString("0123456789");
	sstring.replace(-1, 4, $("aaaa"));
	QVERIFY(sstring == RichString("0123456789"));
	sstring.replace(-1, 0, $("bbbb"));
	QVERIFY(sstring == RichString("0123456789"));
	sstring.replace(15, 0, $("cccc"));
	QVERIFY(sstring == RichString("0123456789"));
	sstring.replace(15, 5, $("dddd"));
	QVERIFY(sstring == RichString("0123456789"));
	sstring.replace(0, 2, $("eeee"));
	QVERIFY(sstring == RichString("eeee23456789"));
	sstring.replace(5, -1, $("dddd"));
	QVERIFY(sstring == RichString("eeee2dddd"));

	// RichString & RichString::replace(int index, int len, const RichString &replacement)
	sstring = RichString("0123456789");
	sstring.replace(-1, 4, RichString("aaaa"));
	QVERIFY(sstring == RichString("0123456789"));
	sstring.replace(-1, 0, RichString("bbbb"));
	QVERIFY(sstring == RichString("0123456789"));
	sstring.replace(15, 0, RichString("cccc"));
	QVERIFY(sstring == RichString("0123456789"));
	sstring.replace(15, 5, RichString("dddd"));
	QVERIFY(sstring == RichString("0123456789"));
	sstring.replace(0, 2, RichString("eeee"));
	QVERIFY(sstring == RichString("eeee23456789"));
	sstring.replace(5, -1, RichString("dddd"));
	QVERIFY(sstring == RichString("eeee2dddd"));

	// RichString & RichString::replace(const QString &before, const QString &after, Qt::CaseSensitivity cs=Qt::CaseSensitive)
	sstring = RichString("01234567890123456789");
	sstring.replace($("0"), $("A"), Qt::CaseSensitive);
	QVERIFY(sstring == RichString("A123456789A123456789"));
	sstring.replace($("23"), $("B"), Qt::CaseSensitive);
	QVERIFY(sstring == RichString("A1B456789A1B456789"));
	sstring.replace($("89"), $("C"), Qt::CaseSensitive);
	QVERIFY(sstring == RichString("A1B4567CA1B4567C"));
	sstring.replace($("67CA1B"), $("D"), Qt::CaseSensitive);
	QVERIFY(sstring == RichString("A1B45D4567C"));
	sstring.replace(QString(), $("E"), Qt::CaseSensitive);
	QVERIFY(sstring == RichString("EAE1EBE4E5EDE4E5E6E7ECE"));
	sstring = RichString("3");
	sstring.replace(QString(), $("AA"), Qt::CaseSensitive);
	QVERIFY(sstring == RichString("AA3AA"));
	sstring = RichString();
	sstring.replace(QString(), $("AA"), Qt::CaseSensitive);
	QVERIFY(sstring == RichString("AA"));
	sstring.setRichString("c<b>a</b>c");
	sstring.replace($("c"), $("b"));
	QVERIFY(sstring.richString() == QLatin1String("b<b>a</b>b"));

	// RichString & RichString::replace(const QString &before, const RichString &after, Qt::CaseSensitivity cs=Qt::CaseSensitive)
	sstring = RichString("01234567890123456789");
	sstring.replace($("0"), RichString("A", RichString::Bold), Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QLatin1String("<b>A</b>123456789<b>A</b>123456789"));
	sstring.replace($("23"), RichString("B", RichString::Italic), Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QLatin1String("<b>A</b>1<i>B</i>456789<b>A</b>1<i>B</i>456789"));
	sstring.replace($("89"), RichString("C", RichString::Underline), Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QLatin1String("<b>A</b>1<i>B</i>4567<u>C</u><b>A</b>1<i>B</i>4567<u>C</u>"));
	sstring.replace($("67CA1B"), RichString("D", RichString::StrikeThrough), Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QLatin1String("<b>A</b>1<i>B</i>45<s>D</s>4567<u>C</u>"));
	sstring.replace(QString(), RichString("E"), Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QLatin1String("E<b>A</b>E1E<i>B</i>E4E5E<s>D</s>E4E5E6E7E<u>C</u>E"));
	sstring = RichString("3");
	sstring.replace(QString(), RichString("AA", RichString::Bold), Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QLatin1String("<b>AA</b>3<b>AA</b>"));
	sstring = RichString();
	sstring.replace(QString(), RichString("AA", RichString::Italic), Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QLatin1String("<i>AA</i>"));

	// RichString & RichString::replace(const QRegularExpression &regExp, const QString &replacement)
	sstring = RichString("this is a pretty test");
	sstring.replace(RE$("^this"), "\\2\\0");
	QVERIFY(sstring.richString() == QLatin1String("\\2this is a pretty test"));
	sstring.replace(RE$("[\\w]+"), "w");
	QVERIFY(sstring.richString() == QLatin1String("\\w w w w w"));
	sstring = RichString("this is a pretty test");
	sstring.replace(RE$("([\\w]*)"), "[\\1]");
	QVERIFY(sstring.richString() == QLatin1String("[this][] [is][] [a][] [pretty][] [test][]"));

	// RichString & RichString::replace(const QRegularExpression &regExp, const QString &replacement)
	sstring = RichString("this is a pretty test");
	sstring.replace(RE$("^this"), "\\2\\0");
	QVERIFY(sstring.richString() == QLatin1String("\\2this is a pretty test"));
	sstring.replace(RE$("[\\w]+"), "w");
	QVERIFY(sstring.richString() == QLatin1String("\\w w w w w"));
	sstring = RichString("this is a pretty test");
	sstring.replace(RE$("([\\w]*)"), "[\\1]");
	QVERIFY(sstring.richString() == QLatin1String("[this][] [is][] [a][] [pretty][] [test][]"));
	sstring.setRichString("<b>a</b>");
	sstring.replace(RE$("a$"), "b");
	QVERIFY(sstring.richString() == QLatin1String("<b>b</b>"));

	// RichString & RichString::replace(QChar before, QChar after, Qt::CaseSensitivity cs=Qt::CaseSensitive)
	sstring.setRichString("<b>This </b>Is <i>a preTtI</i> testi");
	sstring.replace('I', 'X', Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QLatin1String("<b>This</b> Xs <i>a preTtX</i> testi"));
	sstring.replace('T', 'w', Qt::CaseInsensitive);
	QVERIFY(sstring.richString() == QLatin1String("<b>whis</b> Xs <i>a prewwX</i> weswi"));
	sstring.setRichString("c<b>a</b>c");
	sstring.replace('c', 'b');
	QVERIFY(sstring.richString() == QLatin1String("b<b>a</b>b"));

	// RichString & RichString::replace(QChar ch, const QString &after, Qt::CaseSensitivity cs=Qt::CaseSensitive)
	sstring.setRichString("<b>This </b>Is <i>a preTtI</i> testi");
	sstring.replace('i', $("AA"), Qt::CaseInsensitive);
	QVERIFY(sstring.richString() == QLatin1String("<b>ThAAs</b> AAs <i>a preTtAA</i> testAA"));
	sstring.replace('T', $("XX"), Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QLatin1String("<b>XXhAAs</b> AAs <i>a preXXtAA</i> testAA"));

	// RichString & RichString::replace(QChar ch, const RichString &after, Qt::CaseSensitivity cs=Qt::CaseSensitive)
	sstring.setRichString("<b>This </b>Is <i>a preTtI</i> testi");
	sstring.replace('i', RichString("A", RichString::Bold));
	QVERIFY(sstring.richString() == QLatin1String("<b>ThAs</b> Is <i>a preTtI</i> test<b>A</b>"));
	sstring.replace('T', RichString("X", RichString::Underline), Qt::CaseInsensitive);
	QVERIFY(sstring == RichString().setRichString("<u>X</u><b>hAs </b>Is <i>a pre</i><u>XX</u><i>I</i> <u>X</u>es<u>X</u><b>A</b>"));

	// RichString & RichString::remove(int index, int len);
	sstring.setRichString("<b>012</b><i>345</i><u>678</u><s>9</s>");
	sstring.remove(0, 0);
	QVERIFY(sstring.richString() == QLatin1String("<b>012</b><i>345</i><u>678</u><s>9</s>"));
	sstring.remove(0, 2);
	QVERIFY(sstring.richString() == QLatin1String("<b>2</b><i>345</i><u>678</u><s>9</s>"));
	sstring.remove(10, 2);
	QVERIFY(sstring.richString() == QLatin1String("<b>2</b><i>345</i><u>678</u><s>9</s>"));
	sstring.remove(8, 5);
	QVERIFY(sstring.richString() == QLatin1String("<b>2</b><i>345</i><u>678</u><s>9</s>"));
	sstring.remove(7, 3);
	QVERIFY(sstring.richString() == QLatin1String("<b>2</b><i>345</i><u>678</u>"));
	sstring.remove(0, 3);
	QVERIFY(sstring.richString() == QLatin1String("<i>5</i><u>678</u>"));
	sstring.remove(0, -1);
	QVERIFY(sstring.richString().isEmpty());

	// RichString & RichString::remove(const QString &str, Qt::CaseSensitivity cs=Qt::CaseSensitive);
	sstring.setRichString("<b>012</b><i>012</i><u>345</u><s>345</s>");
	sstring.remove("013");
	QVERIFY(sstring.richString() == QLatin1String("<b>012</b><i>012</i><u>345</u><s>345</s>"));
	sstring.remove("01");
	QVERIFY(sstring.richString() == QLatin1String("<b>2</b><i>2</i><u>345</u><s>345</s>"));
	sstring.remove("2345");
	QVERIFY(sstring.richString() == QLatin1String("<b>2</b><s>345</s>"));

	// RichString & RichString::remove(const QRegularExpression &regExp);
	sstring.setRichString("<b>012</b><i>012</i><u>345</u><s>345</s>");
	sstring.remove(RE$("^013"));
	QVERIFY(sstring.richString() == QLatin1String("<b>012</b><i>012</i><u>345</u><s>345</s>"));
	sstring.remove(RE$("^012"));
	QVERIFY(sstring.richString() == QLatin1String("<i>012</i><u>345</u><s>345</s>"));
	sstring.remove(RE$("345$"));
	QVERIFY(sstring.richString() == QLatin1String("<i>012</i><u>345</u>"));
	sstring.remove(RE$("\\d+"));
	QVERIFY(sstring.richString().isEmpty());

	// RichString & RichString::remove(QChar ch, Qt::CaseSensitivity cs=Qt::CaseSensitive);
	sstring.setRichString("<b>This </b>Is <i>a preTtI</i> testi");
	sstring.remove(QChar('T'), Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QLatin1String("<b>his</b> Is <i>a pretI</i> testi"));
	sstring.remove(QChar('i'), Qt::CaseInsensitive);
	QVERIFY(sstring.richString() == QLatin1String("<b>hs</b> s <i>a pret</i> test"));

	// RichString & RichString::richString();
	sstring.setRichString("<b>This </b><i>  is a space testI</i>");
	QVERIFY(sstring.richString() == QLatin1String("<b>This</b>   <i>is a space testI</i>"));

	// RichString & Voice/Class
	sstring.setRichString("<v Person A>Hi <b>Person B</b>! How are you doing?\n<v Person B><c.test>Hi there.</c.test> Doing great!");
	sstring.replace($("are you"), $("is you"));
	QVERIFY(sstring.richString() == QLatin1String("<v Person A>Hi <b>Person B</b>! How is you doing?\n<v Person B><c.test>Hi there.</c.test> Doing great!"));
}

QTEST_GUILESS_MAIN(RichStringTest);
