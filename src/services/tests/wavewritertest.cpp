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

#include "wavewritertest.h"
#include "../decoder.h"
#include "../../common/qxtsignalwaiter.h"

#include <QtTest>                               // krazy:exclude=c++/includes
#include <QtCore>                               // krazy:exclude=c++/includes

#include <KDebug>

using namespace SubtitleComposer;

static void
convertoToFormat(int bitsPerSample, int channels)
{
	static Decoder *decoder = Decoder::instance();
	if(!decoder->isInitialized())
		decoder->initialize(0, "Xine");

	static const char *filesDir = "/home/sergio/Desktop/Desarrollo/subtitlecomposer/src/services/tests/";

	QString inputFile;
	foreach(inputFile, QDir(filesDir).entryList(QStringList("*.wav"))) {
		QString inputPath = filesDir + inputFile;

		QxtSignalWaiter openedWaiter(decoder, SIGNAL(fileOpened(const QString &)), SIGNAL(fileOpenError(const QString &)));
		kDebug() << "opening" << inputPath;
		decoder->openFile(inputPath);
		kDebug() << "waiting opened signal";
		openedWaiter.wait(20000);

		if(decoder->filePath().isEmpty()) {
			kDebug() << "failed to open file";
			return;
		}

		if(decoder->audioStreamNames().isEmpty()) {
			kDebug() << "no audio streams found in file";
			return;
		}

		const int audioStream = 0;

		QxtSignalWaiter decodingWaiter(decoder, SIGNAL(stopped()));
		WaveFormat outputFormat(decoder->audioStreamFormats().at(audioStream).sampleRate(), channels, bitsPerSample);
		QString outputPath = filesDir + QString("converted/") + inputFile.left(inputFile.length() - 4) + "_" + outputFormat.toString() + ".wav";
		kDebug() << "decoding as" << outputFormat.toString() << "to" << outputPath;
		decoder->decode(audioStream, outputPath, outputFormat);
		kDebug() << "waiting coversion end signal";
		decodingWaiter.wait(200000);

		QxtSignalWaiter closedWaiter(decoder, SIGNAL(fileClosed()));
		kDebug() << "closing" << inputFile;
		decoder->closeFile();
		kDebug() << "waiting closed signal";
		closedWaiter.wait(20000);
	}
}

void
WaveWriterTest::testWriteMono8bps()
{
	convertoToFormat(8, 1);
}

void
WaveWriterTest::testWriteMono16bps()
{
	convertoToFormat(16, 1);
}

void
WaveWriterTest::testWriteMono24bps()
{
	convertoToFormat(24, 1);
}

void
WaveWriterTest::testWriteMono32bps()
{
	convertoToFormat(32, 1);
}

void
WaveWriterTest::testWriteStereo8bps()
{
	convertoToFormat(8, 2);
}

void
WaveWriterTest::testWriteStereo16bps()
{
	convertoToFormat(16, 2);
}

void
WaveWriterTest::testWriteStereo24bps()
{
	convertoToFormat(24, 2);
}

void
WaveWriterTest::testWriteStereo32bps()
{
	convertoToFormat(32, 2);
}

/*
void WaveWriterTest::testConstructors()
{
	WaveWriter ranges;
	QVERIFY( ranges.isEmpty() && ranges.rangesCount() == 0 );

	ranges << Range( 1, 2 );
	QVERIFY( ranges.firstIndex() == 1 && ranges.lastIndex() == 2 && ranges.indexesCount() == 2 );

	WaveWriter ranges2( Range( 1, 2 ) );
	QVERIFY( ranges == ranges2 );

	ranges2 = ranges;
	QVERIFY( ranges == ranges2 );

	ranges << Range( 7, 9 );
	QVERIFY( ranges.firstIndex() == 1 && ranges.lastIndex() == 9 && ranges.indexesCount() == 5 );

	WaveWriter ranges3( ranges );
	QVERIFY( ranges == ranges3 );

	ranges3 = ranges;
	QVERIFY( ranges == ranges3 );

	WaveWriter complementRanges = ranges.complement();
	QVERIFY( complementRanges.firstIndex() == 0 );
	QVERIFY( complementRanges.lastIndex() == Range::MaxIndex );

	WaveWriter::ConstIterator complementRangesIt = complementRanges.begin();
	QVERIFY( *(complementRangesIt++) == Range( 0, 0 ) );
	QVERIFY( *(complementRangesIt++) == Range( 3, 6 ) );
	QVERIFY( *(complementRangesIt++) == Range( 10, Range::MaxIndex ) );
}

void WaveWriterTest::testJoinAndTrim()
{
	WaveWriter ranges;

	ranges << Range( 1, 4 );
	QVERIFY( ranges.rangesCount() == 1 && ranges.indexesCount() == 4 );

	ranges << Range( 3, 5 );
	QVERIFY( ranges.rangesCount() == 1 && ranges.indexesCount() == 5 );

	ranges << Range( 7, 7 );
	QVERIFY( ranges.rangesCount() == 2 && ranges.indexesCount() == 6 );

	ranges << Range( 13, 16 );
	QVERIFY( ranges.rangesCount() == 3 && ranges.indexesCount() == 10 );

	ranges << Range( 6, 15 );
	QVERIFY( ranges.rangesCount() == 1 && ranges.indexesCount() == 16 );

	ranges.trimToRange( Range( 0, 17 ) );
	QVERIFY( ranges.rangesCount() == 1 && ranges.indexesCount() == 16 );

	ranges.trimToRange( Range( 0, 16 ) );
	QVERIFY( ranges.rangesCount() == 1 && ranges.indexesCount() == 16 );

	ranges.trimToRange( Range( 0, 15 ) );
	QVERIFY( ranges.rangesCount() == 1 && ranges.indexesCount() == 15 );

	ranges.trimToRange( Range( 0, 5 ) );
	QVERIFY( ranges.rangesCount() == 1 && ranges.indexesCount() == 5 );
}
 */

QTEST_MAIN(WaveWriterTest);

#include "wavewritertest.moc"
