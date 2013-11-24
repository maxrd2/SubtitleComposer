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

#include "wavewriter.h"

#include <KDebug>

#define WAVE_HEADER_SIZE 44

WaveWriter::WaveWriter() :
	m_outputPath(),
	m_outputFormat(),
	m_file(0),
	m_bytesWritten(0),
	m_convertedData(0),
	m_convertedDataSize(0)
{}

WaveWriter::~WaveWriter()
{
	close();

	if(m_convertedData) {
		delete m_convertedData;
		m_convertedData = 0;
		m_convertedDataSize = 0;
	}
}

bool
WaveWriter::open(const QString &outputPath, const WaveFormat &outputFormat)
{
	if(isOpened() && !outputFormat.isValid())
		return false;

	m_file = fopen(outputPath.toLocal8Bit(), "wb+");
	if(!m_file)
		return false;

	// Reserve WAVE_HEADER_SIZE bytes of space for the WAVE header
	unsigned char headerFiller[WAVE_HEADER_SIZE];
	fwrite(headerFiller, 1, WAVE_HEADER_SIZE, m_file);

	m_outputPath = outputPath;
	m_outputFormat = outputFormat;

	return true;
}

void
WaveWriter::close()
{
	if(isOpened()) {
		writeHeader();

		fclose(m_file);
		m_file = 0;
		m_bytesWritten = 0;

		m_outputPath.clear();
	}
}

bool
WaveWriter::isOpened() const
{
	return m_file;
}

const QString &
WaveWriter::outputPath() const
{
	return m_outputPath;
}

const WaveFormat &
WaveWriter::outputFormat() const
{
	return m_outputFormat;
}

double
WaveWriter::writtenSeconds() const
{
	return (m_bytesWritten / m_outputFormat.bytesPerFrame()) / (double)m_outputFormat.sampleRate();
}

bool
WaveWriter::writeSamplesData(void *data, unsigned long dataSize, const WaveFormat &dataFormat)
{
	if(!isOpened())
		return false;

	if(dataFormat == m_outputFormat) {
		fwrite(data, dataSize, 1, m_file);
		m_bytesWritten += dataSize;
	} else {
		if(dataSize % dataFormat.bytesPerFrame())
			return false;

		void *convertedData = convertData(data, dataSize, dataFormat, m_outputFormat, dataSize);
		if(convertedData) {
			fwrite(convertedData, dataSize, 1, m_file);
			m_bytesWritten += dataSize;
		}
	}

	return true;
}

void *
WaveWriter::convertData(void *srcData, unsigned long srcDataSize, const WaveFormat &srcFormat, const WaveFormat &dstFormat, unsigned long &convertedDataSize)
{
	unsigned long srcFrames = srcDataSize / srcFormat.bytesPerFrame();
	unsigned long dstFrames = srcFrames;

	if(srcFormat.channels() != dstFormat.channels()) {
		// only supported channels convertion is downmixing to mono
		if(dstFormat.channels() != 1)
			return 0;
	}

	if(srcFormat.sampleRate() != dstFormat.sampleRate()) {
		// NOTE we only allow sampleRate conversions when srcSampleRate % dstSampleRate == 0 || srcSampleRate % srcSampleRate == 0.
		// N can be a decimation or an augmentation rate; if it is a decimation rate it must also be true that srcSamples % N == 0.

		if(srcFormat.sampleRate() > dstFormat.sampleRate()) {
			if(srcFormat.sampleRate() % dstFormat.sampleRate())
				return 0;
			if(srcFrames % (srcFormat.sampleRate() / dstFormat.sampleRate()))
				return 0;
		} else { // if ( srcFormat.sampleRate() < dstFormat.sampleRate() )
			if(dstFormat.sampleRate() % srcFormat.sampleRate())
				return 0;
		}

		dstFrames = srcFrames * dstFormat.sampleRate() / srcFormat.sampleRate();
	}

	convertedDataSize = dstFrames * dstFormat.bytesPerFrame();

	if(m_convertedData == 0 || convertedDataSize > m_convertedDataSize) {
		delete m_convertedData;
		m_convertedData = new char[convertedDataSize];
		m_convertedDataSize = convertedDataSize;
	}

	void *dstData = m_convertedData;

	if(srcFormat.sampleRate() == dstFormat.sampleRate()) {
		const int srcDataIncrement = srcFormat.bytesPerSample() * srcFormat.channels();
		const int dstDataIncrement = dstFormat.bytesPerSample() * dstFormat.channels();

		for(unsigned long srcDataOffset = 0, dstDataOffset = 0;
		    srcDataOffset < srcDataSize;
		    srcDataOffset += srcDataIncrement, dstDataOffset += dstDataIncrement)
			convertSample(srcData, srcDataOffset, srcFormat, dstData, dstDataOffset, dstFormat);
	} else if(srcFormat.sampleRate() > dstFormat.sampleRate()) {
		const int srcDataIncrement = srcFormat.bytesPerSample() * srcFormat.channels() * (srcFormat.sampleRate() / dstFormat.sampleRate());
		const int dstDataIncrement = dstFormat.bytesPerSample() * dstFormat.channels();

		for(unsigned long srcDataOffset = 0, dstDataOffset = 0;
		    srcDataOffset < srcDataSize;
		    srcDataOffset += srcDataIncrement, dstDataOffset += dstDataIncrement)
			convertSample(srcData, srcDataOffset, srcFormat, dstData, dstDataOffset, dstFormat);
	} else { // if ( srcFormat.sampleRate() < dstFormat.sampleRate() )
		const unsigned long augmentation = dstFrames / srcFrames;
		const int srcDataIncrement = srcFormat.bytesPerSample() * srcFormat.channels();
		const int dstDataIncrement = dstFormat.bytesPerSample() * dstFormat.channels() * augmentation;

		for(unsigned long initialDstDataOffset = 0; initialDstDataOffset < augmentation; ++initialDstDataOffset)
			for(unsigned long srcDataOffset = 0, dstDataOffset = initialDstDataOffset * dstDataIncrement;
			    srcDataOffset < srcDataSize;
			    srcDataOffset += srcDataIncrement, dstDataOffset += dstDataIncrement)
				convertSample(srcData, srcDataOffset, srcFormat, dstData, dstDataOffset, dstFormat);
	}

	return m_convertedData;
}

void
WaveWriter::convertSample(void *srcData, unsigned long srcDataOffset, const WaveFormat &srcFormat, void *dstData, unsigned long dstDataOffset, const WaveFormat &dstFormat)
{
	const unsigned srcBytesPerSample = srcFormat.bytesPerSample();
	const unsigned dstBytesPerSample = dstFormat.bytesPerSample();

	if(srcFormat.channels() == dstFormat.channels()) {
		if(dstBytesPerSample > srcBytesPerSample) {
			const unsigned bytesPerSampleDelta = dstBytesPerSample - srcBytesPerSample;
			for(int channelIndex = 0; channelIndex < srcFormat.channels(); ++channelIndex) {
				// dstBytesPerSample is greater than srcBytesPerSample, hence is greater than 1 and dstData is signed

				// zero the the dstData bytes without data
				unsigned long i = 0;
				for(; i < bytesPerSampleDelta; ++i)
					*(((char *)dstData) + dstDataOffset + i) = 0;

				if(srcBytesPerSample == 1) {
					// dstData is signed and srcData is unsigned
					*(((char *)dstData) + dstDataOffset + i) = *(((unsigned char *)srcData) + srcDataOffset - bytesPerSampleDelta + i) - 128;
				} else {
					// both dstData and srcData are signed
					for(; i < dstBytesPerSample; ++i)
						*(((char *)dstData) + dstDataOffset + i) = *(((char *)srcData) + srcDataOffset - bytesPerSampleDelta + i);
				}

				srcDataOffset += srcBytesPerSample;
				dstDataOffset += dstBytesPerSample;
			}
		} else {
			// dstBytesPerSample is less or equal than srcBytesPerSample, hence it can be one and dstData be unsigned or signed

			const unsigned bytesPerSampleDelta = srcBytesPerSample - dstBytesPerSample;
			for(int channelIndex = 0; channelIndex < srcFormat.channels(); ++channelIndex) {
				if(srcBytesPerSample == 1) {
					// both dstData and srcData are unsigned
					*(((unsigned char *)dstData) + dstDataOffset) = *(((unsigned char *)srcData) + srcDataOffset + bytesPerSampleDelta);
				} else {
					if(dstBytesPerSample == 1) {
						// dstData is unsigned and srcData is signed
						*(((unsigned char *)dstData) + dstDataOffset) = *(((char *)srcData) + srcDataOffset + bytesPerSampleDelta) + 128;
					} else {
						// both dstData and srcData are signed
						for(unsigned long i = 0; i < dstBytesPerSample; ++i)
							*(((char *)dstData) + dstDataOffset + i) = *(((char *)srcData) + srcDataOffset + bytesPerSampleDelta + i);
					}
				}

				srcDataOffset += srcBytesPerSample;
				dstDataOffset += dstBytesPerSample;
			}
		}
	} else {
		qint64 sampleValue = 0;

		for(int channelIndex = 0; channelIndex < srcFormat.channels(); ++channelIndex) {
			if(srcBytesPerSample == 1) {
				// srcData is unsigned
				sampleValue += *(((unsigned char *)srcData) + srcDataOffset) - 128;
			} else {
				// srcData is signed
				for(unsigned long i = 0; i < srcBytesPerSample; ++i)
					sampleValue += (*(((char *)srcData) + srcDataOffset + i) << (8 * i));
			}

			srcDataOffset += srcBytesPerSample;
		}

		sampleValue /= srcFormat.channels();

		if(dstBytesPerSample >= srcBytesPerSample)
			sampleValue = sampleValue << ((dstBytesPerSample - srcBytesPerSample) * 8);
		else
			sampleValue = sampleValue >> ((srcBytesPerSample - dstBytesPerSample) * 8);

		if(dstBytesPerSample == 1) {
			// dstData is unsigned
			*(((unsigned char *)dstData) + dstDataOffset) = (unsigned char)(sampleValue + 128);
		} else {
			memcpy(((char *)dstData) + dstDataOffset, &sampleValue, dstBytesPerSample);
		}
	}
}

void
WaveWriter::writeHeader()
{
	unsigned long bitsPerSample = m_outputFormat.bitsPerSample();
	unsigned long channels = m_outputFormat.channels();
	unsigned long sampleRate = m_outputFormat.sampleRate();
	unsigned long blockAlign = m_outputFormat.blockAlign();

	unsigned long byteRate = sampleRate * blockAlign;
	unsigned long samples = m_bytesWritten / blockAlign;
	unsigned long subchunk2Size = samples * blockAlign;

	// go the start of file
	fseek(m_file, 0, SEEK_SET);

	// RIFF header
	fwrite("RIFF", 1, 4, m_file);                 // ChunkID
	writeNumber(36 + subchunk2Size, 4);   // chunkSize
	fwrite("WAVE", 1, 4, m_file);                 // Format

	// fmt sub-chunk
	fwrite("fmt ", 1, 4, m_file);                 // Subchunk1ID
	writeNumber(16, 4);                                   // Subchunk1Size
	writeNumber(1, 2);                                    // AudioFormat
	writeNumber(channels, 2);                             // NumChannels
	writeNumber(sampleRate, 4);                   // SampleRate
	writeNumber(byteRate, 4);                             // ByteRate
	writeNumber(blockAlign, 2);                   // BlockAlign
	writeNumber(bitsPerSample, 2);                // BitsPerSample

	// data sub-chunk
	fwrite("data", 1, 4, m_file);                 // Subchunk2ID
	writeNumber(subchunk2Size, 4);                // Subchunk1Size
}

void
WaveWriter::writeNumber(unsigned long number, unsigned long writeSize, bool littleEndian)
{
	unsigned char writeData[writeSize];
	int idx, idx2, divisor;

	// get the little endian number representation
	for(idx = writeSize - 1; idx >= 0; --idx) {
		divisor = 1;
		for(idx2 = 0; idx2 < idx; ++idx2)
			divisor *= 256;
		writeData[idx] = (unsigned char)(number / divisor);
		number = number - (divisor * (number % divisor));
	}

	if(!littleEndian) {
		unsigned char aux;
		for(unsigned int idx = 0; idx < writeSize; ++idx) {
			aux = writeData[writeSize - idx];
			writeData[writeSize - idx] = writeData[idx];
			writeData[idx] = aux;
		}
	}

	fwrite(writeData, 1, writeSize, m_file);
}
