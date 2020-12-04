#include "richdocumenttest.h"

#include "helpers/common.h"

#include <QRegularExpression>
#include <QTest>
#include <QDebug>

using namespace SubtitleComposer;

enum Format {
	Plain = 1,
	Html = 2,
	Both = Plain | Html
};
Q_DECLARE_METATYPE(Format)

void
RichDocumentTest::testCursor()
{
	const QString backslashes = $("oXXo");
	doc.setHtml(backslashes);
	QTextCursor c(&doc);
	QEXPECT_FAIL("", "Already at doc Start", Continue);
	QVERIFY(c.movePosition(QTextCursor::Start));
	QVERIFY(c.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1));
	QCOMPARE(c.selectedText(), $("o"));
	QEXPECT_FAIL("", "Dunno why... some internal anchor == position?", Continue);
	QVERIFY(c.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 2));
	c.clearSelection();
	QVERIFY(c.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 2));
	QVERIFY(c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor));
	QCOMPARE(c.selectedText(), $("o"));
}

void
RichDocumentTest::testHtml_data()
{
	QTest::addColumn<QString>("html");
	QTest::addColumn<QString>("htmlOut");
	QTest::addColumn<QString>("textOut");

	QTest::newRow("bold styles")
			<< "<p>It has <b>bold</b> and <strong>strong</strong> styles.</p>"
			<< "It has <b>bold</b> and <b>strong</b> styles."
			<< "It has bold and strong styles.";
	QTest::newRow("italic styles")
			<< "<div>It has <i>italic</i> and <em>em</em> styles.</div>"
			<< "It has <i>italic</i> and <i>em</i> styles."
			<< "It has italic and em styles.";
	QTest::newRow("underline/strikethrough styles")
			<< "<span>It has <u>underline</u> and <s>strike</s> styles.</span>"
			<< "It has <u>underline</u> and <s>strike</s> styles."
			<< "It has underline and strike styles.";
	QTest::newRow("color styles")
			<< "<p>Try <font color=\"red\">red</font> and <font color=\"black\">black</font> and <span style=\"color:blue\">blue</span> color.</p>"
			<< "Try <font color=#ff0000>red</font> and <font color=#000000>black</font> and <font color=#0000ff>blue</font> color."
			<< "Try red and black and blue color.";
	QTest::newRow("unicode literal")
			<< "An unicode ♪ char."
			<< "An unicode \u266A char."
			<< "An unicode \u266A char.";
	QTest::newRow("misc tags, breaks etc")
			<< "<p>A non&nbsp;breakable space. "
			   "A newline.\n"
			   "A line break.<br>"
			   "And a</p><p>different paragraph</p>"
			   "and some <p>unclosed and bad</div> <b><i><u><s>tags."
			<< "A non\u00A0breakable space. "
			   "A newline.<br>\n"
			   "A line break.<br>\n"
			   "And a<br>\ndifferent paragraph<br>\n"
			   "and some <br>\nunclosed and bad <b><i><u><s>tags.</b></i></u></s>"
			<< "A non breakable space. "
			   "A newline.\n"
			   "A line break.\n"
			   "And a\ndifferent paragraph\n"
			   "and some \nunclosed and bad tags.";
}

void
RichDocumentTest::testHtml()
{
	QFETCH(QString, html);
	QFETCH(QString, htmlOut);
	QFETCH(QString, textOut);

	doc.setHtml(html);
	QCOMPARE(doc.toHtml(), htmlOut);
	QCOMPARE(doc.toPlainText(), textOut);

	doc.setHtml(htmlOut);
	QCOMPARE(doc.toHtml(), htmlOut);
	QCOMPARE(doc.toPlainText(), textOut);
}

void
RichDocumentTest::testRegExpReplace_data()
{
	QTest::addColumn<Format>("inType");
	QTest::addColumn<QString>("input");
	QTest::addColumn<QRegularExpression>("re");
	QTest::addColumn<Format>("repType");
	QTest::addColumn<QString>("replacement");
	QTest::addColumn<QString>("expected");

	QTest::newRow("t1")
			<< Plain << $("xx12xx345xx6xx7xx")
			<< RE$("x+")
			<< Plain << QString()
			<< QString();
	QTest::newRow("t2")
			<< Plain << $("xx12xx345xx6xx7xx")
			<< RE$("x+")
			<< Plain << $("y")
			<< QString();
	QTest::newRow("t3")
			<< Plain << $("xx12xx345xx6xx7xx")
			<< RE$("((6x)|x+)")
			<< Plain << $("\\2")
			<< QString();
	QTest::newRow("t4")
			<< Plain << $("xx12xx345xx6xx7xx")
			<< RE$("((6x)|x+)")
			<< Plain << $("o\\2o")
			<< QString();
	QTest::newRow("t5")
			<< Plain << $("xx12xx345xx6xx7xx")
			<< RE$("((\\d)x+)+")
			<< Plain << $("o\\2o")
			<< QString();
	QTest::newRow("swaps")
			<< Plain << $("aa12bb345cc6xx7yy")
			<< RE$("([a-z]+)\\d+([a-z]+)")
			<< Plain << $("\\2 swapped \\1")
			<< QString();
	// these are failing cause replacer is not very good with replacing tags yet
//	QTest::newRow("styles1")
//			<< Html << $("am<b>az</b>ing su<i>per</i> <u>stuff</u>")
//			<< RE$("\\b(\\w+)\\b")
//			<< Plain << $("X\\1X")
//			<< $("Xam<b>az</b>ingX Xsu<i>perX</i> <u>XstuffX</u>");
//	QTest::newRow("styles2")
//			<< Html << $("am<b>az</b>ing su<i>per</i> <u>stuff</u>")
//			<< RE$("\\b\\w+\\b")
//			<< Plain << $("word")
//			<< $("word word <u>word</u>");
}

void
RichDocumentTest::testRegExpReplace()
{
	QFETCH(Format, inType);
	QFETCH(QString, input);
	QFETCH(QRegularExpression, re);
	QFETCH(Format, repType);
	QFETCH(QString, replacement);
	QFETCH(QString, expected);

	if(inType == Plain)
		doc.setPlainText(input, true);
	else
		doc.setHtml(input, true);

	if(expected.isEmpty())
		expected = QString(input).replace(re, replacement);

	if(inType == Plain && repType == Plain) {
		doc.replace(re, replacement, false);
		QCOMPARE(doc.toPlainText(), expected);
	}

	doc.undo();
	doc.replace(re, replacement, true);
	QString docOut = inType == Plain && repType == Plain ? doc.toPlainText() : doc.toHtml();
	QCOMPARE(docOut, expected);
}

void
RichDocumentTest::testCleanupSpaces_data()
{
	QTest::addColumn<Format>("inType");
	QTest::addColumn<QString>("input");
	QTest::addColumn<QString>("expected");

	QTest::newRow("t1") << Html
		<< $("♪ It  <b>\t  \t  seems </b><i>\t  today<br>that</i> all <i>  	\t you</i> see ♪")
		<< $("\u266A It seems today\nthat all you see \u266A");
	QTest::newRow("t2") << Plain
		<< $("\n\n \t This  \t  is\tsome \t \n \t \n\t \t\n\n \t good subtitle\t\n\ttext. \t")
		<< $("This is some\ngood subtitle\ntext.");
	QTest::newRow("t3") << Plain
		<< $(" \t \nline text\n \t ")
		<< $("line text");
	QTest::newRow("t4") << Html
		<< $(" \t &nbsp; <br> &nbsp; <p>line text</p><p>\n \t </p><div><br>&nbsp;<br></div>")
		<< $("line text");
}

void
RichDocumentTest::testCleanupSpaces()
{
	QFETCH(Format, inType);
	QFETCH(QString, input);
	QFETCH(QString, expected);

	if(inType == Plain)
		doc.setPlainText(input);
	else
		doc.setHtml(input);
	doc.cleanupSpaces();
	QCOMPARE(doc.toPlainText(), expected);
}

void
RichDocumentTest::testUpperLower()
{
	doc.setPlainText($("some čćžšđ òàùèì lowercase \u266A♪\\!! text"));
	doc.toUpper();
	QCOMPARE(doc.toPlainText(), $("SOME ČĆŽŠĐ ÒÀÙÈÌ LOWERCASE \u266A♪\\!! TEXT"));
	doc.toLower();
	QCOMPARE(doc.toPlainText(), $("some čćžšđ òàùèì lowercase \u266A♪\\!! text"));
}

void
RichDocumentTest::testSentence_data()
{
	QTest::addColumn<bool>("sentenceStart");
	QTest::addColumn<QString>("input");
	QTest::addColumn<QString>("expected");

	QTest::newRow("t1") << true
		<< $("begINNING of ♪ some \t \u266A SENTENCE,")
		<< $("Beginning of ♪ some \t \u266A sentence,");
	QTest::newRow("t2") << false
		<< $("Which CONTINUES? to finish¿\nthe SENTENCE")
		<< $("which continues? To finish¿\nThe sentence");
	QTest::newRow("t3") << false
		<< $("BREAKS: LINE. AND PUP'S ENDS HERE!")
		<< $("breaks: line. And pup's ends here!");
}

void
RichDocumentTest::testSentence()
{
	QFETCH(bool, sentenceStart);
	QFETCH(QString, input);
	QFETCH(QString, expected);

	doc.setPlainText(input);
	doc.toSentenceCase(&sentenceStart);
	QCOMPARE(doc.toPlainText(), expected);
}

void
RichDocumentTest::testTitle_data()
{
	QTest::addColumn<bool>("sentenceStart");
	QTest::addColumn<QString>("input");
	QTest::addColumn<QString>("expected");

	QTest::newRow("t1") << true
		<< $("begINNING of ♪ s-o-m-e \t \u266A SENTENCE,")
		<< $("Beginning Of ♪ S-o-m-e \t \u266A Sentence,");
	QTest::newRow("t2") << false
		<< $("Whi_ch CONT:INUES? to finish¿\nthe SENTENCE")
		<< $("Whi_ch Cont:Inues? To Finish¿\nThe Sentence");
	QTest::newRow("t3") << false
		<< $("BRE+AKS: LINE. AND PUP'S ENDS HERE!")
		<< $("Bre+aks: Line. And Pup's Ends Here!");
}

void
RichDocumentTest::testTitle()
{
	QFETCH(bool, sentenceStart);
	QFETCH(QString, input);
	QFETCH(QString, expected);

	doc.setPlainText(input);
	doc.toSentenceCase(&sentenceStart, true, true);
	QCOMPARE(doc.toPlainText(), expected);
}

QTEST_GUILESS_MAIN(RichDocumentTest)
