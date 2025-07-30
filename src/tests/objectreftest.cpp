/*
    SPDX-FileCopyrightText: 2025 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "objectreftest.h"

#include <QTest>

#include "core/richtext/richdocument.h"
#include "core/subtitle.h"

#include <klocalizedstring.h>

using namespace SubtitleComposer;


ObjectRefTest::ObjectRefTest()
	: QObject()
{
	KLocalizedString::setApplicationDomain("subtitlecomposer");
}

ObjectRefTest::~ObjectRefTest()
{
}

void
ObjectRefTest::test()
{
	QExplicitlySharedDataPointer<SubtitleComposer::Subtitle> subA(new SubtitleComposer::Subtitle);
	QExplicitlySharedDataPointer<SubtitleComposer::Subtitle> subB(new SubtitleComposer::Subtitle);

	const int lines = 200;

	for(int n = 0; n < lines; n++) {
		SubtitleLine *la = new SubtitleLine(n * 1000, n * 1000 + 500);
		la->primaryDoc()->setPlainText(QString("A %1").arg(n));
		subA->insertLine(la);

		SubtitleLine *lb = new SubtitleLine(n * 1000, n * 1000 + 500);
		lb->primaryDoc()->setPlainText(QString("B %1").arg(n));
		subB->insertLine(lb);
	}

	QVERIFY(subA->count() == lines);
	for(int i = 0; i < subA->count(); i++) {
		QVERIFY(subA->at(i)->subtitle() == subA.data());
		QVERIFY(subA->at(i)->index() == i);
		// qDebug() << "subA" << subA->at(i)->index() << ":" << subA->at(i)->primaryDoc()->toPlainText();
	}

	QVERIFY(subB->count() == lines);
	for(int i = 0; i < subB->count(); i++) {
		QVERIFY(subB->at(i)->subtitle() == subB.data());
		QVERIFY(subB->at(i)->index() == i);
		// qDebug() << "subB" << subB->at(i)->index() << ":" << subB->at(i)->primaryDoc()->toPlainText();
	}
}

QTEST_MAIN(ObjectRefTest);
