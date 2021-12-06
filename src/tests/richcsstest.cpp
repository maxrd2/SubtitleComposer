/*
    SPDX-FileCopyrightText: 2021-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#define private public // TODO: FIXME: compare selectors with final stylesheet
#include "richcsstest.h"
#undef private

#include "helpers/common.h"

#include <QDebug>
#include <QTest>

using namespace SubtitleComposer;

void
RichCssTest::testParseSelector_data()
{
	QTest::addColumn<QString>("selIn");
	QTest::addColumn<QString>("selOut");

	QTest::newRow("spaces around")
			<< "\t tag.class1.class2 "
			<< "tag.class1.class2";
	QTest::newRow("spaces inside")
			<< "tag\t.class1  \t .class2   .class3 .class4"
			<< "tag .class1 .class2 .class3 .class4";
	QTest::newRow("attribute selector")
			<< "tag[attr = value]"
			<< "tag[attr=value]";
	QTest::newRow("attribute selector quoted")
			<< "tag[attr   =   \"value with spaces\"]"
			<< "tag[attr=\"value with spaces\"]";
	QTest::newRow("selector with symbols 1")
			<< "tag::first-child"
			<< "tag::first-child";
	QTest::newRow("selector with symbols 2")
			<< "tag::nth-child(2n + 1)"
			<< "tag::nth-child(2n+1)";
	QTest::newRow("child selector")
			<< "tag1   >   tag2"
			<< "tag1>tag2";
	QTest::newRow("immediate sibling selector")
			<< "tag1  +  tag2"
			<< "tag1+tag2";
	QTest::newRow("sibling selector")
			<< "tag1 ~ tag2"
			<< "tag1~tag2";
	QTest::newRow("descendant selector")
			<< "tag1   tag2"
			<< "tag1 tag2";
	QTest::newRow("unicode escape 1")
			<< "\\31 23"
			<< "123";
	QTest::newRow("unicode escape 2")
			<< "\\20AC ab"
			<< "€ab";
	QTest::newRow("unicode escape 3")
			<< "\\20AC  ab"
			<< "€ ab";
	QTest::newRow("unicode escape 4")
			<< "\\0020ACab"
			<< "€ab";
	QTest::newRow("unicode escape 5")
			<< "\\0020acab"
			<< "€ab";
	QTest::newRow("unicode escape 6")
			<< "\\0020AC  ab"
			<< "€ ab";
	QTest::newRow("unicode escape 7")
			<< "\\E9motion"
			<< "émotion";
	QTest::newRow("unicode escape 8")
			<< "\\E9 dition"
			<< "édition";
	QTest::newRow("unicode escape 9")
			<< "\\0000E9dition"
			<< "édition";
}

void
RichCssTest::testParseSelector()
{
	QFETCH(QString, selIn);
	QFETCH(QString, selOut);

	QCOMPARE(RichCSS::parseCssSelector(selIn), selOut);
}

void
RichCssTest::testParseRules_data()
{
	QTest::addColumn<QString>("cssIn");
	QTest::addColumn<QString>("cssOut");

	QTest::newRow("no spaces, no semicolon")
			<< "line-height:0"
			<< "line-height:0;";
	QTest::newRow("spaces around")
			<< "\t line-height:1px; "
			<< "line-height:1px;";
	QTest::newRow("spaces inside")
			<< "\t line-height \t : \t 1px \t ; "
			<< "line-height:1px;";
	QTest::newRow("multiple same rules")
			<< "\t line-height \t : \t 1px \t ; \t line-height \t : \t 3px \t ; "
			<< "line-height:3px;";
	QTest::newRow("attribute selector quoted")
			<< "\t line-height \t : \t 1px \t ; \t weight \t : \t bold \t ; "
			<< "line-height:1px;weight:bold;";
	QTest::newRow("apostrophe")
			<< "background : url ( \t '  something somewhere with \t spaces  ' \t )  "
			<< "background:url('  something somewhere with \t spaces  ');";
	QTest::newRow("quote with unicode")
			<< "background:url(\"  something som\\E9where with \t spaces \")"
			<< "background:url(\"  something soméwhere with \t spaces \");";
	QTest::newRow("quote with escaped quote")
			<< "background:url(\"  something som\\\"where with \t spaces \")"
			<< "background:url(\"  something som\\\"where with \t spaces \");";
	QTest::newRow("gradient")
			<< " background-image : linear-gradient  (  to \t  bottom  ,  dimgray  ,   lightgray  )  ;  "
			<< "background-image:linear-gradient(to bottom,dimgray,lightgray);";
	QTest::newRow("invalid stuff")
			<< "tag1 tag2"
			<< "";
}

void
RichCssTest::testParseRules()
{
	QFETCH(QString, cssIn);
	QFETCH(QString, cssOut);

	QCOMPARE(RichCSS::parseCssRules(cssIn).toString(), cssOut);
}

QTEST_GUILESS_MAIN(RichCssTest)
