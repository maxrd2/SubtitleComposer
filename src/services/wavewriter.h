#ifndef WAVEWRITER_H
#define WAVEWRITER_H

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "waveformat.h"

#include <stdio.h>
#include <QtCore/QString>

class WaveWriter
{
public:
	WaveWriter();
	~WaveWriter();

	bool open(const QString &outputPath, const WaveFormat &outputFormat);
	void close();

	bool isOpened() const;
	const QString & outputPath() const;
	const WaveFormat & outputFormat() const;

	double writtenSeconds() const;

// if necessary, data is converted to outputFormat
	bool writeSamplesData(void *data, unsigned long dataSize, const WaveFormat &dataFormat);

private:
	void * convertData(void *srcData, unsigned long srcDataSize, const WaveFormat &srcFormat, const WaveFormat &dstFormat, unsigned long &convertedDataSize);
	static void convertSample(void *srcData, unsigned long srcDataOffset, const WaveFormat &srcFormat, void *dstData, unsigned long dstDataOffset, const WaveFormat &dstFormat);

	void writeHeader();
	void writeNumber(unsigned long number, unsigned long writeSize, bool littleEndian = true);

private:
	QString m_outputPath;
	WaveFormat m_outputFormat;

	FILE *m_file;
	long m_bytesWritten;

	char *m_convertedData;
	unsigned long m_convertedDataSize;
};

#endif
