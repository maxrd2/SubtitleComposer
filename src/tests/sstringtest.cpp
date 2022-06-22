/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "sstringtest.h"
#include "core/sstring.h"
#include "helpers/common.h"

#include <QDebug>
#include <QTest>
#include <QRegExp>
#include <QRegularExpression>

using namespace SubtitleComposer;

void
SStringTest::testStyleFlags()
{
	SString sstring("0123456789");
	sstring.setStyleFlagsAt(0, SString::Bold);
	QVERIFY(sstring.styleFlagsAt(0) == SString::Bold);
	QVERIFY(sstring.cummulativeStyleFlags() == SString::Bold);
	QVERIFY(sstring.richString() == QLatin1String("<b>0</b>123456789"));
	sstring.setStyleFlagsAt(2, SString::Italic);
	QVERIFY(sstring.styleFlagsAt(2) == SString::Italic);
	QVERIFY(sstring.cummulativeStyleFlags() == (SString::Bold | SString::Italic));
	QVERIFY(sstring.richString() == QLatin1String("<b>0</b>1<i>2</i>3456789"));
	sstring.setStyleFlagsAt(1, SString::Italic);
	QVERIFY(sstring.styleFlagsAt(1) == SString::Italic);
	QVERIFY(sstring.cummulativeStyleFlags() == (SString::Bold | SString::Italic));
	QVERIFY(sstring.richString() == QLatin1String("<b>0</b><i>12</i>3456789"));
	sstring.setStyleFlagsAt(0, SString::Italic);
	QVERIFY(sstring.styleFlagsAt(0) == SString::Italic);
	QVERIFY(sstring.cummulativeStyleFlags() == SString::Italic);
	QVERIFY(sstring.richString() == QLatin1String("<i>012</i>3456789"));
	sstring.setStyleFlagsAt(9, SString::Underline);
	QVERIFY(sstring.styleFlagsAt(9) == SString::Underline);
	QVERIFY(sstring.cummulativeStyleFlags() == (SString::Italic | SString::Underline));
	QVERIFY(sstring.richString() == QLatin1String("<i>012</i>345678<u>9</u>"));

	sstring.setStyleFlags(0, 15, 0);
	QVERIFY(sstring.richString() == QLatin1String("0123456789"));
	sstring.setStyleFlags(0, 3, SString::Bold);
	QVERIFY(sstring.richString() == QLatin1String("<b>012</b>3456789"));
	sstring.setStyleFlags(3, 3, SString::Italic);
	QVERIFY(sstring.richString() == QLatin1String("<b>012</b><i>345</i>6789"));
	sstring.setStyleFlags(6, 3, SString::Underline);
	QVERIFY(sstring.richString() == QLatin1String("<b>012</b><i>345</i><u>678</u>9"));
	sstring.setStyleFlags(9, 3, SString::StrikeThrough);
	QVERIFY(sstring.richString() == QLatin1String("<b>012</b><i>345</i><u>678</u><s>9</s>"));

	sstring.setStyleFlags(0, 15, SString::Bold | SString::Underline, false);
	QVERIFY(sstring.richString() == QLatin1String("012<i>345</i>678<s>9</s>"));
	sstring.setStyleFlags(0, 15, SString::StrikeThrough, true);
	QVERIFY(sstring.richString() == QLatin1String("<s>012<i>345</i>6789</s>"));
	sstring.setStyleFlags(4, 1, SString::Italic, false);
	QVERIFY(sstring.richString() == QLatin1String("<s>012<i>3</i>4<i>5</i>6789</s>"));
}

void
SStringTest::testLeftMidRight()
{
	SString sstring;
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
SStringTest::testInsert()
{
	SString sstring;

	sstring.append(QString());
	QVERIFY(sstring == SString());

	sstring.append(SString("x", SString::Bold));
	QVERIFY(sstring.richString() == QLatin1String("<b>x</b>"));
	sstring.append($("y"));
	QVERIFY(sstring.richString() == QLatin1String("<b>xy</b>"));
	sstring.append('z');
	QVERIFY(sstring.richString() == QLatin1String("<b>xyz</b>"));

	sstring.prepend(SString("c", SString::Italic));
	QVERIFY(sstring.richString() == QLatin1String("<i>c</i><b>xyz</b>"));
	sstring.prepend($("b"));
	QVERIFY(sstring.richString() == QLatin1String("<i>bc</i><b>xyz</b>"));
	sstring.prepend('a');
	QVERIFY(sstring.richString() == QLatin1String("<i>abc</i><b>xyz</b>"));

	sstring.insert(3, SString("g", SString::Underline));
	QVERIFY(sstring.richString() == QLatin1String("<i>abc</i><u>g</u><b>xyz</b>"));
	sstring.insert(4, $("h"));
	QVERIFY(sstring.richString() == QLatin1String("<i>abc</i><u>gh</u><b>xyz</b>"));
	sstring.insert(5, 'i');
	QVERIFY(sstring.richString() == QLatin1String("<i>abc</i><u>ghi</u><b>xyz</b>"));

	sstring.clear();
	QVERIFY(sstring == SString());
}

void
SStringTest::testReplace()
{
	SString sstring;

	// SString & SString::replace(int index, int len, const QString &replacement)
	sstring = SString("0123456789");
	sstring.replace(-1, 4, $("aaaa"));
	QVERIFY(sstring == SString("0123456789"));
	sstring.replace(-1, 0, $("bbbb"));
	QVERIFY(sstring == SString("0123456789"));
	sstring.replace(15, 0, $("cccc"));
	QVERIFY(sstring == SString("0123456789"));
	sstring.replace(15, 5, $("dddd"));
	QVERIFY(sstring == SString("0123456789"));
	sstring.replace(0, 2, $("eeee"));
	QVERIFY(sstring == SString("eeee23456789"));
	sstring.replace(5, -1, $("dddd"));
	QVERIFY(sstring == SString("eeee2dddd"));

	// SString & SString::replace(int index, int len, const SString &replacement)
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

	// SString & SString::replace(const QString &before, const QString &after, Qt::CaseSensitivity cs=Qt::CaseSensitive)
	sstring = SString("01234567890123456789");
	sstring.replace($("0"), $("A"), Qt::CaseSensitive);
	QVERIFY(sstring == SString("A123456789A123456789"));
	sstring.replace($("23"), $("B"), Qt::CaseSensitive);
	QVERIFY(sstring == SString("A1B456789A1B456789"));
	sstring.replace($("89"), $("C"), Qt::CaseSensitive);
	QVERIFY(sstring == SString("A1B4567CA1B4567C"));
	sstring.replace($("67CA1B"), $("D"), Qt::CaseSensitive);
	QVERIFY(sstring == SString("A1B45D4567C"));
	sstring.replace(QString(), $("E"), Qt::CaseSensitive);
	QVERIFY(sstring == SString("EAE1EBE4E5EDE4E5E6E7ECE"));
	sstring = SString("3");
	sstring.replace(QString(), $("AA"), Qt::CaseSensitive);
	QVERIFY(sstring == SString("AA3AA"));
	sstring = SString();
	sstring.replace(QString(), $("AA"), Qt::CaseSensitive);
	QVERIFY(sstring == SString("AA"));
	sstring.setRichString("c<b>a</b>c");
	sstring.replace($("c"), $("b"));
	QVERIFY(sstring.richString() == QLatin1String("b<b>a</b>b"));

	// SString & SString::replace(const QString &before, const SString &after, Qt::CaseSensitivity cs=Qt::CaseSensitive)
	sstring = SString("01234567890123456789");
	sstring.replace($("0"), SString("A", SString::Bold), Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QLatin1String("<b>A</b>123456789<b>A</b>123456789"));
	sstring.replace($("23"), SString("B", SString::Italic), Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QLatin1String("<b>A</b>1<i>B</i>456789<b>A</b>1<i>B</i>456789"));
	sstring.replace($("89"), SString("C", SString::Underline), Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QLatin1String("<b>A</b>1<i>B</i>4567<u>C</u><b>A</b>1<i>B</i>4567<u>C</u>"));
	sstring.replace($("67CA1B"), SString("D", SString::StrikeThrough), Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QLatin1String("<b>A</b>1<i>B</i>45<s>D</s>4567<u>C</u>"));
	sstring.replace(QString(), SString("E"), Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QLatin1String("E<b>A</b>E1E<i>B</i>E4E5E<s>D</s>E4E5E6E7E<u>C</u>E"));
	sstring = SString("3");
	sstring.replace(QString(), SString("AA", SString::Bold), Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QLatin1String("<b>AA</b>3<b>AA</b>"));
	sstring = SString();
	sstring.replace(QString(), SString("AA", SString::Italic), Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QLatin1String("<i>AA</i>"));

	// SString & SString::replace(const QRegExp &regExp, const QString &replacement)
	sstring = SString("this is a pretty test");
	sstring.replace(QRegExp("^this"), "\\2\\0");
	QVERIFY(sstring.richString() == QLatin1String("\\2this is a pretty test"));
	sstring.replace(QRegExp("[\\w]+"), "w");
	QVERIFY(sstring.richString() == QLatin1String("\\w w w w w"));
	sstring = SString("this is a pretty test");
	sstring.replace(QRegExp("([\\w]*)"), "[\\1]");
	QVERIFY(sstring.richString() == QLatin1String("[this][] [is][] [a][] [pretty][] [test][]"));

	// SString & SString::replace(const QRegExp &regExp, const QString &replacement)
	sstring = SString("this is a pretty test");
	sstring.replace(QRegExp("^this"), "\\2\\0");
	QVERIFY(sstring.richString() == QLatin1String("\\2this is a pretty test"));
	sstring.replace(QRegExp("[\\w]+"), "w");
	QVERIFY(sstring.richString() == QLatin1String("\\w w w w w"));
	sstring = SString("this is a pretty test");
	sstring.replace(QRegExp("([\\w]*)"), "[\\1]");
	QVERIFY(sstring.richString() == QLatin1String("[this][] [is][] [a][] [pretty][] [test][]"));
	sstring.setRichString("<b>a</b>");
	sstring.replace(QRegExp("a$"), "b");
	QVERIFY(sstring.richString() == QLatin1String("<b>b</b>"));

	// SString & SString::replace(const QRegularExpression &regExp, const QString &replacement)
	sstring = SString("this is a pretty test");
	sstring.replace(RE$("^this"), "\\2\\0");
	QVERIFY(sstring.richString() == QLatin1String("\\2this is a pretty test"));
	sstring.replace(RE$("[\\w]+"), "w");
	QVERIFY(sstring.richString() == QLatin1String("\\w w w w w"));
	sstring = SString("this is a pretty test");
	sstring.replace(RE$("([\\w]*)"), "[\\1]");
	QEXPECT_FAIL("", "SString will be rewritten - it's too complex/buggy/slow now.", Continue);
	QVERIFY(sstring.richString() == QLatin1String("[this][] [is][] [a][] [pretty][] [test][]"));

	// SString & SString::replace(const QRegularExpression &regExp, const QString &replacement)
	sstring = SString("this is a pretty test");
	sstring.replace(RE$("^this"), "\\2\\0");
	QVERIFY(sstring.richString() == QLatin1String("\\2this is a pretty test"));
	sstring.replace(RE$("[\\w]+"), "w");
	QVERIFY(sstring.richString() == QLatin1String("\\w w w w w"));
	sstring = SString("this is a pretty test");
	sstring.replace(RE$("([\\w]*)"), "[\\1]");
	QEXPECT_FAIL("", "SString will be rewritten - it's too complex/buggy/slow now.", Continue);
	QVERIFY(sstring.richString() == QLatin1String("[this][] [is][] [a][] [pretty][] [test][]"));
	sstring.setRichString("<b>a</b>");
	sstring.replace(RE$("a$"), "b");
	QVERIFY(sstring.richString() == QLatin1String("<b>b</b>"));

	// SString & SString::replace(QChar before, QChar after, Qt::CaseSensitivity cs=Qt::CaseSensitive)
	sstring.setRichString("<b>This </b>Is <i>a preTtI</i> testi");
	sstring.replace('I', 'X', Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QLatin1String("<b>This </b>Xs <i>a preTtX</i> testi"));
	sstring.replace('T', 'w', Qt::CaseInsensitive);
	QVERIFY(sstring.richString() == QLatin1String("<b>whis </b>Xs <i>a prewwX</i> weswi"));
	sstring.setRichString("c<b>a</b>c");
	sstring.replace('c', 'b');
	QVERIFY(sstring.richString() == QLatin1String("b<b>a</b>b"));

	// SString & SString::replace(QChar ch, const QString &after, Qt::CaseSensitivity cs=Qt::CaseSensitive)
	sstring.setRichString("<b>This </b>Is <i>a preTtI</i> testi");
	sstring.replace('i', $("AA"), Qt::CaseInsensitive);
	QVERIFY(sstring.richString() == QLatin1String("<b>ThAAs </b>AAs <i>a preTtAA</i> testAA"));
	sstring.replace('T', $("XX"), Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QLatin1String("<b>XXhAAs </b>AAs <i>a preXXtAA</i> testAA"));

	// SString & SString::replace(QChar ch, const SString &after, Qt::CaseSensitivity cs=Qt::CaseSensitive)
	sstring.setRichString("<b>This </b>Is <i>a preTtI</i> testi");
	sstring.replace('i', SString("A", SString::Bold));
	QVERIFY(sstring.richString() == QLatin1String("<b>ThAs </b>Is <i>a preTtI</i> test<b>A</b>"));
	sstring.replace('T', SString("X", SString::Underline), Qt::CaseInsensitive);
	QVERIFY(sstring == SString().setRichString("<u>X</u><b>hAs </b>Is <i>a pre</i><u>XX</u><i>I</i> <u>X</u>es<u>X</u><b>A</b>"));

	// SString & SString::remove(int index, int len);
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

	// SString & SString::remove(const QString &str, Qt::CaseSensitivity cs=Qt::CaseSensitive);
	sstring.setRichString("<b>012</b><i>012</i><u>345</u><s>345</s>");
	sstring.remove("013");
	QVERIFY(sstring.richString() == QLatin1String("<b>012</b><i>012</i><u>345</u><s>345</s>"));
	sstring.remove("01");
	QVERIFY(sstring.richString() == QLatin1String("<b>2</b><i>2</i><u>345</u><s>345</s>"));
	sstring.remove("2345");
	QVERIFY(sstring.richString() == QLatin1String("<b>2</b><s>345</s>"));

	// SString & SString::remove(const QRegExp &regExp);
	sstring.setRichString("<b>012</b><i>012</i><u>345</u><s>345</s>");
	sstring.remove(QRegExp("^013"));
	QVERIFY(sstring.richString() == QLatin1String("<b>012</b><i>012</i><u>345</u><s>345</s>"));
	sstring.remove(QRegExp("^012"));
	QVERIFY(sstring.richString() == QLatin1String("<i>012</i><u>345</u><s>345</s>"));
	sstring.remove(QRegExp("345$"));
	QVERIFY(sstring.richString() == QLatin1String("<i>012</i><u>345</u>"));
	sstring.remove(QRegExp("\\d+"));
	QVERIFY(sstring.richString().isEmpty());

	// SString & SString::remove(QChar ch, Qt::CaseSensitivity cs=Qt::CaseSensitive);
	sstring.setRichString("<b>This </b>Is <i>a preTtI</i> testi");
	sstring.remove(QChar('T'), Qt::CaseSensitive);
	QVERIFY(sstring.richString() == QLatin1String("<b>his </b>Is <i>a pretI</i> testi"));
	sstring.remove(QChar('i'), Qt::CaseInsensitive);
	QVERIFY(sstring.richString() == QLatin1String("<b>hs </b>s <i>a pret</i> test"));
}

QTEST_GUILESS_MAIN(SStringTest);
