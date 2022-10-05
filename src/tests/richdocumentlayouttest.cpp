/*
    SPDX-FileCopyrightText: 2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "richdocumentlayouttest.h"

#include "core/richtext/richdocument.h"
#include "helpers/debug.h"

#include <QDebug>
#include <QString>
#include <QTest>
#include <QTextCharFormat>
#include <QTextLayout>

using namespace SubtitleComposer;

RichDocumentLayoutTest::RichDocumentLayoutTest()
	: richLayout(new RichDocument(this))
{
}

// format helper
static QVector<QTextLayout::FormatRange>
parseFormat(const QString &str, bool merged=false)
{
	static QMap<QChar, QTextCharFormat> fmtMap;
	if(fmtMap.isEmpty()) {
		fmtMap[QChar(' ')] = QTextCharFormat();
		{
			QTextCharFormat d;
			d.setFontWeight(QFont::Bold);
			d.setFontItalic(false);
			d.setForeground(Qt::red);
			fmtMap[QChar('d')] = d;
		}
		{
			QTextCharFormat l;
			l.setFontWeight(QFont::Normal);
			l.setFontItalic(true);
			l.setForeground(Qt::green);
			fmtMap[QChar('l')] = l;
		}
		{
			QTextCharFormat m;
			m.setFontWeight(QFont::Bold);
			m.setFontItalic(true);
			m.setForeground(Qt::green);
			fmtMap[QChar('m')] = m;
		}
	};

	QVector<QTextLayout::FormatRange> fmts;
	int start = 0;
	int end = 1;
	for(;;) {
		if(str.length() <= end || str.at(start) != str.at(end)) {
			QTextCharFormat fmt = fmtMap[str.at(start)];
			if(!fmt.isEmpty()) {
				if(merged)
					fmt.setProperty(RichDocument::Merged, QTextCharFormat());
				fmts.push_back(QTextLayout::FormatRange{start, end - start, fmt});
			}
			start = end;
		}
		if(str.length() <= end)
			break;
		end++;
	}
	return fmts;
}

// Qt's QTextCharFormat::operator==() fails if properties/styles are not it same order
static bool
compare(const QVector<QTextLayout::FormatRange> &lhs, const QVector<QTextLayout::FormatRange> &rhs)
{
	if(lhs.size() != rhs.size())
		return false;
	for(int i = 0; i < lhs.size(); i++) {
		const QTextLayout::FormatRange &a = lhs.at(i);
		const QTextLayout::FormatRange &b = rhs.at(i);
		if(a.start != b.start || a.length != b.length)
			return false;
		const QMap<int, QVariant> &fa = a.format.properties();
		const QMap<int, QVariant> &fb = b.format.properties();
		for(auto it = fa.constBegin(); it != fa.constEnd(); ++it) {
			if(it.key() == RichDocument::Merged)
				continue;
			const auto v = fb.find(it.key());
			if(v == fb.cend()) {
				qDebug() << "rhs doesn't contain" << propertyName(QTextFormat::Property(it.key()));
				return false;
			}
			if(it.value() != v.value()) {
				qDebug() << "rhs mismatch" << propertyName(QTextFormat::Property(it.key())) << it.value() << "!=" << v.value();
				return false;
			}
		}
	}
	return true;
}

void
RichDocumentLayoutTest::testMerge_data()
{
	QTest::addColumn<QString>("doc");
	QTest::addColumn<QString>("layout");
	QTest::addColumn<QString>("merged");

	QTest::newRow("test1")
			<< "ddddd   ddddd"
			<< "lllll   lllll"
			<< "mmmmm   mmmmm";
	QTest::newRow("test2")
			<< "ddddd   ddddd"
			<< "  llllllll   "
			<< "ddmmmlllmmddd";
	QTest::newRow("test3")
			<< " dd d   ddd d"
			<< "  llllllll   "
			<< " dmlmlllmmd d";
	QTest::newRow("test4")
			<< "  dddddddd   "
			<< "lllll   lllll"
			<< "llmmmdddmmlll";
	QTest::newRow("test5")
			<< " ddd   ddd    "
			<< "   lll   lll  "
			<< " ddmll ddmll  ";
	QTest::newRow("test6")
			<< "   ddd   ddd  "
			<< " lll   lll    "
			<< " llmdd llmdd  ";
	QTest::newRow("test7")
			<< "ddddddddddddd"
			<< "    llll     "
			<< "ddddmmmmddddd";
}

void
RichDocumentLayoutTest::testMerge()
{
	QFETCH(QString, doc);
	QFETCH(QString, layout);
	QFETCH(QString, merged);

	QVector<QTextLayout::FormatRange> docFmts = parseFormat(doc);
	QVector<QTextLayout::FormatRange> layoutFmts = parseFormat(layout);
	QVector<QTextLayout::FormatRange> mergedFmts = parseFormat(merged, true);
	QVector<QTextLayout::FormatRange> res = richLayout.mergeCSS(docFmts, layoutFmts);

//	qDebug().noquote() << dumpFormatRanges(merged, mergedFmts);
//	qDebug().noquote() << dumpFormatRanges(merged, res);
	QVERIFY2(compare(res, mergedFmts), "Compared values are not the same");

	QVERIFY2(compare(richLayout.mergeCSS(docFmts, res), mergedFmts), "Compared values are not the same");
}

QTEST_MAIN(RichDocumentLayoutTest)
